#include "PluginSortFilterProxyModel.h"
#include "PluginListModel.h"

namespace BSPluginList
{

void PluginSortFilterProxyModel::hideForceEnabledFiles(bool doHide)
{
  m_HideForceEnabledFiles = doHide;
  invalidateRowsFilter();
}

bool PluginSortFilterProxyModel::canDropMimeData(const QMimeData* data,
                                                 Qt::DropAction action, int row,
                                                 int column,
                                                 const QModelIndex& parent) const
{
  if ((sortColumn() != PluginListModel::COL_PRIORITY)) {
    return false;
  }

  if (sortOrder() == Qt::DescendingOrder) {
    --row;
  }

  const QModelIndex proxyIndex  = index(row, column, parent);
  const QModelIndex sourceIndex = mapToSource(proxyIndex);
  return sourceModel()->canDropMimeData(data, action, sourceIndex.row(),
                                        sourceIndex.column(), sourceIndex.parent());
}

bool PluginSortFilterProxyModel::filterAcceptsRow(
    int source_row, [[maybe_unused]] const QModelIndex& source_parent) const
{
  const auto source_index = sourceModel()->index(source_row, 0);
  const auto plugin =
      source_index.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
  if (m_HideForceEnabledFiles && plugin && plugin->isPrimaryFile()) {
    return false;
  }

  return true;
}

}  // namespace BSPluginList
