#include "FormParser.h"

#include <bit>
#include <ranges>
#include <utility>

namespace TESData
{

auto FormParserManager::registrationMap() -> RegistrationMap&
{
  static RegistrationMap registrationMap;
  return registrationMap;
}

std::shared_ptr<const IFormParser> FormParserManager::getParser(Game game,
                                                                TESFile::Type type)
{
  if (game == Game::SSE) {
    if (const auto it = registrationMap().find(type); it != registrationMap().end()) {
      return it->second;
    }
  }

  return registrationMap()[TESFile::Type()];
}

static QString readBytes(std::istream& stream, int size)
{
  QString data;
  for (int i = 0; i < size; ++i) {
    char ch;
    stream.get(ch);
    if (stream.eof()) {
      break;
    }
    data += u"%1 "_s.arg(static_cast<std::uint8_t>(ch), 2, 16, QChar(u'0')).toUpper();
  }
  return data;
}

static QString readLstring(bool localized, std::istream& stream)
{
  if (localized) {
    const std::uint32_t index = TESFile::readType<std::uint32_t>(stream);
    return u"<lstring:%1>"_s.arg(index);
  } else {
    std::string str;
    std::getline(stream, str, '\0');
    return QString::fromStdString(str);
  }
}

static QString readFormId(std::span<const std::string> masters,
                          const std::string& plugin, std::istream& stream)
{
  const std::uint32_t formId = TESFile::readType<std::uint32_t>(stream);
  if (!formId) {
    return u"NONE"_s;
  }

  const std::uint8_t localIndex = formId >> 24U;
  const std::string& file = localIndex < masters.size() ? masters[localIndex] : plugin;
  return u"%2 | %1"_s.arg(formId & 0xFFFFFFU, 6, 16, QChar(u'0'))
      .toUpper()
      .arg(QString::fromStdString(file));
}

static void parseUnknown(DataItem* parent, int& index, int fileIndex,
                         TESFile::Type signature, std::istream& stream)
{
  DataItem* item = nullptr;
  for (int i = index; i < parent->numChildren(); ++i) {
    const auto child = parent->childAt(i);
    if (child->signature() == signature) {
      item  = child;
      index = i + 1;
      break;
    }
  }

  if (item == nullptr) {
    item = parent->insertChild(index++, signature, u"Unknown"_s,
                               DataItem::ConflictType::Override);
  }

  item->setData(fileIndex, readBytes(stream, 256));
}

template <>
ParseTask FormParser<>::parseForm(DataItem* root, int fileIndex,
                                  [[maybe_unused]] bool localized,
                                  [[maybe_unused]] std::span<const std::string> masters,
                                  [[maybe_unused]] const std::string& plugin,
                                  const TESFile::Type& signature,
                                  std::istream* const& stream) const
{
  int index = 0;
  for (;;) {
    if (signature == "EDID"_ts) {
      std::string editorId;
      std::getline(*stream, editorId, '\0');
      root->getOrInsertChild(index++, "EDID"_ts, u"Editor ID"_s)
          ->setData(fileIndex, QString::fromStdString(editorId));
    } else {
      parseUnknown(root, index, fileIndex, signature, *stream);
    }

    co_await std::suspend_always();
  }
}

}  // namespace TESData

#pragma warning(push)
#pragma warning(disable : 4456)
#include "FormParser.SSE.inl"
#pragma warning(pop)
