#ifndef TESDATA_BRANCHCONFLICTPARSER_H
#define TESDATA_BRANCHCONFLICTPARSER_H

#include "PluginList.h"
#include "RecordPath.h"

#include <istream>
#include <vector>

namespace TESData
{

class BranchConflictParser final
{
public:
  BranchConflictParser(PluginList* pluginList, const std::string& pluginName,
                       const RecordPath& path);

  bool Group(TESFile::GroupData group);
  void EndGroup();
  bool Form(TESFile::FormData form);
  void EndForm();
  bool Chunk(TESFile::Type type);
  void Data(std::istream& stream);

private:
  PluginList* m_PluginList;
  std::string m_PluginName;
  TESData::RecordPath m_Path;

  std::vector<std::string> m_Masters;
  RecordPath m_CurrentPath;
  TESFile::Type m_CurrentType;
  TESFile::Type m_CurrentChunk;
  std::string m_CurrentName;
};

}  // namespace TESData

#endif  // TESDATA_BRANCHCONFLICTPARSER_H
