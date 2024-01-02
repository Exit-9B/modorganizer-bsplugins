#ifndef TESFILE_READER_H
#define TESFILE_READER_H

#include "Stream.h"

#include <concepts>
#include <cstdint>
#include <filesystem>
#include <istream>
#include <utility>

namespace TESFile
{

template <typename Handler>
concept ReaderHandler = requires(Handler& handler) {
  {
    handler.Group(std::declval<const GroupData&>())
  } -> std::convertible_to<bool>;
  {
    handler.Form(std::declval<const FormData&>())
  } -> std::convertible_to<bool>;
  {
    handler.Chunk(std::declval<Type>())
  } -> std::convertible_to<bool>;
  {
    handler.ChunkData(std::declval<Type>(), std::declval<std::istream&>())
  };
};

template <ReaderHandler Handler>
class Reader
{
public:
  void parse(const std::filesystem::path& path, Handler& handler);

  void parse(std::istream& stream, Handler& handler);

private:
  enum HeaderSize
  {
    HeaderSize_Standard  = sizeof(RecordHeader),
    HeaderSize_Oblivion  = 20,
    HeaderSize_Morrowind = 16,
  };

  std::uint32_t parsePluginInfo(std::istream& stream, Handler& handler);

  std::uint32_t parseRecord(std::istream& stream, Handler& handler);

  std::uint32_t handleForm(std::istream& stream, const RecordHeader& header,
                           Handler& handler);

  std::uint32_t handleGroup(std::istream& stream, const RecordHeader& header,
                            Handler& handler);

  std::uint32_t parseChunk(std::istream& stream, Handler& handler);

  TESFormat chunkFormat_;
  int headerSize_;
};

}  // namespace TESFile

#include "Reader.inl"

#endif  // TESFILE_READER_H
