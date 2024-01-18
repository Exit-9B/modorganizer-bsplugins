#include "PluginSortFilterProxyModel.h"
#include "PluginListModel.h"

#include <algorithm>

namespace BSPluginList
{

void PluginSortFilterProxyModel::hideForceEnabledFiles(bool doHide)
{
  m_HideForceEnabledFiles = doHide;
  invalidateRowsFilter();
}

bool PluginSortFilterProxyModel::filterMatchesPlugin(const QString& plugin) const
{
  if (m_CurrentFilter.isEmpty()) {
    return true;
  }

  QString filterCopy = QString(m_CurrentFilter);
  filterCopy.replace("||", ";").replace("OR", ";").replace("|", ";");

  const auto ORList = QStringTokenizer(filterCopy, u';', Qt::SkipEmptyParts);

  return std::ranges::any_of(ORList, [&plugin](auto&& ORSegment) {
    const auto ANDkeywords = QStringTokenizer(ORSegment, u' ', Qt::SkipEmptyParts);

    return std::ranges::all_of(ANDkeywords, [&plugin](auto&& currentKeyword) {
      return plugin.contains(currentKeyword, Qt::CaseInsensitive);
    });
  });
}

bool PluginSortFilterProxyModel::canDropMimeData(const QMimeData* data,
                                                 Qt::DropAction action, int row,
                                                 int column,
                                                 const QModelIndex& parent) const
{
  if (sortColumn() != PluginListModel::COL_PRIORITY) {
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

bool PluginSortFilterProxyModel::dropMimeData(const QMimeData* data,
                                              Qt::DropAction action, int row,
                                              int column, const QModelIndex& parent)
{
  if (sortColumn() != PluginListModel::COL_PRIORITY) {
    return false;
  }

  if (sortOrder() == Qt::DescendingOrder) {
    --row;
  }

  const QModelIndex proxyIndex  = index(row, column, parent);
  const QModelIndex sourceIndex = mapToSource(proxyIndex);
  return sourceModel()->dropMimeData(data, action, sourceIndex.row(),
                                     sourceIndex.column(), sourceIndex.parent());
}

void PluginSortFilterProxyModel::updateFilter(const QString& filter)
{
  m_CurrentFilter = filter;
  invalidateRowsFilter();
}

bool PluginSortFilterProxyModel::filterAcceptsRow(
    int source_row, [[maybe_unused]] const QModelIndex& source_parent) const
{
  const auto source_index = sourceModel()->index(source_row, 0);
  const auto plugin =
      source_index.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
  if (m_HideForceEnabledFiles && plugin &&
      (plugin->forceLoaded() || plugin->forceEnabled())) {
    return false;
  }

  return filterMatchesPlugin(sourceModel()->data(source_index).toString());
}

}  // namespace BSPluginList
