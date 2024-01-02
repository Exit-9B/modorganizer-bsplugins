#include "PluginsWidget.h"

#include "PluginListContextMenu.h"
#include "PluginSortFilterProxyModel.h"
#include "ui_pluginswidget.h"

#include <QMenu>

namespace BSPluginList
{

PluginsWidget::PluginsWidget(MOBase::IOrganizer* organizer,
                             IPanelInterface* panelInterface, QWidget* parent)
    : QWidget(parent), ui{new Ui_PluginsWidget()}, m_PanelInterface{panelInterface}
{
  ui->setupUi(this);

  pluginList      = new TESData::PluginList(organizer);
  pluginListModel = new PluginListModel(pluginList);
  proxyModel      = new PluginSortFilterProxyModel();
  proxyModel->setSourceModel(pluginListModel);
  proxyModel->setDynamicSortFilter(true);
  ui->pluginList->setModel(proxyModel);
  ui->pluginList->setup();
  ui->pluginList->sortByColumn(PluginListModel::COL_PRIORITY, Qt::AscendingOrder);
  optionsMenu = listOptionsMenu();
  ui->listOptionsBtn->setMenu(optionsMenu);

  connect(pluginListModel, &PluginListModel::pluginStatesChanged, ui->pluginList,
          &PluginListView::updateOverwriteMarkers);
  connect(pluginListModel, &PluginListModel::pluginOrderChanged, ui->pluginList,
          &PluginListView::updateOverwriteMarkers);
  connect(pluginListModel, &QAbstractItemModel::modelReset, ui->pluginList,
          &PluginListView::clearOverwriteMarkers);

  connect(pluginListModel, &PluginListModel::pluginStatesChanged, this,
          &PluginsWidget::updatePluginCount);

  connect(ui->pluginList, &QTreeView::customContextMenuRequested,
          [=, this](const QPoint& pos) {
            PluginListContextMenu menu{ui->pluginList->indexAt(pos), pluginListModel,
                                       ui->pluginList, organizer->modList(),
                                       pluginList};
            const QPoint p = ui->pluginList->viewport()->mapToGlobal(pos);
            menu.exec(p);
          });

  connect(ui->pluginList->selectionModel(), &QItemSelectionModel::selectionChanged,
          this, &PluginsWidget::onSelectionChanged);

  panelInterface->onPanelActivated([this]() {
    this->onPanelActivated();
  });

  panelInterface->onSelectedOriginsChanged([this](const QList<QString>& origins) {
    this->onSelectedOriginsChanged(origins);
  });

  synchronizePluginLists(organizer);
  updatePluginCount();
}

PluginsWidget::~PluginsWidget() noexcept
{
  delete ui;
  delete pluginList;
  delete pluginListModel;
  delete proxyModel;
  delete optionsMenu;
}

void PluginsWidget::updatePluginCount()
{
  int activeVisibleCount = 0;

  for (int i = 0, count = pluginList->pluginCount(); i < count; ++i) {
    const auto info = pluginList->getPlugin(i);
    if (info && info->enabled()) {
      ++activeVisibleCount;
    }
  }

  ui->activePluginsCounter->display(activeVisibleCount);
}

void PluginsWidget::changeEvent(QEvent* e)
{
  QWidget::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
    ui->retranslateUi(this);
    break;
  default:
    break;
  }
}

void PluginsWidget::onSelectionChanged(
    [[maybe_unused]] const QItemSelection& selected,
    [[maybe_unused]] const QItemSelection& deselected)
{
  QList<QString> selectedFiles;
  for (auto& idx : ui->pluginList->selectionModel()->selectedRows()) {
    const auto model = ui->pluginList->model();
    const auto& name = model->data(idx, Qt::DisplayRole).toString();
    selectedFiles.append(name);
  }
  m_PanelInterface->setSelectedFiles(selectedFiles);
}

void PluginsWidget::onPanelActivated()
{
  pluginListModel->refresh();
}

void PluginsWidget::onSelectedOriginsChanged(const QList<QString>& origins)
{
  ui->pluginList->setHighlightedOrigins(origins);
}

void PluginsWidget::toggleHideForceEnabled()
{
  const bool doHide = toggleForceEnabled->isChecked();
  proxyModel->hideForceEnabledFiles(doHide);
}

QMenu* PluginsWidget::listOptionsMenu()
{
  QMenu* const menu  = new QMenu(this);
  toggleForceEnabled = menu->addAction(tr("Hide force-enabled files"), this,
                                       &PluginsWidget::toggleHideForceEnabled);
  toggleForceEnabled->setCheckable(true);

  return menu;
}

void PluginsWidget::synchronizePluginLists(MOBase::IOrganizer* organizer)
{
  MOBase::IPluginList* const ipluginlist = organizer->pluginList();
  if (ipluginlist == nullptr || ipluginlist == pluginList) {
    return;
  }

  ipluginlist->onPluginMoved(
      [this](const QString& name, int oldPriority, int newPriority) {
        pluginListModel->movePlugin(name, oldPriority, newPriority);
      });

  ipluginlist->onPluginStateChanged(
      [this](const std::map<QString, MOBase::IPluginList::PluginStates>& infos) {
        pluginListModel->changePluginStates(infos);
      });

  ipluginlist->onRefreshed([=, this]() {
    pluginListModel->invalidate();
  });

  pluginList->onPluginMoved(
      [=, pluginList = pluginList](const QString& name,
                                   [[maybe_unused]] int oldPriority, int newPriority) {
        if (pluginList->isRefreshing()) {
          return;
        }

        ipluginlist->setPriority(name, newPriority);
      });

  pluginList->onPluginStateChanged(
      [=, pluginList = pluginList](
          const std::map<QString, MOBase::IPluginList::PluginStates>& infos) {
        if (pluginList->isRefreshing() || infos.empty()) {
          return;
        }

        for (const auto& [name, state] : infos) {
          ipluginlist->setState(name, state);
        }

        // kick the plugin list so it actually updates
        const MOBase::IPluginGame* const managedGame = organizer->managedGame();
        const QStringList primaryPlugins =
            managedGame ? managedGame->primaryPlugins() : QStringList();

        if (!primaryPlugins.isEmpty()) {
          ipluginlist->setPriority(primaryPlugins[0], 0);
        }
      });
}

}  // namespace BSPluginList
