#include "FormParser.h"

#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace BSPluginInfo
{

inline void parseUnknown(DataItem* parent, int& index, int fileIndex,
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
    item = parent->insertChild(index++, signature, "Unknown");
  }

  QString data;
  while (data.length() < 1023) {
    char ch;
    stream.get(ch);
    if (stream.eof())
      break;
    data += u"%1 "_s.arg(static_cast<std::uint8_t>(ch), 2, 16, QChar(u'0')).toUpper();
  }
  data.chop(1);

  item->setDisplayData(fileIndex, std::move(data));
}

template <>
ParseTask FormParser<>::parseForm(DataItem* root, int fileIndex, bool localized,
                                  const TESFile::Type& signature,
                                  std::istream* const& stream) const
{
  int index = 0;
  for (;;) {
    if (signature == "EDID"_ts) {
      std::string editorId;
      std::getline(*stream, editorId, '\0');
      root->getOrInsertChild(index++, "EDID"_ts, "Editor ID")
          ->setDisplayData(fileIndex, QString::fromStdString(editorId));
    } else {
      parseUnknown(root, index, fileIndex, signature, *stream);
    }

    co_await std::suspend_always();
  }
}

template <>
ParseTask FormParser<"GMST">::parseForm(DataItem* root, int fileIndex, bool localized,
                                        const TESFile::Type& signature,
                                        std::istream* const& stream) const
{
  DataItem* parent = root;
  int index        = 0;

  std::string editorId = "\0";
  if (signature == "EDID"_ts) {
    std::getline(*stream, editorId, '\0');
    parent->getOrInsertChild(index++, "EDID"_ts, u"Editor ID"_s)
        ->setDisplayData(fileIndex, QString::fromStdString(editorId));

    co_await std::suspend_always();
  }

  if (signature == "DATA"_ts) {
    QString data;
    switch (editorId[0]) {
    case 'b':
      data = TESFile::readType<std::uint32_t>(*stream) ? u"True"_s : u"False"_s;
      break;
    case 'f':
      data = QString::number(TESFile::readType<float>(*stream));
      break;
    case 'i':
      data = QString::number(TESFile::readType<std::int32_t>(*stream));
      break;
    case 's': {
      if (localized) {
        data = u"<lstring>"_s;
      } else {
        std::string str;
        std::getline(*stream, str, '\0');
        data = QString::fromStdString(str);
      }
    } break;
    case 'u':
      data = QString::number(TESFile::readType<std::uint32_t>(*stream));
      break;
    }
    parent->getOrInsertChild(index++, "DATA"_ts, u"Value"_s)
        ->setDisplayData(fileIndex, data);

    co_await std::suspend_always();
  }
}

}  // namespace BSPluginInfo
