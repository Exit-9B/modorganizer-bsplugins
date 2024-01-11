#include "PluginListView.h"

#include "GUI/CopyEventFilter.h"
#include "GUI/GenericIconDelegate.h"
#include "MOPlugin/Settings.h"
#include "PluginListModel.h"
#include "PluginListStyledItemDelegate.h"
#include "PluginListViewMarkingScrollBar.h"
#include "PluginSortFilterProxyModel.h"
#include "TESData/PluginList.h"

#include <widgetutility.h>

#include <QHeaderView>
#include <QSortFilterProxyModel>

namespace BSPluginList
{

PluginListView::PluginListView(QWidget* parent) : QTreeView(parent)
{
  setVerticalScrollBar(new PluginListViewMarkingScrollBar(this));
  MOBase::setCustomizableColumns(this);
  setItemDelegate(new PluginListStyledItemDelegate(this));
  installEventFilter(new GUI::CopyEventFilter(this));
}

void PluginListView::setup()
{
  // sortByColumn(PluginListModel::COL_PRIORITY, Qt::AscendingOrder);
  setItemDelegateForColumn(
      PluginListModel::COL_CONFLICTS,
      new GUI::GenericIconDelegate(this, PluginListModel::ConflictsIconRole));
  setItemDelegateForColumn(
      PluginListModel::COL_FLAGS,
      new GUI::GenericIconDelegate(this, PluginListModel::FlagsIconRole));

  header()->resizeSection(PluginListModel::COL_NAME, 292);
  header()->resizeSection(PluginListModel::COL_CONFLICTS, 71);
  header()->resizeSection(PluginListModel::COL_FLAGS, 60);
  header()->resizeSection(PluginListModel::COL_PRIORITY, 62);
  header()->resizeSection(PluginListModel::COL_MODINDEX, 79);
  header()->setSectionResizeMode(0, QHeaderView::Stretch);

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
          &PluginListView::updateOverwriteMarkers);
}

QColor PluginListView::markerColor(const QModelIndex& index) const
{
  const uint pluginIndex    = index.data(PluginListModel::IndexRole).toUInt();
  const bool highlight      = m_Markers.highlight.contains(pluginIndex);
  const bool overriding     = m_Markers.overriding.contains(pluginIndex);
  const bool overridden     = m_Markers.overridden.contains(pluginIndex);
  const bool overwritingAux = m_Markers.overwritingAux.contains(pluginIndex);
  const bool overwrittenAux = m_Markers.overwrittenAux.contains(pluginIndex);

  if (highlight) {
    return Settings::instance()->containedColor();
  } else if (overridden) {
    return Settings::instance()->overwrittenLooseFilesColor();
  } else if (overriding) {
    return Settings::instance()->overwritingLooseFilesColor();
  } else if (overwrittenAux) {
    return Settings::instance()->overwrittenArchiveFilesColor();
  } else if (overwritingAux) {
    return Settings::instance()->overwritingArchiveFilesColor();
  }

  return QColor();
}

void PluginListView::setHighlightedOrigins(const QStringList& origins)
{
  m_Markers.highlight.clear();
  for (int i = 0, rowCount = model()->rowCount(); i < rowCount; ++i) {
    const auto idx    = model()->index(i, 0);
    const auto origin = idx.data(PluginListModel::OriginRole).toString();
    if (origins.contains(origin)) {
      m_Markers.highlight.insert(idx.data(PluginListModel::IndexRole).toUInt());
    }
  }

  dataChanged(model()->index(0, 0),
              model()->index(model()->rowCount() - 1, model()->columnCount() - 1));
  verticalScrollBar()->repaint();
}

void PluginListView::clearOverwriteMarkers()
{
  m_Markers.overriding.clear();
  m_Markers.overridden.clear();
  m_Markers.overwritingAux.clear();
  m_Markers.overwrittenAux.clear();
}

void PluginListView::updateOverwriteMarkers()
{
  const QModelIndexList indexes = selectionModel()->selectedRows();

  const auto insert = [](auto& dest, const auto& from) {
    for (const QVariant& elem : from) {
      dest.insert(elem.toUInt());
    }
  };

  clearOverwriteMarkers();
  for (const auto& idx : indexes) {
    insert(m_Markers.overriding,
           model()->data(idx, PluginListModel::OverridingRole).toList());
    insert(m_Markers.overridden,
           model()->data(idx, PluginListModel::OverriddenRole).toList());
    insert(m_Markers.overwritingAux,
           model()->data(idx, PluginListModel::OverwritingAuxRole).toList());
    insert(m_Markers.overwrittenAux,
           model()->data(idx, PluginListModel::OverwrittenAuxRole).toList());
  }

  dataChanged(model()->index(0, 0),
              model()->index(model()->rowCount() - 1, model()->columnCount() - 1));
  verticalScrollBar()->repaint();
}

bool PluginListView::event(QEvent* event)
{
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

    if (keyEvent->modifiers() == Qt::ControlModifier &&
        (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)) {

      if (selectionModel()->hasSelection() &&
          selectionModel()->selectedRows().count() == 1) {

        QModelIndex idx = selectionModel()->currentIndex();
        emit openOriginExplorer(idx);
        return true;
      }
    }

    bool sorted = false;
    if (const auto proxy = qobject_cast<const PluginSortFilterProxyModel*>(model())) {
      sorted = proxy->sortColumn() == PluginListModel::COL_PRIORITY;
    }

    if (sorted && keyEvent->modifiers() == Qt::ControlModifier &&
        (keyEvent->key() == Qt::Key_Up || keyEvent->key() == Qt::Key_Down)) {
      return moveSelection(keyEvent->key());
    } else if (keyEvent->key() == Qt::Key_Space) {
      return toggleSelectionState();
    }
  }

  return QTreeView::event(event);
}

void PluginListView::paintEvent(QPaintEvent* event)
{
  if (m_FirstPaint) {
    header()->setSectionResizeMode(0, QHeaderView::Interactive);
    header()->setStretchLastSection(true);
    m_FirstPaint = false;
  }

  QTreeView::paintEvent(event);
}

bool PluginListView::moveSelection(int key)
{
  const auto sortProxy   = static_cast<PluginSortFilterProxyModel*>(model());
  const auto pluginModel = qobject_cast<PluginListModel*>(sortProxy->sourceModel());

  if (pluginModel == nullptr) {
    return false;
  }

  const auto cindex = indexViewToModel(currentIndex(), pluginModel);
  const auto sourceRows =
      indexViewToModel(selectionModel()->selectedRows(), pluginModel);

  const int offset =
      key == Qt::Key_Up && sortProxy->sortOrder() == Qt::AscendingOrder ? -1 : 1;

  pluginModel->shiftPluginsPriority(sourceRows, offset);

  // setSelected(cindex, sourceRows);

  return true;
}

bool PluginListView::toggleSelectionState()
{
  const auto sortProxy   = static_cast<PluginSortFilterProxyModel*>(model());
  const auto pluginModel = qobject_cast<PluginListModel*>(sortProxy->sourceModel());

  if (pluginModel == nullptr) {
    return false;
  }

  const auto sourceRows =
      indexViewToModel(selectionModel()->selectedRows(), pluginModel);

  pluginModel->toggleState(sourceRows);

  return true;
}

QModelIndex PluginListView::indexViewToModel(const QModelIndex& index,
                                             const QAbstractItemModel* model) const
{
  if (index.model() == model) {
    return index;
  } else if (const auto* const proxy =
                 qobject_cast<const QAbstractProxyModel*>(index.model())) {
    return indexViewToModel(proxy->mapToSource(index), model);
  } else {
    return QModelIndex();
  }
}

QModelIndexList PluginListView::indexViewToModel(const QModelIndexList& indices,
                                                 const QAbstractItemModel* model) const
{
  QModelIndexList result;
  for (const auto& idx : indices) {
    result.append(indexViewToModel(idx, model));
  }
  return result;
}

}  // namespace BSPluginList
