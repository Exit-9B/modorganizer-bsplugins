#ifndef TESDATA_FILEINFO_H
#define TESDATA_FILEINFO_H

#include <ifiletree.h>
#include <memoizedlock.h>

#include <boost/container/flat_set.hpp>

#include <QDateTime>
#include <QSet>
#include <QString>

namespace TESData
{

class PluginList;

class FileInfo
{
public:
  enum EConflictFlag : uint
  {
    CONFLICT_NONE                = 0x0,
    CONFLICT_OVERRIDE            = 0x1,
    CONFLICT_OVERRIDDEN          = 0x2,
    CONFLICT_ARCHIVE_OVERWRITE   = 0x4,
    CONFLICT_ARCHIVE_OVERWRITTEN = 0x8,

    CONFLICT_MIXED         = CONFLICT_OVERRIDE | CONFLICT_OVERRIDDEN,
    CONFLICT_ARCHIVE_MIXED = CONFLICT_ARCHIVE_OVERWRITE | CONFLICT_ARCHIVE_OVERWRITTEN,
  };

  struct FileSystemData
  {
    QString name;
    QString fullPath;
    QDateTime time;

    bool hasMasterExtension;
    bool hasLightExtension;

    bool forceLoaded;
    bool forceEnabled;
    bool forceDisabled;

    bool hasIni;
    boost::container::flat_set<QString, MOBase::FileNameComparator> archives;
  };

  struct Metadata
  {
    QString author;
    QString description;

    bool isMasterFlagged;
    bool isLightFlagged;
    bool isOverlayFlagged;
    bool hasNoRecords;

    boost::container::flat_set<QString, MOBase::FileNameComparator> masters;
    mutable boost::container::flat_set<QString, MOBase::FileNameComparator> masterUnset;
  };

  struct State
  {
    bool enabled;
    int priority = -1;
    QString index;
    int loadOrder;
    QString group;

    bool operator<(const State& other) const { return (loadOrder < other.loadOrder); }
  };

  struct Conflicts
  {
    EConflictFlag m_CurrentConflictState = CONFLICT_NONE;
    QSet<int> m_OverridingList;
    QSet<int> m_OverriddenList;
    QSet<int> m_OverwritingArchiveList;
    QSet<int> m_OverwrittenArchiveList;
  };

  FileInfo(PluginList* pluginList, const QString& name, bool forceLoaded,
           bool forceEnabled, bool forceDisabled, const QString& fullPath,
           bool lightSupported);

  [[nodiscard]] const QString& name() const { return m_FileSystemData.name; }

  [[nodiscard]] bool hasMasterExtension() const
  {
    return m_FileSystemData.hasMasterExtension;
  }

  [[nodiscard]] bool hasLightExtension() const
  {
    return m_FileSystemData.hasLightExtension;
  }

  [[nodiscard]] bool forceLoaded() const { return m_FileSystemData.forceLoaded; }
  [[nodiscard]] bool forceEnabled() const { return m_FileSystemData.forceEnabled; }
  [[nodiscard]] bool forceDisabled() const { return m_FileSystemData.forceDisabled; }

  [[nodiscard]] bool hasIni() const { return m_FileSystemData.hasIni; }
  void setHasIni(bool hasIni) { m_FileSystemData.hasIni = hasIni; }
  [[nodiscard]] const auto& archives() const { return m_FileSystemData.archives; }
  void addArchive(const QString& archive) { m_FileSystemData.archives.insert(archive); }

  [[nodiscard]] const QString& author() const { return m_Metadata.author; }
  void setAuthor(const QString& author) { m_Metadata.author = author; }
  [[nodiscard]] const QString& description() const { return m_Metadata.description; }
  void setDescription(const QString& text) { m_Metadata.description = text; }
  [[nodiscard]] bool isMasterFlagged() const { return m_Metadata.isMasterFlagged; }
  void setMasterFlagged(bool value) { m_Metadata.isMasterFlagged = value; }
  [[nodiscard]] bool isLightFlagged() const { return m_Metadata.isLightFlagged; }
  void setLightFlagged(bool value) { m_Metadata.isLightFlagged = value; }
  [[nodiscard]] bool isOverlayFlagged() const { return m_Metadata.isOverlayFlagged; }
  void setOverlayFlagged(bool value) { m_Metadata.isOverlayFlagged = value; }
  [[nodiscard]] bool hasNoRecords() const { return m_Metadata.hasNoRecords; }
  void setHasNoRecords(bool value) { m_Metadata.hasNoRecords = value; }

  [[nodiscard]] const auto& masters() const { return m_Metadata.masters; }
  void addMaster(const QString& master) { m_Metadata.masters.insert(master); }

  [[nodiscard]] bool hasMissingMasters() const
  {
    return !m_Metadata.masterUnset.empty();
  }

  [[nodiscard]] const auto& missingMasters() const { return m_Metadata.masterUnset; }

  template <std::ranges::input_range R>
  void setMissingMasters(R&& range) const
  {
    m_Metadata.masterUnset.clear();
    m_Metadata.masterUnset.insert(boost::container::ordered_unique_range,
                                  std::begin(range), std::end(range));
  }

  [[nodiscard]] bool enabled() const { return m_State.enabled; }
  void setEnabled(bool enabled) { m_State.enabled = enabled; }
  [[nodiscard]] int priority() const { return m_State.priority; }
  void setPriority(int priority)
  {
    m_State.priority = priority;
    m_Conflicts.invalidate();
  }
  [[nodiscard]] const QString& index() const { return m_State.index; }
  void setIndex(const QString& index) { m_State.index = index; }
  [[nodiscard]] int loadOrder() const { return m_State.loadOrder; }
  void setLoadOrder(int loadOrder) { m_State.loadOrder = loadOrder; }
  [[nodiscard]] const QString& group() const { return m_State.group; }
  void setGroup(const QString& group) { m_State.group = group; }

  [[nodiscard]] EConflictFlag conflictState() const
  {
    return m_Conflicts.value().m_CurrentConflictState;
  }

  [[nodiscard]] const auto& getPluginOverriding() const
  {
    return m_Conflicts.value().m_OverridingList;
  }

  [[nodiscard]] const auto& getPluginOverridden() const
  {
    return m_Conflicts.value().m_OverriddenList;
  }

  [[nodiscard]] const auto& getPluginOverwritingArchive() const
  {
    return m_Conflicts.value().m_OverwritingArchiveList;
  }

  [[nodiscard]] const auto& getPluginOverwrittenArchive() const
  {
    return m_Conflicts.value().m_OverwrittenArchiveList;
  }

  [[nodiscard]] bool isMasterFile() const;
  [[nodiscard]] bool isSmallFile() const;
  [[nodiscard]] bool isAlwaysEnabled() const;
  [[nodiscard]] bool canBeToggled() const;
  [[nodiscard]] bool mustLoadAfter(const FileInfo& other) const;

  void invalidateConflicts() const { m_Conflicts.invalidate(); }

private:
  [[nodiscard]] Conflicts doConflictCheck() const;

  PluginList* m_PluginList;
  FileSystemData m_FileSystemData;
  Metadata m_Metadata;
  State m_State;
  mutable MOBase::MemoizedLocked<Conflicts> m_Conflicts;
};

}  // namespace TESData

#endif  // TESDATA_FILEINFO_H
