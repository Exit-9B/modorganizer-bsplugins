#include "PluginGroupProxyModel.h"
#include "PluginListDropInfo.h"
#include "PluginListModel.h"

#include <QSortFilterProxyModel>

namespace BSPluginList
{

PluginGroupProxyModel::PluginGroupProxyModel(MOBase::IOrganizer* organizer,
                                             QObject* parent)
    : QAbstractProxyModel(parent), m_Organizer{organizer}
{}

void PluginGroupProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  emit beginResetModel();

  if (const auto oldSource = this->sourceModel()) {
    disconnect(oldSource, nullptr, this, nullptr);
  }

  QAbstractProxyModel::setSourceModel(sourceModel);

  if (sourceModel) {
    connect(sourceModel, &QAbstractItemModel::layoutChanged, this,
            &PluginGroupProxyModel::onSourceLayoutChanged, Qt::UniqueConnection);
    connect(sourceModel, &QAbstractItemModel::rowsInserted, this,
            &PluginGroupProxyModel::onSourceRowsInserted, Qt::UniqueConnection);
    connect(sourceModel, &QAbstractItemModel::rowsRemoved, this,
            &PluginGroupProxyModel::onSourceRowsRemoved, Qt::UniqueConnection);
    connect(sourceModel, &QAbstractItemModel::modelReset, this,
            &PluginGroupProxyModel::onSourceModelReset, Qt::UniqueConnection);
    connect(sourceModel, &QAbstractItemModel::dataChanged, this,
            &PluginGroupProxyModel::onSourceDataChanged, Qt::UniqueConnection);

    buildGroups();
  }

  emit endResetModel();
}

bool PluginGroupProxyModel::hasChildren(const QModelIndex& parent) const
{
  return rowCount(parent) > 0;
}

int PluginGroupProxyModel::rowCount(const QModelIndex& parent) const
{
  if (!parent.isValid()) {
    return static_cast<int>(m_TopLevel.size());
  } else {
    const auto& item  = m_ProxyItems.at(parent.internalId());
    const auto& group = item.groupInfo;
    return group ? static_cast<int>(group->children.size()) : 0;
  }
}

int PluginGroupProxyModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return sourceModel()->columnCount();
}

QModelIndex PluginGroupProxyModel::index(int row, int column,
                                         const QModelIndex& parent) const
{
  if (row < 0 || column < 0) {
    return QModelIndex();
  }

  if (parent.isValid()) {
    const auto& item = m_ProxyItems.at(parent.internalId());
    if (const auto& group = item.groupInfo) {
      if (row < group->children.size()) {
        const auto id = group->children[row];
        return createIndex(row, column, id);
      }
    }
  } else {
    if (row < m_TopLevel.size()) {
      const auto id = m_TopLevel[row];
      return createIndex(row, column, id);
    }
  }
  return QModelIndex();
}

QModelIndex PluginGroupProxyModel::parent(const QModelIndex& index) const
{
  const auto& item    = m_ProxyItems.at(index.internalId());
  const auto parentId = item.parentId;
  if (parentId == NO_ID) {
    return QModelIndex();
  } else {
    const auto& parent = m_ProxyItems.at(parentId);
    return createIndex(parent.row, 0, parentId);
  }
}

QModelIndex PluginGroupProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
  if (!sourceIndex.isValid()) {
    return QModelIndex();
  }

  const auto id    = m_SourceMap[sourceIndex.row()];
  const auto& item = m_ProxyItems.at(id);
  QModelIndex parentIndex;
  if (item.parentId != NO_ID) {
    const auto& parentItem = m_ProxyItems.at(item.parentId);
    parentIndex            = index(parentItem.row, 0);
  }
  return index(item.row, sourceIndex.column(), parentIndex);
}

QModelIndex PluginGroupProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
  if (!proxyIndex.isValid()) {
    return QModelIndex();
  }

  const auto& item = m_ProxyItems.at(proxyIndex.internalId());
  return sourceModel()->index(item.sourceRow, proxyIndex.column());
}

Qt::ItemFlags PluginGroupProxyModel::flags(const QModelIndex& index) const
{
  if (!index.isValid()) {
    return QAbstractProxyModel::flags(index);
  }

  const auto& item = m_ProxyItems.at(index.internalId());
  if (!item.isGroup()) {
    if (item.isSourceItem()) {
      return sourceModel()->flags(mapToSource(index)) | Qt::ItemNeverHasChildren;
    } else {
      return Qt::ItemNeverHasChildren;
    }
  }

  Qt::ItemFlags result = QAbstractProxyModel::flags(index);
  if (index.column() == PluginListModel::COL_NAME) {
    result |= Qt::ItemIsEditable;
  }
  result |= Qt::ItemIsSelectable;
  result |= Qt::ItemIsDragEnabled;
  result |= Qt::ItemIsDropEnabled;
  result |= Qt::ItemIsEnabled;
  return result;
}

static QVariant groupData(const QString& name, int column, int role)
{
  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    switch (column) {
    case PluginListModel::COL_NAME:
      return name;
    default:
      return QVariant();
    }
  case Qt::FontRole: {
    QFont result;
    if (column == PluginListModel::COL_NAME) {
      result.setItalic(true);
      result.setBold(true);
    }
    return result;
  }
  case Qt::TextAlignmentRole:
    return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
  default:
    return QVariant();
  }
}

QVariant PluginGroupProxyModel::data(const QModelIndex& index, int role) const
{
  const auto& item = m_ProxyItems.at(index.internalId());
  if (item.isSourceItem()) {
    return sourceModel()->data(mapToSource(index), role);
  }

  if (const auto& group = item.groupInfo) {
    return groupData(group->name, index.column(), role);
  }

  return QVariant();
}

bool PluginGroupProxyModel::setData(const QModelIndex& index, const QVariant& value,
                                    int role)
{
  const auto& item = m_ProxyItems.at(index.internalId());
  if (item.isSourceItem()) {
    return sourceModel()->setData(mapToSource(index), value, role);
  }

  if (const auto& group = item.groupInfo) {
    if (role == Qt::EditRole) {
      if (index.column() == 0) {
        const QString valueString = value.toString();
        if (valueString.isEmpty())
          return false;

        emit groupRenameRequested(index, valueString);
      }
    }
  }

  return false;
}

QModelIndex PluginGroupProxyModel::buddy(const QModelIndex& index) const
{
  const auto& item = m_ProxyItems.at(index.internalId());
  if (item.isSourceItem()) {
    return mapFromSource(sourceModel()->buddy(mapToSource(index)));
  }

  return index;
}

QVariant PluginGroupProxyModel::headerData(int section, Qt::Orientation orientation,
                                           int role) const
{
  return sourceModel()->headerData(section, orientation, role);
}

int PluginGroupProxyModel::mapLowerBoundToSourceRow(std::size_t id) const
{
  const auto& item = m_ProxyItems.at(id);
  if (item.isSourceItem()) {
    return item.sourceRow;
  } else if (item.isGroup()) {
    const auto child      = item.groupInfo->children.front();
    const auto& childItem = m_ProxyItems.at(child);
    return childItem.sourceRow;
  } else {
    if (item.row + 1 == m_TopLevel.size()) {
      return -1;
    }
    const auto next = m_TopLevel.at(item.row + 1);
    return mapLowerBoundToSourceRow(next);
  }
}

bool PluginGroupProxyModel::isAboveDivider(std::size_t id) const
{
  const auto& item = m_ProxyItems.at(id);
  return item.isDivider();
}

bool PluginGroupProxyModel::isBelowDivider(std::size_t id) const
{
  const auto& item = m_ProxyItems.at(id);
  if (item.parentId != NO_ID) {
    return item.row == 0 && isBelowDivider(item.parentId);
  } else {
    return item.row > 0 && m_ProxyItems.at(m_TopLevel.at(item.row - 1)).isDivider();
  }
}

QMimeData* PluginGroupProxyModel::mimeData(const QModelIndexList& indexes) const
{
  m_DraggingGroups.clear();

  QModelIndexList sourceIndexes;

  for (const auto& idx : indexes) {
    const auto sourceIndex = mapToSource(idx);
    if (sourceIndex.isValid()) {
      sourceIndexes.append(sourceIndex);
    } else {
      m_DraggingGroups.push_back(idx.internalId());
      for (int i = 0, count = rowCount(idx); i < count; ++i) {
        sourceIndexes.append(mapToSource(index(i, 0, idx)));
      }
    }
  }

  return sourceModel()->mimeData(sourceIndexes);
}

bool PluginGroupProxyModel::canDropMimeData(const QMimeData* data,
                                            Qt::DropAction action, int row, int column,
                                            const QModelIndex& parent_) const
{
  // HACK: fix drop below expanded item
  auto parent = parent_;
  if (m_DroppingBelowExpandedItem) {
    parent = index(row - 1, column, parent_);
    row    = 0;
  }

  const bool draggedOntoGroup = parent.isValid() && row == -1;
  const bool draggedToBottom  = parent.isValid() && row == rowCount(parent);
  const auto idx      = draggedOntoGroup || draggedToBottom ? index(parent.row() + 1, 0)
                                                            : index(row, column, parent);
  const int sourceRow = idx.isValid() ? mapLowerBoundToSourceRow(idx.internalId()) : -1;

  bool canDrop = true;
  if (isBelowDivider(idx.internalId())) {
    canDrop = canDrop && sourceModel()->canDropMimeData(data, action, sourceRow + 1, 0,
                                                        QModelIndex());
  }
  if (isAboveDivider(idx.internalId())) {
    canDrop = canDrop && sourceModel()->canDropMimeData(data, action, sourceRow - 1, 0,
                                                        QModelIndex());
  }

  return canDrop &&
         sourceModel()->canDropMimeData(data, action, sourceRow, 0, QModelIndex());
}

template <typename T>
static T* findBaseModel(QAbstractItemModel* sourceModel)
{
  for (auto nextModel = sourceModel; nextModel;) {
    const auto proxyModel = qobject_cast<QAbstractProxyModel*>(nextModel);
    if (proxyModel) {
      nextModel = proxyModel->sourceModel();
    } else {
      if (const auto baseModel = qobject_cast<T*>(nextModel)) {
        return baseModel;
      }
      nextModel = nullptr;
    }
  }
  return nullptr;
}

bool PluginGroupProxyModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                         int row, int column,
                                         const QModelIndex& parent_)
{
  // HACK: fix drop below expanded item
  auto parent = parent_;
  if (m_DroppingBelowExpandedItem) {
    parent = index(row - 1, column, parent_);
    row    = 0;
  }

  const bool draggedOntoGroup = parent.isValid() && row == -1;
  const bool draggedToBottom  = parent.isValid() && row == rowCount(parent);
  const auto idx      = draggedOntoGroup || draggedToBottom ? index(parent.row() + 1, 0)
                                                            : index(row, column, parent);
  const int sourceRow = idx.isValid() ? mapLowerBoundToSourceRow(idx.internalId()) : -1;

  QString groupName;
  const auto baseModel = findBaseModel<PluginListModel>(sourceModel());
  PluginListDropInfo dropInfo{data, row, parent,
                              baseModel ? baseModel->m_Plugins : nullptr};
  if (parent.isValid()) {
    QModelIndex groupIdx = parent;
    while (groupIdx.parent().isValid()) {
      groupIdx = groupIdx.parent();
    }

    const auto& parentItem = m_ProxyItems.at(groupIdx.internalId());
    const auto& groupInfo  = parentItem.groupInfo;
    groupName              = groupInfo ? groupInfo->name : QString();
  }

  if (baseModel) {
    auto regroupRows = dropInfo.sourceRows();
    if (draggedToBottom || groupName.isEmpty()) {
      for (const std::size_t id : m_DraggingGroups) {
        const auto& groupItem = m_ProxyItems.at(id);
        const auto group      = groupItem.groupInfo;

        for (const std::size_t childId : group->children) {
          const auto& childItem = m_ProxyItems.at(childId);
          const auto childIndex = createIndex(childItem.row, 0, childId);

          std::erase(regroupRows, childIndex.data(PluginListModel::IndexRole).toInt());
        }
      }
    }

    baseModel->m_Plugins->setGroup(regroupRows, groupName);
  }
  m_DraggingGroups.clear();

  if (!sourceModel()->dropMimeData(data, action, sourceRow, 0, QModelIndex())) {
    return false;
  }

  emit layoutAboutToBeChanged();
  buildGroups();
  emit layoutChanged();

  return true;
}

void PluginGroupProxyModel::onSourceDataChanged(const QModelIndex& topLeft,
                                                const QModelIndex& bottomRight,
                                                const QList<int>& roles)
{
  const auto topLeftProxy     = mapFromSource(topLeft);
  const auto bottomRightProxy = mapFromSource(bottomRight);

  emit dataChanged(topLeftProxy, bottomRightProxy, roles);

  if (!roles.isEmpty() && !roles.contains(PluginListModel::GroupingRole)) {
    return;
  }

  emit layoutAboutToBeChanged();
  buildGroups();
  emit layoutChanged();
}

void PluginGroupProxyModel::onSourceLayoutChanged(
    [[maybe_unused]] const QList<QPersistentModelIndex>& parents,
    QAbstractItemModel::LayoutChangeHint hint)
{
  emit layoutAboutToBeChanged({}, hint);
  buildGroups();
  emit layoutChanged({}, hint);
}

void PluginGroupProxyModel::onSourceModelReset()
{
  emit layoutAboutToBeChanged();
  buildGroups();
  emit layoutChanged();
}

void PluginGroupProxyModel::onSourceRowsInserted(const QModelIndex& parent)
{
  if (parent.isValid()) {
    return;
  }

  emit layoutAboutToBeChanged();
  buildGroups();
  emit layoutChanged();
}

void PluginGroupProxyModel::onSourceRowsRemoved(const QModelIndex& parent)
{
  if (parent.isValid()) {
    return;
  }

  emit layoutAboutToBeChanged();
  buildGroups();
  emit layoutChanged();
}

void PluginGroupProxyModel::buildGroups()
{
  m_TopLevel.clear();
  m_SourceMap.clear();

  const auto sortProxy = qobject_cast<const QSortFilterProxyModel*>(sourceModel());
  const bool sorted =
      sortProxy && sortProxy->sortColumn() == PluginListModel::COL_PRIORITY;

  int primaryDivider = -1;
  int masterDivider  = -1;
  for (int i = 0; i < sourceModel()->rowCount(); ++i) {
    const auto idx = sourceModel()->index(i, 0);
    const auto plugin =
        idx.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();

    if (sorted && plugin) {
      if (sortProxy->sortOrder() == Qt::AscendingOrder) {
        if (plugin->forceLoaded()) {
          primaryDivider = i;
        } else if (plugin->isMasterFile()) {
          masterDivider = i;
        }
      } else {
        if (masterDivider == -1 && plugin->isMasterFile()) {
          masterDivider = i - 1;
        } else if (primaryDivider == -1 && plugin->forceLoaded()) {
          primaryDivider = i - 1;
          break;
        }
      }
    }
  }

  boost::container::flat_map<QString, int> groupRepeats;

  QString lastGroup;
  std::size_t groupId = NO_ID;
  for (int i = 0, count = sourceModel()->rowCount(); i < count; ++i) {
    const auto idx      = sourceModel()->index(i, 0);
    const QString name  = idx.data().toString();
    const QString group = idx.data(PluginListModel::GroupingRole).toString();

    if (sorted && group != lastGroup) {
      lastGroup = group;

      if (group.isNull()) {
        groupId = NO_ID;
      } else {
        const int row = static_cast<int>(m_TopLevel.size());
        groupId = createItem(group, row, -1, NO_ID, std::make_shared<Group>(group),
                             groupRepeats[group]++);
        m_TopLevel.push_back(groupId);
      }
    }

    {
      auto& siblings =
          groupId == NO_ID ? m_TopLevel : m_ProxyItems[groupId].groupInfo->children;

      const int row = static_cast<int>(siblings.size());
      const auto id = createItem(name, row, i, groupId, nullptr);
      siblings.push_back(id);
      m_SourceMap.push_back(id);
    }

    if (sorted && (i == primaryDivider || i == masterDivider) &&
        (i != 0 && i != count - 1)) {
      lastGroup = QString();
      groupId   = NO_ID;

      const QString key = QString();
      const int row     = static_cast<int>(m_TopLevel.size());
      const auto id     = createItem(key, row, -1, NO_ID, nullptr, groupRepeats[key]++);
      m_TopLevel.push_back(id);
    }
  }

  for (std::size_t id = 0; id < m_ProxyItems.size(); ++id) {
    auto& item = m_ProxyItems[id];
    if (item.row < 0)
      continue;

    bool invalidate = false;
    if (item.parentId == NO_ID) {
      if (item.row >= m_TopLevel.size() || m_TopLevel[item.row] != id) {
        invalidate = true;
      }
    } else {
      const auto& parentItem = m_ProxyItems.at(item.parentId);
      if (const auto group = parentItem.groupInfo) {
        if (item.row >= group->children.size() || group->children[item.row] != id) {
          invalidate = true;
        }
      }
    }

    if (invalidate) {
      // we want to try to keep items alive even if they are temporarily removed from
      // the model, so assign them to an index that looks valid but won't be displayed
      const int fakeRow = static_cast<int>(m_TopLevel.size());
      for (int column = 0, count = columnCount(); column < count; ++column) {
        changePersistentIndex(createIndex(item.row, column, id),
                              createIndex(fakeRow, column, id));
      }
      item = ProxyItem{fakeRow, -1, NO_ID, nullptr};
    }
  }
}

std::size_t PluginGroupProxyModel::createItem(const QString& name, int row,
                                              int sourceRow, std::size_t parent,
                                              std::shared_ptr<Group> group, int repeat)
{
  if (m_ItemMap.count(name) <= repeat) {
    const auto id = m_ProxyItems.size();
    m_ItemMap.emplace(name, id);
    m_ProxyItems.emplace_back(row, sourceRow, parent, std::move(group));
    return id;
  } else {
    const auto it = m_ItemMap.find(name) + repeat;
    auto& item    = m_ProxyItems.at(it->second);
    if (row != item.row) {
      for (int column = 0, count = columnCount(); column < count; ++column) {
        changePersistentIndex(createIndex(item.row, column, it->second),
                              createIndex(row, column, it->second));
      }
    }
    item = ProxyItem{row, sourceRow, parent, std::move(group)};
    return it->second;
  }
}

}  // namespace BSPluginList
