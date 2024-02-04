#ifndef TESDATA_SINGLERECORDPARSER_H
#define TESDATA_SINGLERECORDPARSER_H

#include "DataItem.h"
#include "RecordPath.h"
#include "TESFile/Stream.h"

#include <iplugingame.h>

#include <coroutine>
#include <functional>
#include <memory>
#include <vector>

namespace TESData
{

class IFormParser;

class SingleRecordParser final
{
public:
  SingleRecordParser(const QString& gameName, const RecordPath& path,
                     const std::string& file, DataItem* root, int index);

  bool Group(TESFile::GroupData group);
  bool Form(TESFile::FormData form);
  bool Chunk(TESFile::Type type);
  void Data(std::istream& stream);

private:
  QString m_GameName;
  RecordPath m_Path;
  std::string m_File;
  DataItem* m_DataRoot;
  int m_FileIndex;

  TESFile::Type m_CurrentType;
  std::coroutine_handle<> m_ParseTask;
  std::istream* m_ChunkStream;

  std::vector<std::string> m_Masters;
  int m_Depth        = 0;
  bool m_Localized   = false;
  bool m_RecordFound = false;
  TESFile::Type m_CurrentChunk;
};

}  // namespace TESData

#endif  // TESDATA_SINGLERECORDPARSER_H
