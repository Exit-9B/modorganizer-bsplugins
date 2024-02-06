#include "Reader.h"

#include <fmt/format.h>
#include <zlib.h>

#include <cstring>
#include <fstream>
#include <stdexcept>

namespace TESFile
{

template <ReaderHandler Handler>
inline void Reader<Handler>::parse(const std::filesystem::path& path, Handler& handler)
{
  std::ifstream stream;
  stream.open(path, std::ios_base::binary | std::ios_base::in);
  if (!stream.good()) {
    throw std::runtime_error(std::strerror(errno));
  }
  parse(stream, handler);
}

template <ReaderHandler Handler>
inline void Reader<Handler>::parse(std::istream& stream, Handler& handler)
{
  parsePluginInfo(stream, handler);
  while (stream.good() && stream.peek() != std::char_traits<char>::eof()) {
    parseRecord(stream, handler);
  }
}

template <ReaderHandler Handler>
inline std::uint32_t Reader<Handler>::parsePluginInfo(std::istream& stream,
                                                      Handler& handler)
{
  const RecordHeader header = readType<RecordHeader>(stream);
  if (stream.fail()) {
    throw std::runtime_error("record incomplete");
  }

  if (header.type == "TES4"_ts) {
    if (header.old.firstChunk == "HEDR"_ts) {
      chunkFormat_ = TESFormat::Oblivion;
      headerSize_  = HeaderSize_Oblivion;
      stream.seekg(HeaderSize_Oblivion - sizeof(RecordHeader), std::istream::cur);
    } else {
      chunkFormat_ = TESFormat::Standard;
      headerSize_  = sizeof(RecordHeader);
    }
  } else if (header.type == "TES3"_ts) {
    chunkFormat_ = TESFormat::Morrowind;
    headerSize_  = HeaderSize_Morrowind;
    stream.seekg(HeaderSize_Morrowind - sizeof(RecordHeader), std::istream::cur);
  } else {
    throw std::runtime_error(
        fmt::format("Unrecognized plugin info type: '{}'", header.type.value));
  }

  return handleForm(stream, header, handler);
}

template <ReaderHandler Handler>
inline std::uint32_t Reader<Handler>::parseRecord(std::istream& stream,
                                                  Handler& handler)
{
  RecordHeader header;
  stream.read(reinterpret_cast<char*>(&header), headerSize_);
  if (stream.fail()) {
    throw std::runtime_error("record incomplete");
  }

  if (header.type == "GRUP"_ts) {
    return handleGroup(stream, header, handler);
  } else {
    return handleForm(stream, header, handler);
  }
}

template <ReaderHandler Handler>
inline std::uint32_t Reader<Handler>::handleForm(std::istream& stream,
                                                 const RecordHeader& header,
                                                 Handler& handler)
{
  std::uint32_t dataSize = header.dataSize;
  const bool compressed  = header.formData.flags & RecordFlags::Compressed;
  if (handler.Form(
          FormData(header.type, header.formData.flags, header.formData.formId))) {

    std::string data;
    data.resize(dataSize);
    stream.read(data.data(), dataSize);
    if (stream.fail()) {
      throw std::runtime_error("chunk incomplete");
    }

    std::istringstream chunkstream;
    if (!compressed) {
      chunkstream.str(std::move(data));
    } else {
      if (data.size() < 4) {
        throw std::runtime_error("chunk incomplete");
      }

      std::memcpy(&dataSize, data.data(), sizeof(dataSize));

      std::string inflated;
      inflated.resize(dataSize);

      int zret;
      ::z_stream zstreambuf{
          .next_in = reinterpret_cast<z_const ::Bytef*>(data.data() + sizeof(dataSize)),
          .avail_in  = static_cast<::uInt>(data.size() - sizeof(dataSize)),
          .next_out  = reinterpret_cast<::Bytef*>(inflated.data()),
          .avail_out = static_cast<::uInt>(inflated.size()),
          .zalloc    = Z_NULL,
          .zfree     = Z_NULL,
          .opaque    = Z_NULL,
      };
      zret = ::inflateInit(&zstreambuf);
      if (zret != Z_OK) {
        throw std::runtime_error("zlib failed to init");
      }

      zret = ::inflate(&zstreambuf, Z_FINISH);
      if (zret != Z_STREAM_END) {
        throw std::runtime_error("zlib failed to read data");
      }
      zret = ::inflateEnd(&zstreambuf);

      chunkstream.str(std::move(inflated));
    }

    while (dataSize != 0) {
      const std::uint32_t fieldSize = parseChunk(chunkstream, handler);

      if (fieldSize > dataSize) {
        throw std::runtime_error(
            fmt::format("Subrecord exceeded record size ({}-{})", dataSize, fieldSize));
      }

      dataSize -= fieldSize;
    }

    if constexpr (requires { handler.EndForm(); }) {
      handler.EndForm();
    }
  } else {
    stream.seekg(dataSize, std::istream::cur);
    if (stream.fail()) {
      throw std::runtime_error("record incomplete");
    }
  }

  return headerSize_ + header.dataSize;
}

template <ReaderHandler Handler>
inline std::uint32_t Reader<Handler>::handleGroup(std::istream& stream,
                                                  const RecordHeader& header,
                                                  Handler& handler)
{
  std::uint32_t dataSize = header.dataSize - headerSize_;

  if (handler.Group(GroupData(header.groupData.label, header.groupData.groupType))) {

    while (dataSize != 0) {
      const std::uint32_t recordSize = parseRecord(stream, handler);

      if (recordSize > dataSize) {
        throw std::runtime_error(
            fmt::format("Record exceeded group size ({}-{})", dataSize, recordSize));
      }

      dataSize -= recordSize;
    }

    if constexpr (requires { handler.EndGroup(); }) {
      handler.EndGroup();
    }
  } else {
    stream.seekg(dataSize, std::istream::cur);
    if (stream.fail()) {
      throw std::runtime_error("group incomplete");
    }
  }

  return header.dataSize;
}

template <ReaderHandler Handler>
inline std::uint32_t Reader<Handler>::parseChunk(std::istream& stream, Handler& handler)
{
  ChunkHeader header = readType<ChunkHeader>(stream);
  if (stream.fail()) {
    throw std::runtime_error("chunk header incomplete");
  }
  std::uint32_t readSize = sizeof(ChunkHeader) + header.dataSize;

  std::uint32_t dataSize = header.dataSize;
  if (header.type == "XXXX"_ts) {
    if (dataSize != 4) {
      throw std::runtime_error("XXXX field has invalid size");
    }
    dataSize = readType<std::uint32_t>(stream);
    header   = readType<ChunkHeader>(stream);
    if (stream.fail()) {
      throw std::runtime_error("chunk size incomplete");
    }
    readSize += sizeof(ChunkHeader) + dataSize;
  }

  if (handler.Chunk(header.type)) {
    std::string field;
    field.resize(dataSize);
    stream.read(field.data(), dataSize);
    if (stream.fail()) {
      throw std::runtime_error("chunk data incomplete");
    }
    std::istringstream data(std::move(field));
    data.exceptions(std::ios_base::failbit);
    handler.Data(data);
  } else {
    stream.seekg(dataSize, std::istream::cur);
    if (stream.fail()) {
      throw std::runtime_error("chunk incomplete");
    }
  }

  return readSize;
}

}  // namespace TESFile
