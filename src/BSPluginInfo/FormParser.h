#ifndef BSPLUGININFO_FORMPARSER_H
#define BSPLUGININFO_FORMPARSER_H

#include "DataItem.h"
#include "TESFile/Stream.h"
#include "TESFile/Type.h"

#include <coroutine>
#include <istream>
#include <map>
#include <memory>

namespace BSPluginInfo
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
                              const TESFile::Type& signature,
                              std::istream* const& stream) const = 0;
};

class FormParserManager final
{
  template <typename T, TESFile::Type Type>
  friend struct RegisterFormParser;

public:
  FormParserManager() = delete;

  static std::shared_ptr<const IFormParser> getParser(TESFile::Type type)
  {
    const auto it = registrationMap().find(type);
    if (it != registrationMap().end()) {
      return it->second;
    } else {
      return registrationMap()[TESFile::Type()];
    }
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
                      const TESFile::Type& signature,
                      std::istream* const& stream) const override;
};

}  // namespace BSPluginInfo

#include "FormParser.inl"

#endif  // BSPLUGININFO_FORMPARSER_H
