#ifndef TESFILE_TYPE_H
#define TESFILE_TYPE_H

#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace TESFile
{

template <std::integral T>
constexpr std::make_unsigned_t<T> bswap(T i)
{
  return []<std::size_t... N>(std::make_unsigned_t<T> i, std::index_sequence<N...>) {
    return ((((i >> (N * 8)) & 0xFF) << ((sizeof(T) - 1 - N) * 8)) | ...);
  }(i, std::make_index_sequence<sizeof(T)>());
}

#pragma pack(push, 1)
struct Type
{
  std::uint32_t value;

  constexpr Type() = default;

  template <std::size_t N>
  constexpr Type(const char (&str)[N])
      : value{[&]<std::size_t... N>(std::index_sequence<N...>) {
          return ((static_cast<std::uint32_t>(str[N]) << N * 8U) | ...);
        }(std::make_index_sequence<N - 1>())}
  {}

  constexpr operator std::uint32_t() const { return value; }

  constexpr auto operator<=>(const Type& other) const
  {
    return bswap(value) <=> bswap(other.value);
  }

  [[nodiscard]] std::string string() const { return std::string(view()); }

  [[nodiscard]] std::string_view view() const
  {
    return std::string_view(reinterpret_cast<const char*>(&value), sizeof(value));
  }

  [[nodiscard]] const char* data() const
  {
    return reinterpret_cast<const char*>(&value);
  }

  [[nodiscard]] static constexpr std::size_t size() { return sizeof(value); }
};
static_assert(sizeof(Type) == 4);
static_assert(alignof(Type) == 1);
static_assert(Type().value == 0);
#pragma pack(pop)

namespace literals
{
  template <Type T>
  consteval Type operator""_ts()
  {
    return T;
  }
  static_assert("TES4"_ts.value == '4SET');
  static_assert("AMMO"_ts < "BOOK"_ts);
}  // namespace literals
}  // namespace TESFile

using namespace TESFile::literals;

#endif  // TESFILE_TYPE_H
