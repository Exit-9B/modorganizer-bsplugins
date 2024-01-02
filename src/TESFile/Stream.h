#ifndef TESFILE_STREAM_H
#define TESFILE_STREAM_H

#include "Type.h"

#include <cstdint>
#include <cstring>
#include <istream>
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
  CellVisibleDistantChildren,
};

struct RecordFlags
{
  enum TES4Flag : std::uint32_t
  {
    Master = 0x1,
    LightNew = 0x100, // Starfield ESL flag
    LightOld = 0x200, // SSE/F4 ESL flag
    Overlay = 0x200, // Starfield ESQ flag
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

class GroupData
{
public:
  GroupData(std::uint32_t label, GroupType type) : formId_{label}, groupType_{type} {}

  [[nodiscard]] bool hasFormType() const { return groupType_ == GroupType::Top; }

  [[nodiscard]] bool hasParent() const
  {
    return groupType_ == GroupType::WorldChildren ||
           groupType_ == GroupType::CellChildren ||
           groupType_ == GroupType::TopicChildren ||
           groupType_ == GroupType::CellPersistentChildren ||
           groupType_ == GroupType::CellTemporaryChildren ||
           groupType_ == GroupType::CellVisibleDistantChildren;
  }

  [[nodiscard]] bool hasBlock() const
  {
    return groupType_ == GroupType::InteriorCellBlock ||
           groupType_ == GroupType::InteriorCellSubBlock;
  }

  [[nodiscard]] bool hasGridCell() const
  {
    return groupType_ == GroupType::ExteriorCellBlock ||
           groupType_ == GroupType::ExteriorCellSubBlock;
  }

  [[nodiscard]] Type formType() const { return formType_; }

  [[nodiscard]] std::uint32_t parent() const { return formId_; }

  [[nodiscard]] std::int32_t block() const { return number_; }

  [[nodiscard]] std::pair<std::uint16_t, std::uint16_t> gridCell() const
  {
    return {cell_.y, cell_.x};
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

class FormData
{
public:
  FormData(Type type, std::uint32_t flags, std::uint32_t formId)
      : type_{type}, flags_{flags}, formId_{formId}
  {}

  [[nodiscard]] Type type() const { return type_; }

  [[nodiscard]] std::uint32_t flags() const { return flags_; }

  [[nodiscard]] std::uint32_t formId() const { return formId_; }

  [[nodiscard]] std::uint8_t localModIndex() const { return formId_ >> 24; }

private:
  Type type_;
  std::uint32_t flags_;
  std::uint32_t formId_;
};

}  // namespace TESFile

using namespace TESFile::literals;

#endif  // TESFILE_STREAM_H
