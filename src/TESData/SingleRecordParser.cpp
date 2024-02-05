#include "SingleRecordParser.h"
#include "FormParser.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace TESData
{

using Game = FormParserManager::Game;

static constexpr auto GameMap = std::to_array<std::pair<QStringView, Game>>({
    {u"Skyrim Special Edition", Game::SSE},
    {u"Fallout 4", Game::FO4},
    {u"Skyrim", Game::TES5},
    {u"New Vegas", Game::FNV},
    {u"Fallout 3", Game::FO3},
    {u"Oblivion", Game::TES4},
});

static Game gameIdentifier(QStringView gameName)
{
  const auto it = std::ranges::find_if(GameMap, [=](auto&& pair) {
    return pair.first == gameName;
  });

  if (it != std::end(GameMap)) {
    return it->second;
  } else {
    return Game::Unknown;
  }
}

SingleRecordParser::SingleRecordParser(const QString& gameName, const RecordPath& path,
                                       const std::string& file, DataItem* root,
                                       int index)
    : m_GameName{gameName}, m_Path{path}, m_File{file}, m_DataRoot{root},
      m_FileIndex{index}
{}

bool SingleRecordParser::Group(TESFile::GroupData group)
{
  if (m_Depth == m_Path.groups().size()) {
    return false;
  }

  if (group.hasParent()) {
    const std::uint8_t localIndex = group.parent() >> 24U;
    const std::string& owner =
        localIndex < m_Masters.size() ? m_Masters[localIndex] : m_File;
    const auto files            = m_Path.files();
    const std::uint8_t newIndex = static_cast<std::uint8_t>(
        std::distance(std::begin(files), std::ranges::find(files, owner)));

    group.setLocalIndex(newIndex);
  }

  if (group == m_Path.groups()[m_Depth]) {
    ++m_Depth;
    return true;
  }

  return false;
}

bool SingleRecordParser::Form(TESFile::FormData form)
{
  m_CurrentType  = form.type();
  m_CurrentFlags = form.flags();

  if (m_Depth == 0) {
    if (form.type() == "TES4"_ts) {
      m_Localized = (form.flags() & TESFile::RecordFlags::Localized);
    }

    return true;
  }

  if (m_Path.hasFormId()) {
    if ((form.formId() & 0xFFFFFF) != (m_Path.formId() & 0xFFFFFF)) {
      return false;
    }

    const std::uint8_t localIndex = form.formId() >> 24U;
    const std::string& owner =
        localIndex < m_Masters.size() ? m_Masters[localIndex] : m_File;
    if (m_Path.files()[m_Path.formId() >> 24U] != owner) {
      return false;
    }

    m_RecordFound   = true;
    const auto game = gameIdentifier(m_GameName);
    FormParserManager::getParser(game, m_CurrentType)
        ->parseFlags(m_DataRoot, m_FileIndex, m_CurrentFlags);
    return true;
  } else {
    return !m_RecordFound;
  }

  return false;
}

bool SingleRecordParser::Chunk(TESFile::Type type)
{
  m_CurrentChunk = type;
  return true;
}

void SingleRecordParser::Data(std::istream& stream)
{
  m_ChunkStream = &stream;

  if (m_Depth == 0) {
    if (m_CurrentChunk == "MAST"_ts) {
      std::string master;
      std::getline(stream, master, '\0');
      if (!master.empty()) {
        m_Masters.push_back(master);
      }
    }
    return;
  }

  if (m_Path.hasEditorId() && m_CurrentChunk == "EDID"_ts) {
    std::string editorId;
    std::getline(stream, editorId, '\0');
    if (editorId != m_Path.editorId()) {
      return;
    }

    m_RecordFound = true;
    const auto game = gameIdentifier(m_GameName);
    FormParserManager::getParser(game, m_CurrentType)
        ->parseFlags(m_DataRoot, m_FileIndex, m_CurrentFlags);
    stream.seekg(std::ios::beg);
  }

  if (m_Path.hasTypeId() && m_CurrentChunk == "DNAM"_ts) {
    while (!stream.eof()) {
      const TESFile::Type name      = TESFile::readType<TESFile::Type>(stream);
      const std::uint32_t formId    = TESFile::readType<std::uint32_t>(stream);
      const std::uint8_t localIndex = formId >> 24U;
      const auto& file = localIndex < m_Masters.size() ? m_Masters[localIndex] : m_File;
      if (name == m_Path.typeId()) {
        m_DataRoot->getOrInsertChild(0, name, u""_s)
            ->setData(m_FileIndex,
                      u"%2 | %1"_s.arg(formId & 0xFFFFFFU, 6, 16, QChar(u'0'))
                          .toUpper()
                          .arg(QString::fromStdString(file)));
        m_RecordFound = true;
        break;
      }
      if (name == "BBBB"_ts) {
        break;
      }
    }
    return;
  }

  if (!m_RecordFound) {
    return;
  }

  if (!m_ParseTask) {
    const auto game = gameIdentifier(m_GameName);
    m_ParseTask     = FormParserManager::getParser(game, m_CurrentType)
                      ->parseForm(m_DataRoot, m_FileIndex, m_Localized, m_Masters,
                                  m_File, m_CurrentChunk, m_ChunkStream);
  }

  m_ParseTask.resume();
}

}  // namespace TESData
