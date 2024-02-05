#include "RecordFilterProxyModel.h"
#include "MOPlugin/Settings.h"

#include <algorithm>

namespace BSPluginInfo
{

RecordFilterProxyModel::RecordFilterProxyModel(const TESData::PluginList* pluginList,
                                               const QString& pluginName)
    : m_PluginList{pluginList}, m_PluginName{pluginName},
      m_FilterFlags{Filter_AllConflicts}
{
  setRecursiveFilteringEnabled(true);
}

void RecordFilterProxyModel::setFile(const QString& pluginName)
{
  m_PluginName = pluginName;
}

void RecordFilterProxyModel::setFilterFlags(FilterFlags filterFlags)
{
  m_FilterFlags = filterFlags;
  invalidateRowsFilter();
}

void RecordFilterProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
  emit beginResetModel();

  if (const auto oldSource = this->sourceModel()) {
    disconnect(oldSource, nullptr, this, nullptr);
  }

  QSortFilterProxyModel::setSourceModel(sourceModel);

  connect(sourceModel, &QAbstractItemModel::dataChanged, this,
          &RecordFilterProxyModel::onSourceDataChanged);
  connect(sourceModel, &QAbstractItemModel::rowsRemoved, this,
          &RecordFilterProxyModel::onSourceRowsRemoved);

  emit endResetModel();
}

void RecordFilterProxyModel::onSourceDataChanged()
{
  if (dynamicSortFilter()) {
    invalidateRowsFilter();
  }
}

void RecordFilterProxyModel::onSourceRowsRemoved(const QModelIndex& parent, int first,
                                                 int last)
{
  const auto proxyParent = mapFromSource(parent);
  const int proxyFirst   = mapFromSource(sourceModel()->index(first, 0, parent)).row();
  const int proxyLast    = mapFromSource(sourceModel()->index(last, 0, parent)).row();
  beginRemoveRows(proxyParent, std::max(proxyFirst, 0), std::max(proxyLast, 0));
  endRemoveRows();
}

bool RecordFilterProxyModel::filterAcceptsRow(int source_row,
                                              const QModelIndex& source_parent) const
{
  if (!m_PluginList) {
    return true;
  }

  using Item       = TESData::FileEntry::TreeItem;
  const auto index = sourceModel()->index(source_row, 0, source_parent);

  const auto item = index.data(Qt::UserRole).value<const Item*>();
  const auto info = m_PluginList->getPluginByName(m_PluginName);
  if (!item || !info) {
    return true;
  }

  if (!item->record) {
    return false;
  }

  if (m_FilterFlags == Filter_AllRecords) {
    return true;
  }

  const bool ignoreMasters =
      Settings::instance()->get<bool>("ignore_master_conflicts", false);

  bool isConflicted = false;
  bool isLosing     = false;
  for (const auto alternative : item->record->alternatives()) {
    const auto altEntry = m_PluginList->findEntryByHandle(alternative);
    if (!altEntry || altEntry->name() == m_PluginName.toStdString())
      continue;

    const auto altInfo =
        m_PluginList->getPluginByName(QString::fromStdString(altEntry->name()));
    if (!altInfo || !altInfo->enabled())
      continue;

    if (info->priority() > altInfo->priority()) {
      if (!ignoreMasters || !info->masters().contains(altInfo->name())) {
        isConflicted = true;
      }
    } else {
      if (!ignoreMasters || !altInfo->masters().contains(info->name())) {
        isConflicted = true;
        isLosing     = true;
        break;
      }
    }
  }

  if (isConflicted && !isLosing) {
    return m_FilterFlags & Filter_WinningConflicts;
  }

  if (!isConflicted) {
    return m_FilterFlags & Filter_NoConflicts;
  } else {
    if (!isLosing) {
      return m_FilterFlags & Filter_WinningConflicts;
    } else {
      if (m_FilterFlags == Filter_AllConflicts) {
        return true;
      }

      if (sourceModel()->canFetchMore(index)) {
        sourceModel()->fetchMore(index);
      }

      return m_FilterFlags & Filter_LosingConflicts;
    }
  }
}

}  // namespace BSPluginInfo
