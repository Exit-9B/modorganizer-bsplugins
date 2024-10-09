#ifndef TESFILE_STREAM_H
#define TESFILE_STREAM_H

#include "Type.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <istream>
#include <ranges>
#include <utility>

namespace TESFile
{

template <typename T>
inline T readType(std::istream& stream)
{
  union
  {
    char buffer[sizeof(T)];
    T value;
  };
  std::memset(buffer, 0x42, sizeof(T));
  stream.read(buffer, sizeof(T));
  return value;
}

inline std::string readZstring(std::istream& stream)
{
  std::string value;
  std::getline(stream, value, '\0');
  return value;
}

inline bool iequals(const std::string& a, const std::string& b)
{
  return _stricmp(a.data(), b.data()) == 0;
}

template <std::ranges::input_range R, typename Proj = std::identity>
inline auto find(R&& r, const std::string& value, Proj proj = {})
{
  return std::ranges::find_if(
      r,
      [&](auto&& val) {
        return iequals(val, value);
      },
      proj);
}

struct less
{
  bool operator()(const std::string& a, const std::string& b) const
  {
    return _stricmp(a.data(), b.data()) < 0;
  }
};

enum class TESFormat
{
  Standard,
  Oblivion,
  Morrowind,
};

enum class GroupType : std::int32_t
{
  Top,
  WorldChildren,
  InteriorCellBlock,
  InteriorCellSubBlock,
  ExteriorCellBlock,
  ExteriorCellSubBlock,
  CellChildren,
  TopicChildren,
  CellPersistentChildren,
  CellTemporaryChildren,
  QuestChildren,
  CellVisibleDistantChildren = 10,
};

struct RecordFlags
{
  enum TES4Flag : std::uint32_t
  {
    Master    = 0x1,
    Optimized = 0x10,
    Localized = 0x80,
    SmallNew  = 0x100,
    SmallOld  = 0x200,
    Update    = 0x200,
    Medium    = 0x400,
    Blueprint = 0x800,
  };

  enum Flag : std::uint32_t
  {
    Compressed = 0x40000,
  };
};

struct RecordHeader
{
  Type type;
  std::uint32_t dataSize;
  union
  {
    struct
    {
      std::uint32_t flags;
      std::uint32_t formId;
    } formData;
    struct
    {
      std::uint32_t label;
      GroupType groupType;
    } groupData;
  };
  union
  {
    struct
    {
      std::uint32_t revision;
      Type firstChunk;
    } old;
    struct
    {
      std::uint16_t timestamp;
      std::uint16_t revision;
      std::uint16_t version;
      std::uint16_t unknown1;
    };
  };
};
static_assert(sizeof(RecordHeader) == 24);

struct ChunkHeader
{
  Type type;
  std::uint16_t dataSize;
};
static_assert(sizeof(ChunkHeader) == 6);

class GroupData final
{
public:
  constexpr GroupData() : formId_{0}, groupType_{GroupType::Top} {}

  constexpr GroupData(std::uint32_t label, GroupType type)
      : formId_{label}, groupType_{type}
  {}

  constexpr auto operator<=>(const GroupData& other) const
  {
    if (groupType_ != other.groupType_) {
      return groupType_ <=> other.groupType_;
    } else {
      if (hasFormType()) {
        return formType() <=> other.formType();
      } else if (hasBlock()) {
        return block() <=> other.block();
      } else if (hasGridCell()) {
        return gridCell() <=> other.gridCell();
      } else {
        return formId_ <=> other.formId_;
      }
    }
  }

  constexpr bool operator==(const GroupData& other) const
  {
    return groupType_ == other.groupType_ && formId_ == other.formId_;
  }

  [[nodiscard]] constexpr GroupType type() const { return groupType_; }

  [[nodiscard]] constexpr bool hasFormType() const
  {
    return groupType_ == GroupType::Top;
  }

  [[nodiscard]] constexpr bool hasParent() const
  {
    return groupType_ == GroupType::WorldChildren ||
           groupType_ == GroupType::CellChildren ||
           groupType_ == GroupType::TopicChildren ||
           groupType_ == GroupType::CellPersistentChildren ||
           groupType_ == GroupType::CellTemporaryChildren ||
           groupType_ == GroupType::QuestChildren;
  }

  [[nodiscard]] constexpr bool hasDirectParent() const
  {
    return groupType_ == GroupType::WorldChildren ||
           groupType_ == GroupType::CellChildren ||
           groupType_ == GroupType::TopicChildren ||
           groupType_ == GroupType::QuestChildren;
  }

  [[nodiscard]] constexpr bool hasBlock() const
  {
    return groupType_ == GroupType::InteriorCellBlock ||
           groupType_ == GroupType::InteriorCellSubBlock;
  }

  [[nodiscard]] constexpr bool hasGridCell() const
  {
    return groupType_ == GroupType::ExteriorCellBlock ||
           groupType_ == GroupType::ExteriorCellSubBlock;
  }

  [[nodiscard]] constexpr Type formType() const { return formType_; }

  [[nodiscard]] constexpr std::uint32_t parent() const { return formId_; }

  [[nodiscard]] constexpr std::int32_t block() const { return number_; }

  [[nodiscard]] constexpr std::pair<std::int16_t, std::int16_t> gridCell() const
  {
    return {cell_.x, cell_.y};
  }

  constexpr void setLocalIndex(std::uint8_t index)
  {
    if (hasParent()) {
      formId_ = (formId_ & 0xFFFFFFU) | (index << 24U);
    }
  }

private:
  GroupType groupType_;
  union
  {
    Type formType_;
    std::uint32_t formId_;
    std::int32_t number_;
    struct
    {
      std::int16_t y;
      std::int16_t x;
    } cell_;
  };
};

class FormData final
{
public:
  constexpr FormData(Type type, std::uint32_t flags, std::uint32_t formId)
      : type_{type}, flags_{flags}, formId_{formId}
  {}

  [[nodiscard]] constexpr Type type() const { return type_; }

  [[nodiscard]] constexpr std::uint32_t flags() const { return flags_; }

  [[nodiscard]] constexpr std::uint32_t formId() const { return formId_; }

  [[nodiscard]] constexpr std::uint8_t localModIndex() const { return formId_ >> 24; }

private:
  Type type_;
  std::uint32_t flags_;
  std::uint32_t formId_;
};

}  // namespace TESFile

using namespace TESFile::literals;

#endif  // TESFILE_STREAM_H
