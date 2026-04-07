#ifndef TESDATA_FILECONFLICTPARSER_H
#define TESDATA_FILECONFLICTPARSER_H

#include "RecordPath.h"
#include "TESFile/Stream.h"

#include <string>
#include <vector>

namespace TESData
{

class PluginList;
class FileInfo;

class FileConflictParser final
{
public:
  FileConflictParser(PluginList* pluginList, FileInfo* plugin, bool lightSupported,
                     bool mediumSupported, bool blueprintSupported);

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
  bool m_MediumSupported;
  bool m_BlueprintSupported;

  std::string m_PluginName;
  std::vector<std::string> m_Masters;
  RecordPath m_CurrentPath;
  TESFile::Type m_CurrentType;
  TESFile::Type m_CurrentChunk;
  std::string m_CurrentName;
};

}  // namespace TESData

#endif  // TESDATA_FILECONFLICTPARSER_H
