#include "FileInfo.h"
#include "FileEntry.h"
#include "PluginList.h"

using namespace Qt::Literals::StringLiterals;

namespace TESData
{

FileInfo::FileInfo(PluginList* pluginList, const QString& name, bool forceLoaded,
                   bool forceEnabled, bool forceDisabled, const QString& fullPath,
                   bool lightSupported)
    : m_PluginList{pluginList},
      m_FileSystemData{
          .name               = name,
          .fullPath           = fullPath,
          .hasMasterExtension = name.endsWith(u".esm"_s, Qt::CaseInsensitive),
          .hasLightExtension =
              lightSupported && name.endsWith(u".esl"_s, Qt::CaseInsensitive),
          .forceLoaded   = forceLoaded,
          .forceEnabled  = forceEnabled,
          .forceDisabled = forceDisabled,
      },
      m_Conflicts{[this]() {
        return doConflictCheck();
      }}
{}

bool FileInfo::isPrimaryFile() const
{
  return m_FileSystemData.forceLoaded || m_FileSystemData.forceEnabled;
}

bool FileInfo::isMasterFile() const
{
  return m_Metadata.isMasterFlagged || m_FileSystemData.hasMasterExtension ||
         m_FileSystemData.hasLightExtension;
}

bool FileInfo::isSmallFile() const
{
  return m_Metadata.isLightFlagged || m_FileSystemData.hasLightExtension;
}

bool FileInfo::mustLoadAfter(const FileInfo& other) const
{
  const bool hasMaster = this->masters().contains(other.name());
  const bool isMaster  = other.masters().contains(this->name());

  if (hasMaster && !isMaster) {
    return true;
  } else if (isMaster) {
    return false;
  }

  if (other.isPrimaryFile() && !this->isPrimaryFile()) {
    return true;
  }

  if (other.isMasterFile() && !this->isMasterFile()) {
    return true;
  }

  return false;
}

FileInfo::Conflicts FileInfo::doConflictCheck() const
{
  Conflicts conflicts;

  const auto entry = m_PluginList->findEntryByName(name().toStdString());
  if (entry == nullptr) {
    return conflicts;
  }

  for (const auto& record : entry->records()) {
    for (const auto& alternative : record->alternatives()) {
      const auto altEntry = m_PluginList->findEntryByHandle(alternative);

      if (altEntry == nullptr || altEntry == entry) {
        continue;
      }

      const auto altInfo = m_PluginList->getPluginByName(altEntry->name().c_str());
      if (altInfo == nullptr) {
        continue;
      }

      QString altName{altEntry->name().c_str()};
      const int altIndex = m_PluginList->getIndex(altName);

      if (priority() > altInfo->priority()) {
        conflicts.m_OverridingList.insert(altIndex);
      } else {
        conflicts.m_OverriddenList.insert(altIndex);
      }
    }
  }

  uint conflictState = CONFLICT_NONE;
  if (!conflicts.m_OverridingList.empty()) {
    conflictState |= CONFLICT_OVERRIDE;
  }
  if (!conflicts.m_OverriddenList.empty()) {
    conflictState |= CONFLICT_OVERRIDDEN;
  }
  conflicts.m_CurrentConflictState = static_cast<EConflictFlag>(conflictState);

  return conflicts;
}

}  // namespace TESData
