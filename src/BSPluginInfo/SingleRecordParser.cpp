#include "SingleRecordParser.h"

#include <algorithm>
#include <iterator>

using namespace Qt::Literals::StringLiterals;

namespace BSPluginInfo
{

SingleRecordParser::SingleRecordParser(const TESData::RecordPath& path,
                                       const std::string& file, DataItem* root,
                                       int index)
    : m_Path{path}, m_File{file}, m_DataRoot{root}, m_Index{index}
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

    m_RecordFound = true;
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

  if (m_CurrentChunk == "EDID"_ts) {
    std::string editorId;
    std::getline(stream, editorId, '\0');
    if (!m_RecordFound && m_Path.hasEditorId()) {
      if (editorId == m_Path.editorId()) {
        m_RecordFound = true;
      } else {
        return;
      }
    }

    if (m_RecordFound) {
      m_DataRoot
          ->findOrAddChild(
              m_CurrentChunk,
              QString::fromLocal8Bit(m_CurrentChunk.data(), m_CurrentChunk.size()))
          ->setDisplayData(m_Index, QString::fromStdString(editorId));
      return;
    }
  }

  if (m_Path.hasTypeId() && m_CurrentChunk == "DNAM"_ts) {
    while (!stream.eof()) {
      const TESFile::Type name      = TESFile::readType<TESFile::Type>(stream);
      const std::uint32_t formId    = TESFile::readType<std::uint32_t>(stream);
      const std::uint8_t localIndex = formId >> 24U;
      const auto& file = localIndex < m_Masters.size() ? m_Masters[localIndex] : m_File;
      if (name == m_Path.typeId()) {
        m_DataRoot
            ->findOrAddChild(name, QString::fromLocal8Bit(name.data(), name.size()))
            ->setDisplayData(m_Index,
                             u"%2|%1"_s.arg(formId & 0xFFFFFFU, 6, 16, QChar(u'0'))
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

  QString data;
  while (!stream.eof() && data.length() < 1023) {
    char ch;
    stream.get(ch);
    data += u"%1 "_s.arg(static_cast<std::uint8_t>(ch), 2, 16, QChar(u'0')).toUpper();
  }
  data.chop(1);

  m_DataRoot
      ->findOrAddChild(m_CurrentChunk, QString::fromLocal8Bit(m_CurrentChunk.data(),
                                                              m_CurrentChunk.size()))
      ->setDisplayData(m_Index, data);
}

}  // namespace BSPluginInfo
