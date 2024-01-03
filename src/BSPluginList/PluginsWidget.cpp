#include "PluginsWidget.h"

#include "GUI/MessageDialog.h"
#include "GUI/SelectionDialog.h"
#include "PluginListContextMenu.h"
#include "PluginSortFilterProxyModel.h"
#include "ui_pluginswidget.h"

#include <boost/range/adaptor/reversed.hpp>

#include <QMenu>
#include <QMessageBox>

using namespace Qt::Literals::StringLiterals;

namespace BSPluginList
{

PluginsWidget::PluginsWidget(MOBase::IOrganizer* organizer,
                             IPanelInterface* panelInterface, QWidget* parent)
    : QWidget(parent), ui{new Ui_PluginsWidget()}, m_PanelInterface{panelInterface},
      m_Organizer{organizer}
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

  connect(pluginList, &TESData::PluginList::pluginsListChanged, this,
          &PluginsWidget::updatePluginCount);

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
  int activeMasterCount      = 0;
  int activeLightMasterCount = 0;
  int activeOverlayCount     = 0;
  int activeRegularCount     = 0;
  int masterCount            = 0;
  int lightMasterCount       = 0;
  int overlayCount           = 0;
  int regularCount           = 0;
  int activeVisibleCount     = 0;

  const auto managedGame = m_Organizer->managedGame();
  const auto tesSupport  = managedGame ? managedGame->feature<GamePlugins>() : nullptr;

  const bool lightPluginsAreSupported =
      tesSupport && tesSupport->lightPluginsAreSupported();
  const bool overridePluginsAreSupported =
      tesSupport && tesSupport->overridePluginsAreSupported();

  for (int i = 0, count = pluginListModel->rowCount(); i < count; ++i) {
    const auto index = pluginListModel->index(i, 0);
    const auto info =
        index.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
    if (!info)
      continue;

    const bool active  = info->enabled();
    const bool visible = proxyModel->filterAcceptsRow(index.row(), index.parent());
    if (info->isSmallFile()) {
      ++lightMasterCount;
      activeLightMasterCount += active;
      activeVisibleCount += visible && active;
    } else if (info->isMasterFile()) {
      ++masterCount;
      activeMasterCount += active;
      activeVisibleCount += visible && active;
    } else if (info->isOverlayFlagged()) {
      ++overlayCount;
      activeOverlayCount += active;
      activeVisibleCount += visible && active;
    } else {
      ++regularCount;
      activeRegularCount += active;
      activeVisibleCount += visible && active;
    }

    const int activeCount = activeMasterCount + activeLightMasterCount +
                            activeOverlayCount + activeRegularCount;
    const int totalCount = masterCount + lightMasterCount + overlayCount + regularCount;

    ui->activePluginsCounter->display(activeVisibleCount);

    QString toolTip;
    toolTip.reserve(575);
    toolTip += uR"(<table cellspacing="6">)"_s
               uR"(<tr><th>%1</th><th>%2</th><th>%3</th></tr>)"_s.arg(tr("Type"))
                   .arg(tr("Active"), -12)
                   .arg(tr("Total"));

    const QString row = uR"(<tr><td>%1:</td><td align=right>%2    </td>)"_s
                        uR"(<td align=right>%3</td></tr>)"_s;

    toolTip += row.arg(tr("All plugins")).arg(activeCount).arg(totalCount);
    toolTip += row.arg(tr("ESMs")).arg(activeMasterCount).arg(masterCount);
    toolTip += row.arg(tr("ESPs")).arg(activeRegularCount).arg(regularCount);
    toolTip += row.arg(tr("ESMs+ESPs"))
                   .arg(activeMasterCount + activeRegularCount)
                   .arg(masterCount + regularCount);
    if (lightPluginsAreSupported)
      toolTip += row.arg(tr("ESLs")).arg(activeLightMasterCount).arg(lightMasterCount);
    if (overridePluginsAreSupported)
      toolTip += row.arg(tr("Overlay")).arg(activeOverlayCount).arg(overlayCount);
    toolTip += uR"(</table>)"_s;

    ui->activePluginsCounter->setToolTip(toolTip);
  }
}

void PluginsWidget::on_espFilterEdit_textChanged(const QString& filter)
{
  proxyModel->updateFilter(filter);

  if (!filter.isEmpty()) {
    setStyleSheet("QTreeView { border: 2px ridge #f00; }");
    ui->activePluginsCounter->setStyleSheet("QLCDNumber { border: 2px ridge #f00; }");
  } else {
    setStyleSheet("");
    ui->activePluginsCounter->setStyleSheet("");
  }
  updatePluginCount();
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
  for (const auto& idx : ui->pluginList->selectionModel()->selectedRows()) {
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
  updatePluginCount();
}

constexpr auto PATTERN_BACKUP_GLOB  = R"/(.????_??_??_??_??_??)/";
constexpr auto PATTERN_BACKUP_REGEX = R"/(\.(\d\d\d\d_\d\d_\d\d_\d\d_\d\d_\d\d))/";
constexpr auto PATTERN_BACKUP_DATE  = R"/(yyyy_MM_dd_hh_mm_ss)/";

static QString queryRestore(const QString& filePath, QWidget* parent = nullptr)
{
  QFileInfo pluginFileInfo(filePath);
  QString pattern     = pluginFileInfo.fileName() + ".*";
  QFileInfoList files = pluginFileInfo.absoluteDir().entryInfoList(
      QStringList(pattern), QDir::Files, QDir::Name);

  GUI::SelectionDialog dialog(QObject::tr("Choose backup to restore"), parent);
  QRegularExpression exp(QRegularExpression::anchoredPattern(pluginFileInfo.fileName() +
                                                             PATTERN_BACKUP_REGEX));
  QRegularExpression exp2(
      QRegularExpression::anchoredPattern(pluginFileInfo.fileName() + "\\.(.*)"));
  for (const QFileInfo& info : boost::adaptors::reverse(files)) {
    auto match  = exp.match(info.fileName());
    auto match2 = exp2.match(info.fileName());
    if (match.hasMatch()) {
      QDateTime time = QDateTime::fromString(match.captured(1), PATTERN_BACKUP_DATE);
      dialog.addChoice(time.toString(), "", match.captured(1));
    } else if (match2.hasMatch()) {
      dialog.addChoice(match2.captured(1), "", match2.captured(1));
    }
  }

  if (dialog.numChoices() == 0) {
    QMessageBox::information(parent, QObject::tr("No Backups"),
                             QObject::tr("There are no backups to restore"));
    return QString();
  }

  if (dialog.exec() == QDialog::Accepted) {
    return dialog.getChoiceData().toString();
  } else {
    return QString();
  }
}

void PluginsWidget::on_restoreButton_clicked()
{
  const auto app         = this->topLevelWidget();
  const auto profilePath = QDir(m_Organizer->profilePath());
  const auto pluginsName = QDir::cleanPath(profilePath.absoluteFilePath("plugins.txt"));

  QString choice = queryRestore(pluginsName, app);
  if (!choice.isEmpty()) {
    const auto loadOrderName =
        QDir::cleanPath(profilePath.absoluteFilePath("loadorder.txt"));

    if (!MOBase::shellCopy(pluginsName + "." + choice, pluginsName, true, app) ||
        !MOBase::shellCopy(loadOrderName + "." + choice, loadOrderName, true, app)) {

      const auto e = ::GetLastError();

      QMessageBox::critical(
          this, tr("Restore failed"),
          tr("Failed to restore the backup. Errorcode: %1")
              .arg(QString::fromStdWString(MOBase::formatSystemMessage(e))));
    }
    pluginListModel->invalidate();
  }
}

static bool createBackup(const QString& filePath, const QDateTime& time,
                         QWidget* parent = nullptr)
{
  QString outPath = filePath + "." + time.toString(PATTERN_BACKUP_DATE);
  if (MOBase::shellCopy(QStringList(filePath), QStringList(outPath), parent)) {
    QFileInfo fileInfo(filePath);
    MOBase::removeOldFiles(fileInfo.absolutePath(),
                           fileInfo.fileName() + PATTERN_BACKUP_GLOB, 10, QDir::Name);
    return true;
  } else {
    return false;
  }
}

void PluginsWidget::on_saveButton_clicked()
{
  const auto app         = this->topLevelWidget();
  const auto profilePath = QDir(m_Organizer->profilePath());
  const auto pluginsName = QDir::cleanPath(profilePath.absoluteFilePath("plugins.txt"));
  const auto loadOrderName =
      QDir::cleanPath(profilePath.absoluteFilePath("loadorder.txt"));
  const auto lockedOrderName =
      QDir::cleanPath(profilePath.absoluteFilePath("lockedorder.txt"));

  const QDateTime now = QDateTime::currentDateTime();

  if (createBackup(pluginsName, now, app) && createBackup(loadOrderName, now, app) &&
      createBackup(lockedOrderName, now, app)) {
    GUI::MessageDialog::showMessage(tr("Backup of load order created"), app);
  }
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
  static bool refreshing = false;
  static std::function<void()> setRefreshing;
  setRefreshing = [=, this] {
    refreshing = true;
    pluginListModel->invalidate();
    organizer->onNextRefresh(setRefreshing, false);
  };

  organizer->onNextRefresh(setRefreshing, false);

  MOBase::IPluginList* const ipluginlist = organizer->pluginList();
  if (ipluginlist == nullptr || ipluginlist == pluginList) {
    return;
  }

  ipluginlist->onRefreshed([]() {
    refreshing = false;
  });

  ipluginlist->onPluginMoved(
      [this](const QString& name, int oldPriority, int newPriority) {
        if (refreshing)
          return;
        pluginListModel->movePlugin(name, oldPriority, newPriority);
      });

  ipluginlist->onPluginStateChanged(
      [this](const std::map<QString, MOBase::IPluginList::PluginStates>& infos) {
        if (refreshing)
          return;
        pluginListModel->changePluginStates(infos);
      });

  pluginList->onPluginMoved(
      [=, pluginList = pluginList](const QString& name,
                                   [[maybe_unused]] int oldPriority, int newPriority) {
        if (refreshing)
          return;

        ipluginlist->setPriority(name, newPriority);
      });

  pluginList->onPluginStateChanged(
      [=, pluginList = pluginList](
          const std::map<QString, MOBase::IPluginList::PluginStates>& infos) {
        if (refreshing || infos.empty())
          return;

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
