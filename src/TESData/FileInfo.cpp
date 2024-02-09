#include "FileInfo.h"
#include "FileEntry.h"
#include "MOPlugin/Settings.h"
#include "PluginList.h"

#include <algorithm>

using namespace Qt::Literals::StringLiterals;

namespace TESData
{

FileInfo::FileInfo(PluginList* pluginList, const QString& name, bool forceLoaded,
                   bool forceEnabled, bool forceDisabled, bool lightSupported)
    : m_PluginList{pluginList},
      m_FileSystemData{
          .name               = name,
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

bool FileInfo::isMasterFile() const
{
  return m_Metadata.isMasterFlagged || m_FileSystemData.hasMasterExtension ||
         m_FileSystemData.hasLightExtension;
}

bool FileInfo::isSmallFile() const
{
  return m_Metadata.isLightFlagged || m_FileSystemData.hasLightExtension;
}

bool FileInfo::isAlwaysEnabled() const
{
  return m_FileSystemData.forceLoaded || m_FileSystemData.forceEnabled;
}

bool FileInfo::canBeToggled() const
{
  return !m_FileSystemData.forceLoaded && !m_FileSystemData.forceEnabled &&
         !m_FileSystemData.forceDisabled;
}

bool FileInfo::mustLoadAfter(const FileInfo& other) const
{
  const bool hasMaster = this->masters().contains(other.name(), Qt::CaseInsensitive);
  const bool isMaster  = other.masters().contains(this->name(), Qt::CaseInsensitive);

  if (hasMaster && !isMaster) {
    return true;
  } else if (isMaster) {
    return false;
  }

  if (other.forceLoaded() && !this->forceLoaded()) {
    return true;
  }

  if (other.isMasterFile() && !this->isMasterFile()) {
    return true;
  }

  return false;
}

static void checkConflict(QSet<int>& winning, QSet<int>& losing, const FileInfo& file,
                          const TESData::PluginList* pluginList,
                          TESFileHandle alternative, bool ignoreMasters)
{
  const auto entry      = pluginList->findEntryByName(file.name().toStdString());
  const auto otherEntry = pluginList->findEntryByHandle(alternative);
  if (otherEntry == nullptr || otherEntry == entry) {
    return;
  }

  const auto otherFile =
      pluginList->getPluginByName(QString::fromStdString(otherEntry->name()));
  if (otherFile == nullptr) {
    return;
  }

  const QString otherName = QString::fromStdString(otherEntry->name());
  const int otherIndex    = pluginList->getIndex(otherName);

  if (file.priority() > otherFile->priority()) {
    if (!ignoreMasters ||
        !file.masters().contains(otherFile->name(), Qt::CaseInsensitive)) {
      winning.insert(otherIndex);
    }
  } else {
    if (!ignoreMasters ||
        !otherFile->masters().contains(file.name(), Qt::CaseInsensitive)) {
      losing.insert(otherIndex);
    }
  }
}

FileInfo::Conflicts FileInfo::doConflictCheck() const
{
  Conflicts conflicts;

  const auto entry = m_PluginList->findEntryByName(name().toStdString());
  if (entry == nullptr) {
    return conflicts;
  }

  const bool ignoreMasters =
      Settings::instance()->get<bool>("ignore_master_conflicts", false);

  entry->forEachRecord([&](auto&& record) {
    if (record->ignored())
      return;

    for (const auto alternative : record->alternatives()) {
      checkConflict(conflicts.m_OverridingList, conflicts.m_OverriddenList, *this,
                    m_PluginList, alternative, ignoreMasters);
    }
  });

  for (const auto& archive : m_FileSystemData.archives) {
    const auto archiveEntry = m_PluginList->findArchive(archive);
    if (!archiveEntry) {
      continue;
    }

    archiveEntry->forEachMember([&](auto&& item) {
      for (const auto alternative : item->alternatives) {
        checkConflict(conflicts.m_OverwritingArchiveList,
                      conflicts.m_OverwrittenArchiveList, *this, m_PluginList,
                      alternative, ignoreMasters);
      }
    });
  }

  uint conflictState = CONFLICT_NONE;
  if (!conflicts.m_OverridingList.empty()) {
    conflictState |= CONFLICT_OVERRIDE;
  }
  if (!conflicts.m_OverriddenList.empty()) {
    conflictState |= CONFLICT_OVERRIDDEN;
  }
  if (!conflicts.m_OverwritingArchiveList.empty()) {
    conflictState |= CONFLICT_ARCHIVE_OVERWRITE;
  }
  if (!conflicts.m_OverwrittenArchiveList.empty()) {
    conflictState |= CONFLICT_ARCHIVE_OVERWRITTEN;
  }
  conflicts.m_CurrentConflictState = static_cast<EConflictFlag>(conflictState);

  return conflicts;
}

}  // namespace TESData
