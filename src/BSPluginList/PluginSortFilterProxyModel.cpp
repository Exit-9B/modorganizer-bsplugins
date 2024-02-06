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

bool PluginSortFilterProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex& source_parent) const
{
  const auto source_index = sourceModel()->index(source_row, 0, source_parent);
  const auto plugin =
      source_index.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
  if (m_HideForceEnabledFiles && plugin &&
      (plugin->forceLoaded() || plugin->forceEnabled())) {
    return false;
  }

  return filterMatchesPlugin(sourceModel()->data(source_index).toString());
}

bool PluginSortFilterProxyModel::lessThan(const QModelIndex& source_left,
                                          const QModelIndex& source_right) const
{
  switch (source_left.column()) {
  case PluginListModel::COL_CONFLICTS: {
    const auto left =
        source_left.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
    const auto right =
        source_right.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();

    const auto leftState  = left ? left->conflictState() : 0;
    const auto rightState = right ? right->conflictState() : 0;
    return leftState < rightState;
  }
  case PluginListModel::COL_FLAGS: {
    QVariantList lhsList = source_left.data(PluginListModel::FlagsIconRole).toList();
    QVariantList rhsList = source_right.data(PluginListModel::FlagsIconRole).toList();
    if (lhsList.length() != rhsList.length()) {
      return lhsList.length() < rhsList.length();
    } else {
      for (int i = 0; i < lhsList.length(); ++i) {
        if (lhsList.at(i) != rhsList.at(i)) {
          return lhsList.at(i).toString() < rhsList.at(i).toString();
        }
      }
      return false;
    }
  }
  }

  return QSortFilterProxyModel::lessThan(source_left, source_right);
}

void PluginSortFilterProxyModel::updateFilter(const QString& filter)
{
  m_CurrentFilter = filter;
  invalidateRowsFilter();
}

}  // namespace BSPluginList
