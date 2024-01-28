#ifndef TESDATA_FILEREADERHANDLER_H
#define TESDATA_FILEREADERHANDLER_H

#include "RecordPath.h"
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

  bool Group(TESFile::GroupData group);
  void EndGroup();
  bool Form(TESFile::FormData form);
  void EndForm();
  bool Chunk(TESFile::Type type);
  void Data(std::istream& stream);

private:
  void MainRecordData(std::istream& stream);
  void DefaultObjectData(std::istream& stream);
  void GameSettingData(std::istream& stream);
  void StandardData(std::istream& stream);

  PluginList* m_PluginList;
  FileInfo* m_Plugin;
  bool m_LightSupported;
  bool m_OverlaySupported;

  std::vector<std::string> m_Masters;
  RecordPath m_CurrentPath;
  TESFile::Type m_CurrentType;
  TESFile::Type m_CurrentChunk;
  std::string m_CurrentName;
};

}  // namespace TESData

#endif  // TESDATA_FILEREADERHANDLER_H
