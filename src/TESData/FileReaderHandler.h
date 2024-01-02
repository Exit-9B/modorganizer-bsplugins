#ifndef TESDATA_FILEREADERHANDLER_H
#define TESDATA_FILEREADERHANDLER_H

#include "TESFile/Stream.h"

#include <string>
#include <vector>

namespace TESData
{

class PluginList;
class FileInfo;

class FileReaderHandler final
{
public:
  FileReaderHandler(PluginList* pluginList, FileInfo* plugin, bool lightSupported,
                    bool overlaySupported);

  bool Group(const TESFile::GroupData& group);

  bool Form(const TESFile::FormData& form);

  bool Chunk(TESFile::Type type);

  void ChunkData(TESFile::Type type, std::istream& stream);

private:
  static constexpr TESFile::Type NO_TYPE = "\0\0\0\0"_ts;

  PluginList* m_PluginList;
  FileInfo* m_Plugin;
  bool m_LightSupported;
  bool m_OverlaySupported;

  std::vector<std::string> m_Masters;
  TESFile::Type m_CurrentGroup;
};

}  // namespace TESData

#endif  // TESDATA_FILEREADERHANDLER_H
