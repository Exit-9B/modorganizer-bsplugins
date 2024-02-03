#ifndef TESDATA_FORMPARSER_H
#define TESDATA_FORMPARSER_H

#include "DataItem.h"
#include "TESFile/Stream.h"
#include "TESFile/Type.h"

#include <bit>
#include <coroutine>
#include <istream>
#include <map>
#include <memory>
#include <ranges>
#include <span>
#include <utility>

using namespace Qt::Literals::StringLiterals;

namespace TESData
{

namespace Parse
{
  struct Promise;

  struct Task : std::coroutine_handle<Promise>
  {
    using promise_type = Promise;

    Task(std::coroutine_handle<Promise>&& promise)
        : std::coroutine_handle<Promise>::coroutine_handle(promise)
    {}
  };

  struct Promise
  {
    Task get_return_object() { return {Task::from_promise(*this)}; }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
  };
}  // namespace Parse

using ParseTask = Parse::Task;

class IFormParser
{
public:
  virtual ParseTask parseForm(DataItem* root, int fileIndex, bool localized,
                              std::span<const std::string> masters,
                              const std::string& plugin, const TESFile::Type& signature,
                              std::istream* const& stream) const = 0;
};

class FormParserManager final
{
  template <typename T, TESFile::Type Type>
  friend struct RegisterFormParser;

public:
  enum class Game
  {
    TES4,
    FO3,
    FNV,
    TES5,
    FO4,
    SSE,

    Unknown
  };

  FormParserManager() = delete;

  static std::shared_ptr<const IFormParser> getParser(Game game, TESFile::Type type)
  {
    if (game == Game::SSE) {
      if (const auto it = registrationMap().find(type); it != registrationMap().end()) {
        return it->second;
      }
    }

    return registrationMap()[TESFile::Type()];
  }

private:
  static auto& registrationMap()
  {
    static std::map<TESFile::Type, std::shared_ptr<IFormParser>> registrationMap;
    return registrationMap;
  }
};

template <typename T, TESFile::Type Type>
struct RegisterFormParser
{
  explicit RegisterFormParser()
  {
    FormParserManager::registrationMap().emplace(Type, std::make_shared<T>());
  }
};

template <TESFile::Type Type = {}>
class FormParser : public IFormParser
{
  inline static RegisterFormParser<FormParser<Type>, Type> reg{};

public:
  ParseTask parseForm(DataItem* root, int fileIndex, bool localized,
                      std::span<const std::string> masters, const std::string& plugin,
                      const TESFile::Type& signature,
                      std::istream* const& stream) const override;
};

inline QString readBytes(std::istream& stream, int size)
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

inline QString readLstring(bool localized, std::istream& stream)
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

inline QString readFormId(std::span<const std::string> masters,
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
    item = parent->insertChild(index++, signature, u"Unknown"_s,
                               DataItem::ConflictType::Override);
  }

  item->setDisplayData(fileIndex, readBytes(stream, 256));
}

template <>
inline ParseTask
FormParser<>::parseForm(DataItem* root, int fileIndex, [[maybe_unused]] bool localized,
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
          ->setDisplayData(fileIndex, QString::fromStdString(editorId));
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

#endif  // TESDATA_FORMPARSER_H
