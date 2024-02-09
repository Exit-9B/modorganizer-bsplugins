#ifndef BSPLUGINLIST_PLUGINLISTMODEL_H
#define BSPLUGINLIST_PLUGINLISTMODEL_H

#include "TESData/PluginList.h"

#include <QAbstractItemModel>

#include <functional>

namespace BSPluginList
{

class PluginListModel final : public QAbstractItemModel
{
  Q_OBJECT

public:
  friend class PluginGroupProxyModel;

  enum ItemDataRole
  {
    GroupingRole = Qt::UserRole,
    IndexRole,
    InfoRole,
    ConflictsIconRole,
    FlagsIconRole,
    OriginRole,
    OverridingRole,
    OverriddenRole,
    OverwritingAuxRole,
    OverwrittenAuxRole,
    ScrollMarkRole,
  };

  enum EColumn
  {
    COL_NAME,
    COL_CONFLICTS,
    COL_FLAGS,
    COL_PRIORITY,
    COL_MODINDEX,

    COL_COUNT
  };

  explicit PluginListModel(TESData::PluginList* plugins);

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole) override;

  Qt::DropActions supportedDropActions() const override;
  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                       int column, const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                    const QModelIndex& parent) override;

  [[nodiscard]] QStringList
  groups(std::function<bool(const TESData::FileInfo*)> pred = {}) const;
  [[nodiscard]] QStringList masterGroups() const;
  [[nodiscard]] QStringList regularGroups() const;

public slots:
  void refresh();

  void invalidate();
  void invalidateConflicts();

  void movePlugin(const QString& name, int oldPriority, int newPriority);

  void
  changePluginStates(const std::map<QString, MOBase::IPluginList::PluginStates>& infos);

  // enable/disable all plugins
  //
  void setEnabledAll(bool enabled);

  // enable/disable plugins at the given indices.
  //
  void setEnabled(const QModelIndexList& indices, bool enabled);

  // send plugins to the given priority
  //
  void sendToPriority(const QModelIndexList& indices, int priority,
                      bool disjoint = false);

  // shift the priority of mods at the given indices by the given offset
  //
  void shiftPluginsPriority(const QModelIndexList& indices, int offset);

  // toggle the active state of mods at the given indices
  //
  void toggleState(const QModelIndexList& indices);

  // assign plugins to a group
  //
  void setGroup(const QModelIndexList& indices, const QString& group);

  // send plugins to the bottom of a group
  //
  void sendToGroup(const QModelIndexList& indices, const QString& group, bool isESM);

signals:
  void pluginStatesChanged(const QModelIndexList& indices) const;
  void pluginOrderChanged() const;

private:
  [[nodiscard]] QVariant displayData(const QModelIndex& index) const;
  [[nodiscard]] QVariant checkstateData(const QModelIndex& index) const;
  [[nodiscard]] QVariant foregroundData(const QModelIndex& index) const;
  [[nodiscard]] QVariant backgroundData(const QModelIndex& index) const;
  [[nodiscard]] QVariant fontData(const QModelIndex& index) const;
  [[nodiscard]] QVariant alignmentData(const QModelIndex& index) const;
  [[nodiscard]] QVariant tooltipData(const QModelIndex& index) const;
  [[nodiscard]] QVariant conflictData(const QModelIndex& index) const;
  [[nodiscard]] QVariant iconData(const QModelIndex& index) const;

  TESData::PluginList* m_Plugins;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINLISTMODEL_H
