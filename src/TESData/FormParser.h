#ifndef TESDATA_FORMPARSER_H
#define TESDATA_FORMPARSER_H

#include "DataItem.h"
#include "TESFile/Stream.h"
#include "TESFile/Type.h"

#include <coroutine>
#include <istream>
#include <map>
#include <memory>
#include <span>
#include <string>

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
  virtual void parseFlags(DataItem* root, int fileIndex, std::uint32_t flags) const = 0;

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

  [[nodiscard]] static std::shared_ptr<const IFormParser> getParser(Game game,
                                                                    TESFile::Type type);

private:
  using RegistrationMap = std::map<TESFile::Type, std::shared_ptr<IFormParser>>;

  static auto registrationMap() -> RegistrationMap&;
};

template <typename T, TESFile::Type Type>
struct [[maybe_unused]] RegisterFormParser
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
  void parseFlags(DataItem* root, int fileIndex, std::uint32_t flags) const override;

  ParseTask parseForm(DataItem* root, int fileIndex, bool localized,
                      std::span<const std::string> masters, const std::string& plugin,
                      const TESFile::Type& signature,
                      std::istream* const& stream) const override;
};

}  // namespace TESData

#endif  // TESDATA_FORMPARSER_H
