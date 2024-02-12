#ifndef BSPLUGINLIST_PLUGINGROUPPROXYMODEL_H
#define BSPLUGINLIST_PLUGINGROUPPROXYMODEL_H

#include "TESData/FileInfo.h"

#include <imoinfo.h>

#include <boost/container/flat_map.hpp>

#include <QAbstractProxyModel>

#include <limits>
#include <memory>
#include <vector>

namespace BSPluginList
{

class PluginGroupProxyModel final : public QAbstractProxyModel
{
  Q_OBJECT

public:
  explicit PluginGroupProxyModel(MOBase::IOrganizer* organizer,
                                 QObject* parent = nullptr);

  void setSourceModel(QAbstractItemModel* sourceModel) override;

  bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;

  QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
  QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role = Qt::EditRole) override;
  QModelIndex buddy(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  QMimeData* mimeData(const QModelIndexList& indexes) const override;
  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                       int column, const QModelIndex& parent) const override;
  bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column,
                    const QModelIndex& parent) override;

  void setDroppingBelowExpandedItem(bool value) { m_DroppingBelowExpandedItem = value; }

signals:
  void groupRenameRequested(const QModelIndex& index, const QString& name);

private slots:
  void onSourceDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                           const QList<int>& roles = QList<int>());
  void onSourceLayoutChanged(
      const QList<QPersistentModelIndex>& parents = QList<QPersistentModelIndex>(),
      QAbstractItemModel::LayoutChangeHint hint =
          QAbstractItemModel::NoLayoutChangeHint);
  void onSourceModelReset();
  void onSourceRowsInserted(const QModelIndex& parent);
  void onSourceRowsRemoved(const QModelIndex& parent);

private:
  static constexpr std::size_t NO_ID = std::numeric_limits<std::size_t>::max();

  struct Group
  {
    QString name;
    std::vector<std::size_t> children;

    explicit Group(const QString& name) : name{name} {}
  };

  struct ProxyItem
  {
    int row;
    int sourceRow;
    std::size_t parentId;
    std::shared_ptr<Group> groupInfo;

    [[nodiscard]] bool isSourceItem() const { return sourceRow != -1; }
    [[nodiscard]] bool isGroup() const { return groupInfo != nullptr; }
    [[nodiscard]] bool isDivider() const
    {
      return groupInfo == nullptr && sourceRow == -1;
    }
  };

  [[nodiscard]] int mapLowerBoundToSourceRow(std::size_t id) const;
  [[nodiscard]] bool isAboveDivider(std::size_t id) const;
  [[nodiscard]] bool isBelowDivider(std::size_t id) const;

  void buildGroups();
  [[nodiscard]] std::size_t createItem(const QString& name, int row, int sourceRow,
                                       std::size_t parent, std::shared_ptr<Group> group,
                                       int repeat = 0);

  std::vector<ProxyItem> m_ProxyItems;
  boost::container::flat_multimap<QString, std::size_t> m_ItemMap;
  std::vector<std::size_t> m_TopLevel;
  std::vector<std::size_t> m_SourceMap;

  bool m_DroppingBelowExpandedItem;
  mutable std::vector<std::size_t> m_DraggingGroups;

  MOBase::IOrganizer* m_Organizer;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINGROUPPROXYMODEL_H
