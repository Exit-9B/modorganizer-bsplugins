#ifndef BSPLUGININFO_SINGLERECORDPARSER_H
#define BSPLUGININFO_SINGLERECORDPARSER_H

#include "DataItem.h"
#include "TESData/RecordPath.h"
#include "TESFile/Stream.h"

namespace BSPluginInfo
{

class SingleRecordParser final
{
public:
  SingleRecordParser(const TESData::RecordPath& path, const std::string& file,
                     DataItem* root, int index);

  bool Group(TESFile::GroupData group);
  bool Form(TESFile::FormData form);
  bool Chunk(TESFile::Type type);
  void Data(std::istream& stream);

private:
  TESData::RecordPath m_Path;
  std::string m_File;
  DataItem* m_DataRoot;
  int m_Index;

  std::vector<std::string> m_Masters;
  int m_Depth        = 0;
  bool m_Localized   = false;
  bool m_RecordFound = false;
  TESFile::Type m_CurrentChunk;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_SINGLERECORDPARSER_H
