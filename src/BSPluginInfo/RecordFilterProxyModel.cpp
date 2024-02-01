#include "RecordFilterProxyModel.h"

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

  if (m_FilterFlags == Filter_AllConflicts) {
    return std::ranges::count_if(item->record->alternatives(), [this](auto handle) {
             const auto entry = m_PluginList->findEntryByHandle(handle);
             const auto info  = entry ? m_PluginList->getPluginByName(
                                           QString::fromStdString(entry->name()))
                                      : nullptr;
             return info && info->enabled();
           }) > 1;
  } else if (m_FilterFlags == Filter_AllRecords) {
    return true;
  }

  if (sourceModel()->canFetchMore(index)) {
    sourceModel()->fetchMore(index);
  }

  bool isConflicted = false;
  for (const auto alternative : item->record->alternatives()) {
    const auto altEntry = m_PluginList->findEntryByHandle(alternative);
    if (!altEntry || altEntry->name() == m_PluginName.toStdString())
      continue;

    const auto altInfo =
        m_PluginList->getPluginByName(QString::fromStdString(altEntry->name()));
    if (!altInfo || !altInfo->enabled())
      continue;

    isConflicted = true;
    if (altInfo->priority() > info->priority()) {
      return m_FilterFlags & Filter_LosingConflicts;
    }
  }

  if (isConflicted) {
    return m_FilterFlags & Filter_WinningConflicts;
  } else {
    return m_FilterFlags & Filter_NoConflicts;
  }
}

}  // namespace BSPluginInfo
