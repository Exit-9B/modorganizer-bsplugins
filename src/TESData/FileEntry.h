#ifndef TESDATA_FILEENTRY_H
#define TESDATA_FILEENTRY_H

#include "Record.h"
#include "TESFile/Type.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace TESData
{

using TESFileHandle = int;

class FileEntry final
{
public:
  FileEntry(TESFileHandle handle, const std::string& name);

  [[nodiscard]] TESFileHandle handle() const;
  [[nodiscard]] const std::string& name() const;
  [[nodiscard]] std::vector<std::shared_ptr<Record>> records() const;

  std::shared_ptr<Record> createForm(std::uint32_t formId);

  void addForm(const std::string& master, std::uint32_t formId,
               std::shared_ptr<Record> record);
  void addSetting(const std::string& setting, std::shared_ptr<Record> record);
  void addDefaultObject(TESFile::Type type, std::shared_ptr<Record> record);

private:
  TESFileHandle m_Handle;
  std::string m_Name;
  std::map<std::string, std::map<std::uint32_t, std::shared_ptr<Record>>> m_Forms;
  std::map<std::string, std::shared_ptr<Record>> m_Settings;
  std::map<TESFile::Type, std::shared_ptr<Record>> m_DefaultObjects;
};

}  // namespace TESData

#endif  // TESDATA_FILEENTRY_H
