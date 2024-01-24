#include "FileEntry.h"

namespace TESData
{

FileEntry::FileEntry(TESFileHandle handle, const std::string& name)
    : m_Handle{handle}, m_Name{name}
{}

TESFileHandle FileEntry::handle() const
{
  return m_Handle;
}

const std::string& FileEntry::name() const
{
  return m_Name;
}

void FileEntry::forEachRecord(
    std::function<void(const std::shared_ptr<const Record>&)> func) const
{
  for (const auto& [master, forms] : m_Forms) {
    for (const auto& [formId, record] : forms) {
      func(record);
    }
  }

  for (const auto& [setting, record] : m_Settings) {
    func(record);
  }

  for (const auto& [type, record] : m_DefaultObjects) {
    func(record);
  }
}

std::shared_ptr<Record> FileEntry::createForm(std::uint32_t formId)
{
  auto& forms   = m_Forms[m_Name];
  const auto it = forms.find(formId);
  if (it != forms.end()) {
    return it->second;
  }

  const auto record = std::make_shared<Record>();
  forms[formId]     = record;
  record->addAlternative(m_Handle);
  return record;
}

void FileEntry::addForm(const std::string& master, std::uint32_t formId,
                        std::shared_ptr<Record> record)
{
  m_Forms[master][formId] = record;
  record->addAlternative(m_Handle);
}

void FileEntry::addSetting(const std::string& setting, std::shared_ptr<Record> record)
{
  m_Settings[setting] = record;
  record->addAlternative(m_Handle);
}

void FileEntry::addDefaultObject(TESFile::Type type, std::shared_ptr<Record> record)
{
  m_DefaultObjects[type] = record;
  record->addAlternative(m_Handle);
}

}  // namespace TESData
