#ifndef TESDATA_PLUGINLIST_H
#define TESDATA_PLUGINLIST_H

#include "FileEntry.h"
#include "FileInfo.h"
#include "MOTools/ILootCache.h"
#include "TESFile/Type.h"

#include <gameplugins.h>
#include <ifiletree.h>
#include <imoinfo.h>
#include <iplugingame.h>
#include <ipluginlist.h>
#include <memoizedlock.h>

#include <boost/signals2.hpp>

#include <QObject>

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace TESData
{

class PluginList final : public QObject,
                         public MOBase::IPluginList,
                         public MOTools::ILootCache
{
  Q_OBJECT

public:
  using SignalRefreshed   = boost::signals2::signal<void()>;
  using SignalPluginMoved = boost::signals2::signal<void(const QString&, int, int)>;
  using SignalPluginStateChanged =
      boost::signals2::signal<void(const std::map<QString, PluginStates>&)>;

  explicit PluginList(const MOBase::IOrganizer* moInfo);

  PluginList(const PluginList&) = delete;
  PluginList(PluginList&&)      = delete;

  ~PluginList() noexcept;

  PluginList& operator=(const PluginList&) = delete;
  PluginList& operator=(PluginList&&)      = delete;

  [[nodiscard]] int pluginCount() const;

  [[nodiscard]] const FileInfo* getPlugin(int index) const;
  [[nodiscard]] const FileInfo* getPluginByName(const QString& name) const;
  [[nodiscard]] const FileInfo* getPluginByPriority(int priority) const;
  [[nodiscard]] int getIndex(const QString& pluginName) const;
  [[nodiscard]] int getIndexAtPriority(int priority) const;
  [[nodiscard]] QString getOriginName(int index) const;

  [[nodiscard]] FileEntry* findEntryByName(const std::string& pluginName);
  [[nodiscard]] FileEntry* findEntryByHandle(TESFileHandle handle);

  FileEntry* createEntry(const std::string& name);
  void addSetting(const std::string& pluginName, const std::string& setting);
  void addForm(const std::string& pluginName, TESFile::Type type,
               const std::string& master, std::uint32_t formId);
  void addDefaultObject(const std::string& pluginName, TESFile::Type type);

  void refresh(bool invalidate = false);

  void setEnabled(int id, bool enable);
  void setEnabled(const std::vector<int>& ids, bool enable);
  void toggleState(const std::vector<int>& ids);

  [[nodiscard]] bool canMoveToPriority(const std::vector<int>& ids,
                                       int newPriority) const;
  void moveToPriority(std::vector<int> ids, int destination);
  void shiftPriority(const std::vector<int>& ids, int offset);

  void setGroup(const std::vector<int>& ids, const QString& group);

  [[nodiscard]] QStringList loadOrder() const;

  [[nodiscard]] bool isRefreshing() const { return m_Refreshing; }

  // IPluginList

  [[nodiscard]] QStringList pluginNames() const override;
  [[nodiscard]] PluginStates state(const QString& name) const override;
  void setState(const QString& name, PluginStates state) override;
  [[nodiscard]] int priority(const QString& name) const override;
  bool setPriority(const QString& name, int newPriority) override;
  [[nodiscard]] int loadOrder(const QString& name) const override;
  void setLoadOrder(const QStringList& pluginList) override;
  [[deprecated]] bool isMaster(const QString& name) const override;
  [[nodiscard]] QStringList masters(const QString& name) const override;
  [[nodiscard]] QString origin(const QString& name) const override;

  bool onRefreshed(const std::function<void()>& callback) override;
  bool
  onPluginMoved(const std::function<void(const QString&, int, int)>& func) override;
  bool onPluginStateChanged(
      const std::function<void(const std::map<QString, PluginStates>&)>& func) override;

  [[nodiscard]] bool hasMasterExtension(const QString& name) const override;
  [[nodiscard]] bool hasLightExtension(const QString& name) const override;
  [[nodiscard]] bool isMasterFlagged(const QString& name) const override;
  [[nodiscard]] bool isLightFlagged(const QString& name) const override;
  [[nodiscard]] bool isOverlayFlagged(const QString& name) const override;
  [[nodiscard]] bool hasNoRecords(const QString& name) const override;

  // ILootCache

  void clearAdditionalInformation() override;
  void addLootReport(const QString& name, MOTools::Loot::Plugin plugin) override;
  [[nodiscard]] const MOTools::Loot::Plugin* getLootReport(const QString& name) const;

public slots:
  void writePluginLists() const;

signals:
  void pluginsListChanged();

private:
  [[nodiscard]] FileInfo* findPlugin(const QString& name);
  [[nodiscard]] const FileInfo* findPlugin(const QString& name) const;

  void scanDataFiles(bool invalidate);
  void readPluginLists();

  [[nodiscard]] QString groupsPath() const;
  void clearGroups();
  void readGroups(const QString& fileName);
  void writeGroups(const QString& fileName) const;

  void pluginStatesChanged(const QStringList& pluginNames, PluginStates state) const;
  void enforcePluginRelationships();
  void testMasters();
  void updateCache();
  void computeCompileIndices();
  void refreshLoadOrder();

  const MOBase::IOrganizer* m_Organizer;

  std::vector<std::shared_ptr<FileInfo>> m_Plugins;

  std::map<QString, int, MOBase::FileNameComparator> m_PluginsByName;
  std::vector<int> m_PluginsByPriority;

  std::map<QString, MOTools::Loot::Plugin, MOBase::FileNameComparator> m_LootInfo;

  TESFileHandle m_NextHandle = 0;
  std::map<std::string, std::shared_ptr<FileEntry>> m_EntriesByName;
  std::map<TESFileHandle, std::shared_ptr<FileEntry>> m_EntriesByHandle;
  std::map<std::string, std::shared_ptr<Record>> m_Settings;
  std::map<TESFile::Type, std::shared_ptr<Record>> m_DefaultObjects;

  bool m_Refreshing = true;
  SignalRefreshed m_Refreshed;
  SignalPluginMoved m_PluginMoved;
  SignalPluginStateChanged m_PluginStateChanged;
};

}  // namespace TESData

#endif  // TESDATA_PLUGINLIST_H
