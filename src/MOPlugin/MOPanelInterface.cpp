#include "MOPanelInterface.h"

#include <algorithm>
#include <iterator>

using namespace Qt::Literals::StringLiterals;

MOPanelInterface::MOPanelInterface(MOBase::IOrganizer* organizer,
                                   QMainWindow* mainWindow)
    : m_ModList{organizer->modList()}, m_PluginList{organizer->pluginList()},
      m_ModListView{mainWindow->findChild<QTreeView*>(u"modList"_s)},
      m_PluginListView{mainWindow->findChild<QTreeView*>(u"espList"_s)}
{
  if (m_ModListView) {
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

void MOPanelInterface::onModSelectionChanged()
{
  QList<QString> origins;
  const auto indexes = m_ModListView->selectionModel()->selectedRows();
  std::ranges::transform(indexes, std::back_inserter(origins), [](auto&& idx) {
    return idx.data(Qt::DisplayRole).toString();
  });
  m_SelectedOriginsChanged(origins);
}
