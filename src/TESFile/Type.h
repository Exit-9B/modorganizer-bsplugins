#ifndef TESFILE_TYPE_H
#define TESFILE_TYPE_H

#include <bit>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace TESFile
{

#pragma pack(push, 1)
struct Type
{
  std::uint32_t value;

  constexpr operator std::uint32_t() const { return value; }

  constexpr auto operator<=>(const Type& other) const { return value <=> other.value; }

  std::string_view str() const
  {
    return std::string_view(reinterpret_cast<const char*>(&value), sizeof(value));
  }
};
static_assert(sizeof(Type) == 4);
#pragma pack(pop)

namespace literals
{

  consteval Type operator""_ts(const char* id, std::size_t size)
  {
    static_assert(std::endian::native == std::endian::little);

    std::uint32_t value = 0;
    for (int i = 0; i < size; ++i) {
      value += id[i] << (8 * i);
    }
    return Type{value};
  }
  static_assert("TES4"_ts.value == '4SET');

}  // namespace literals
}  // namespace TESFile

using namespace TESFile::literals;

#endif  // TESFILE_TYPE_H
