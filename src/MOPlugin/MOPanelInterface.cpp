#include "MOPanelInterface.h"

#include <log.h>

#include <algorithm>
#include <functional>
#include <iterator>

using namespace Qt::Literals::StringLiterals;

MOPanelInterface::MOPanelInterface(MOBase::IOrganizer* organizer,
                                   QMainWindow* mainWindow)
    : m_ModList{organizer->modList()}, m_PluginList{organizer->pluginList()},
      m_ModListView{mainWindow->findChild<QTreeView*>(u"modList"_s)},
      m_PluginListView{mainWindow->findChild<QTreeView*>(u"espList"_s)}
{
  if (m_ModListView) {
    QObject::connect(m_ModListView, &QTreeView::collapsed, this,
                     &MOPanelInterface::onModSeparatorCollapsed);
    QObject::connect(m_ModListView, &QTreeView::expanded, this,
                     &MOPanelInterface::onModSeparatorExpanded);
    QObject::connect(m_ModListView->selectionModel(),
                     &QItemSelectionModel::selectionChanged, this,
                     &MOPanelInterface::onModSelectionChanged);
  }
}

MOPanelInterface::~MOPanelInterface() noexcept
{
  m_SelectedOriginsChanged.disconnect_all_slots();
}

void MOPanelInterface::assignWidget(QTabWidget* tabWidget, QWidget* panel)
{
  QObject::connect(tabWidget, &QTabWidget::currentChanged,
                   [this, tabWidget, panel](int index) {
                     QWidget* const currentWidget = tabWidget->widget(index);
                     if (currentWidget == panel) {
                       m_PanelActivated();
                     }
                   });
}

void MOPanelInterface::setSelectedFiles(const QList<QString>& selectedFiles)
{
  if (!m_PluginListView) {
    return;
  }

  QItemSelection selection;
  const auto model = m_PluginListView->model();
  for (int row = 0, count = model->rowCount(); row < count; ++row) {
    const auto index = model->index(row, 0);
    if (selectedFiles.contains(index.data(Qt::DisplayRole))) {
      const auto end = model->index(row, model->columnCount() - 1);
      selection.select(index, end);
    }
  }

  m_PluginListView->selectionModel()->select(selection,
                                             QItemSelectionModel::ClearAndSelect);
}

// FIXME: only works for files in the plugins panel
void MOPanelInterface::displayOriginInformation(const QString& file)
{
  const auto model = m_PluginListView->model();
  for (int row = 0, count = model->rowCount(); row < count; ++row) {
    const auto index = model->index(row, 0);
    const auto other = index.data(Qt::DisplayRole).toString();
    if (file.compare(other, Qt::CaseInsensitive) == 0) {
      m_PluginListView->selectionModel()->select(
          QItemSelection(index, model->index(row, model->columnCount() - 1)),
          QItemSelectionModel::ClearAndSelect);
      m_PluginListView->selectionModel()->setCurrentIndex(index,
                                                          QItemSelectionModel::Current);
      m_PluginListView->doubleClicked(index);
      return;
    }
  }

  MOBase::log::warn("failed to open origin info for \"{}\"", file);
}

bool MOPanelInterface::onPanelActivated(const std::function<void()>& func)
{
  auto connection = m_PanelActivated.connect(func);
  return connection.connected();
}

bool MOPanelInterface::onSelectedOriginsChanged(
    const std::function<void(const QList<QString>&)>& func)
{
  auto connection = m_SelectedOriginsChanged.connect(func);
  return connection.connected();
}

void MOPanelInterface::setPluginState(const QString& name, bool enable)
{
  const auto model = m_PluginListView->model();
  for (int i = 0, count = model->rowCount(); i < count; ++i) {
    const auto index = model->index(i, 0);
    if (index.data().toString().compare(name, Qt::CaseInsensitive) == 0) {
      model->setData(index, enable ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
    }
  }
}

void MOPanelInterface::onModSeparatorCollapsed(const QModelIndex& index)
{
  if (m_ModListView->selectionModel()->isSelected(index)) {
    onModSelectionChanged();
  }
}

void MOPanelInterface::onModSeparatorExpanded(const QModelIndex& index)
{
  if (m_ModListView->selectionModel()->isSelected(index)) {
    onModSelectionChanged();
  }
}

void MOPanelInterface::onModSelectionChanged()
{
  QList<QString> origins;
  std::function<void(const QModelIndex&)> addOrigins;
  addOrigins = [&](const QModelIndex& index) {
    if (index.model()->hasChildren(index)) {
      if (m_ModListView->isExpanded(index)) {
        return;
      }

      for (int i = 0, count = index.model()->rowCount(index); i < count; ++i) {
        addOrigins(index.model()->index(i, 0, index));
      }
    } else {
      origins.append(index.data(Qt::DisplayRole).toString());
    }
  };

  const auto indexes = m_ModListView->selectionModel()->selectedRows();
  std::ranges::for_each(indexes, addOrigins);
  m_SelectedOriginsChanged(origins);
}
