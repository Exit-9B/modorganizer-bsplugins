#include "PluginListView.h"

#include "GUI/CopyEventFilter.h"
#include "GUI/GenericIconDelegate.h"
#include "MOPlugin/Settings.h"
#include "PluginGroupProxyModel.h"
#include "PluginListModel.h"
#include "PluginListStyledItemDelegate.h"
#include "PluginListViewMarkingScrollBar.h"
#include "PluginSortFilterProxyModel.h"
#include "TESData/PluginList.h"

#include <widgetutility.h>

#include <QHeaderView>
#include <QSortFilterProxyModel>

#include <stdexcept>

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

  header()->resizeSection(PluginListModel::COL_NAME, 332);
  header()->resizeSection(PluginListModel::COL_CONFLICTS, 71);
  header()->resizeSection(PluginListModel::COL_FLAGS, 60);
  header()->resizeSection(PluginListModel::COL_PRIORITY, 62);
  header()->resizeSection(PluginListModel::COL_MODINDEX, 79);
  header()->setSectionResizeMode(0, QHeaderView::Stretch);

  connect(this, &QTreeView::collapsed, this, &PluginListView::updateOverwriteMarkers);
  connect(this, &QTreeView::expanded, this, &PluginListView::updateOverwriteMarkers);
  connect(selectionModel(), &QItemSelectionModel::selectionChanged, this,
          &PluginListView::updateOverwriteMarkers);
}

void PluginListView::setModel(QAbstractItemModel* model)
{
  for (auto nextModel = model; nextModel;) {
    const auto proxyModel = qobject_cast<QAbstractProxyModel*>(nextModel);
    if (proxyModel) {
      if (const auto groupProxy = qobject_cast<PluginGroupProxyModel*>(proxyModel)) {
        connect(groupProxy, &PluginGroupProxyModel::groupRenameRequested, this,
                &PluginListView::onGroupRenameRequested);
      } else if (const auto sortProxy =
                     qobject_cast<PluginSortFilterProxyModel*>(proxyModel)) {
        m_SortProxy = sortProxy;
      }
      nextModel = proxyModel->sourceModel();
    } else {
      if (const auto pluginModel = qobject_cast<PluginListModel*>(nextModel)) {
        m_PluginModel = pluginModel;
      } else {
        throw std::logic_error("PluginListView's model should be a PluginListModel");
      }
      nextModel = nullptr;
    }
  }

  QTreeView::setModel(model);
}

QRect PluginListView::visualRect(const QModelIndex& index) const
{
  QRect rect = QTreeView::visualRect(index);
  if (index.column() == 0) {
    if (index.isValid() && !index.model()->hasChildren(index) ||
        !index.data().isValid()) {
      rect.adjust(-indentation(), 0, 0, 0);
    }
  }
  return rect;
}

QColor PluginListView::markerColor(const QModelIndex& index) const
{
  bool ok;
  const uint pluginIndex = index.data(PluginListModel::IndexRole).toUInt(&ok);
  if (ok) {
    const bool highlight      = m_Markers.highlight.contains(pluginIndex);
    const bool overriding     = m_Markers.overriding.contains(pluginIndex);
    const bool overridden     = m_Markers.overridden.contains(pluginIndex);
    const bool overwritingAux = m_Markers.overwritingAux.contains(pluginIndex);
    const bool overwrittenAux = m_Markers.overwrittenAux.contains(pluginIndex);

    // the color logic looks backwards but this is what the mod list does
    if (highlight) {
      return Settings::instance()->containedColor();
    } else if (overridden) {
      return Settings::instance()->overwritingLooseFilesColor();
    } else if (overriding) {
      return Settings::instance()->overwrittenLooseFilesColor();
    } else if (overwrittenAux) {
      return Settings::instance()->overwritingArchiveFilesColor();
    } else if (overwritingAux) {
      return Settings::instance()->overwrittenArchiveFilesColor();
    }
  }

  const auto rowIndex = index.siblingAtColumn(0);
  if (model()->hasChildren(rowIndex) && !isExpanded(rowIndex)) {
    std::vector<QColor> colors;
    for (int i = 0; i < model()->rowCount(rowIndex); ++i) {
      const auto childIndex = model()->index(i, index.column(), rowIndex);
      const auto childColor = markerColor(childIndex);
      if (childColor.isValid()) {
        colors.push_back(childColor);
      }
    }

    if (colors.empty()) {
      return QColor();
    }

    int r = 0, g = 0, b = 0, a = 0;
    for (const auto& color : colors) {
      r += color.red();
      g += color.green();
      b += color.blue();
      a += color.alpha();
    }

    return QColor(
        static_cast<int>(r / colors.size()), static_cast<int>(g / colors.size()),
        static_cast<int>(b / colors.size()), static_cast<int>(a / colors.size()));
  }

  return QColor();
}

static void visitRows(const QAbstractItemModel* model,
                      std::function<void(const QModelIndex&)> visit,
                      const QModelIndex& root = QModelIndex())
{
  if (root.isValid()) {
    visit(root);
  }

  for (int i = 0, count = model->rowCount(root); i < count; ++i) {
    const auto idx = model->index(i, 0, root);
    visitRows(model, visit, idx);
  }
}

void PluginListView::setHighlightedOrigins(const QStringList& origins)
{
  m_Markers.highlight.clear();
  visitRows(model(), [this, &origins](auto&& idx) {
    const auto origin = idx.data(PluginListModel::OriginRole).toString();
    if (origins.contains(origin)) {
      m_Markers.highlight.insert(idx.data(PluginListModel::IndexRole).toUInt());
    }
  });

  viewport()->update();
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
  QModelIndexList indexes = selectionModel()->selectedRows();
  for (const auto& idx : selectionModel()->selectedRows()) {
    if (model()->hasChildren(idx) && !isExpanded(idx)) {
      for (int i = 0, count = model()->rowCount(idx); i < count; ++i) {
        indexes.append(model()->index(i, idx.column(), idx));
      }
    }
  }

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

  viewport()->update();
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
    if (const auto proxy = m_SortProxy) {
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

void PluginListView::dragMoveEvent(QDragMoveEvent* event)
{
  // HACK: dropping below an expanded item sends the same event as dropping below its
  // children, so set an additional flag to signal this
  if (const auto m = qobject_cast<PluginGroupProxyModel*>(model())) {
    m->setDroppingBelowExpandedItem(dropIndicatorPosition() ==
                                        QAbstractItemView::BelowItem &&
                                    isExpanded(indexAt(event->position().toPoint())));
  }

  QTreeView::dragMoveEvent(event);
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
  if (m_PluginModel == nullptr) {
    return false;
  }

  const auto sourceRows =
      indexViewToModel(selectionModel()->selectedRows(), m_PluginModel, true);

  const auto sortOrder = m_SortProxy ? m_SortProxy->sortOrder() : Qt::DescendingOrder;
  const int offset     = key == Qt::Key_Up && sortOrder == Qt::AscendingOrder ? -1 : 1;

  m_PluginModel->shiftPluginsPriority(sourceRows, offset);

  return true;
}

bool PluginListView::toggleSelectionState()
{
  if (m_PluginModel == nullptr) {
    return false;
  }

  const auto sourceRows =
      indexViewToModel(selectionModel()->selectedRows(), m_PluginModel, false);

  m_PluginModel->toggleState(sourceRows);

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
                                                 const QAbstractItemModel* model,
                                                 bool includeChildren) const
{
  QModelIndexList result;

  for (const auto& idx : indices) {
    const auto modelIdx = indexViewToModel(idx, model);
    if (modelIdx.isValid()) {
      result.append(modelIdx);
    }

    if (includeChildren) {
      for (int row = 0, count = idx.model()->rowCount(idx); row < count; ++row) {
        const auto childIdx      = idx.model()->index(row, 0, idx);
        const auto modelChildIdx = indexViewToModel(childIdx, model);
        if (modelChildIdx.isValid()) {
          result.append(modelChildIdx);
        }
      }
    }
  }

  return result;
}

void PluginListView::onGroupRenameRequested(const QModelIndex& index,
                                            const QString& name)
{
  if (!index.model()->hasChildren(index)) {
    return;
  }

  QModelIndexList sourceRows;
  for (int row = 0, count = index.model()->rowCount(index); row < count; ++row) {
    const auto childIndex = index.model()->index(row, 0, index);
    sourceRows.append(indexViewToModel(childIndex, m_PluginModel));
  }

  const auto persistentIndex = QPersistentModelIndex(index.model()->index(0, 0, index));
  const bool expanded        = isExpanded(index);

  m_PluginModel->setGroup(sourceRows, name);

  const auto newIndex = persistentIndex.parent();
  const auto newRight = newIndex.siblingAtColumn(index.model()->columnCount() - 1);
  setExpanded(newIndex, expanded);
  selectionModel()->select(QItemSelection(newIndex, newRight),
                           QItemSelectionModel::ClearAndSelect);
  selectionModel()->setCurrentIndex(newIndex, QItemSelectionModel::Current);
}

}  // namespace BSPluginList
