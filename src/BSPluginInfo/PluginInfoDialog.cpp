#include "PluginInfoDialog.h"
#include "AuxConflictModel.h"
#include "GUI/IGeometrySettings.h"
#include "MOPlugin/Settings.h"
#include "PluginRecordView.h"
#include "ui_plugininfodialog.h"

#include <QTreeView>
#include <QVBoxLayout>

namespace BSPluginInfo
{

PluginInfoDialog::PluginInfoDialog(MOBase::IOrganizer* organizer,
                                   TESData::PluginList* pluginList,
                                   const QString& pluginName, QWidget* parent)
    : QDialog(parent), ui{new Ui::PluginInfoDialog()}, m_PluginList{pluginList},
      m_PluginName{pluginName}
{
  ui->setupUi(this);
  setWindowTitle(pluginName);

  ui->pluginRecordView->setup(organizer, pluginList, pluginName);

  const auto winnerModel      = new AuxWinnerModel(pluginList, ui->winningTree);
  const auto loserModel       = new AuxLoserModel(pluginList, ui->losingTree);
  const auto nonConflictModel = new AuxNonConflictModel(pluginList, ui->noConflictTree);

  const auto entry  = pluginList->findEntryByName(pluginName.toStdString());
  const auto handle = entry ? entry->handle() : -1;

  if (const auto plugin = pluginList->getPluginByName(pluginName)) {
    for (const auto& archive : plugin->archives()) {
      const auto archiveEntry = m_PluginList->findArchive(archive);
      if (archiveEntry) {
        m_Archives.append(archive);

        int winningCount    = 0;
        int losingCount     = 0;
        int noConflictCount = 0;

        archiveEntry->forEachMember([&](auto&& member) {
          QList<TESData::TESFileHandle> handles;
          std::ranges::copy(member->alternatives, std::back_inserter(handles));
          std::ranges::sort(handles, std::less<int>(), [&](auto altHandle) {
            const auto altEntry = pluginList->findEntryByHandle(altHandle);
            const auto info     = altEntry ? pluginList->getPluginByName(
                                             QString::fromStdString(altEntry->name()))
                                           : nullptr;
            return info ? info->priority() : -1;
          });

          if (handles.length() <= 1) {
            ++noConflictCount;
            nonConflictModel->appendItem(QString::fromStdString(member->path),
                                         std::move(handles));
          } else if (handles.last() == handle) {
            ++winningCount;
            winnerModel->appendItem(QString::fromStdString(member->path),
                                    std::move(handles));
          } else {
            ++losingCount;
            loserModel->appendItem(QString::fromStdString(member->path),
                                   std::move(handles));
          }
        });

        ui->winningCount->display(winningCount);
        ui->losingCount->display(losingCount);
        ui->noConflictCount->display(noConflictCount);

        const auto dataTree = new QTreeView(ui->archivesTreeStack);

        const auto model = new AuxTreeModel(archiveEntry, dataTree);
        const auto proxy = new QSortFilterProxyModel(dataTree);
        proxy->setRecursiveFilteringEnabled(true);
        proxy->setSourceModel(model);

        dataTree->setModel(proxy);

        ui->archivesTreeStack->addWidget(dataTree);
      }
    }
  }

  ui->winningTree->setModel(winnerModel);
  ui->losingTree->setModel(loserModel);
  ui->noConflictTree->setModel(nonConflictModel);

  m_WinningExpander.set(ui->winningExpander, ui->winningTree, true);
  m_LosingExpander.set(ui->losingExpander, ui->losingTree, true);
  m_NoConflictExpander.set(ui->noConflictExpander, ui->noConflictTree, true);

  m_FilterWinning.setEdit(ui->winningLineEdit);
  m_FilterWinning.setList(ui->winningTree);
  m_FilterWinning.setUseSourceSort(true);

  m_FilterLosing.setEdit(ui->losingLineEdit);
  m_FilterLosing.setList(ui->losingTree);
  m_FilterLosing.setUseSourceSort(true);

  m_FilterNoConflicts.setEdit(ui->noConflictLineEdit);
  m_FilterNoConflicts.setList(ui->noConflictTree);
  m_FilterNoConflicts.setUseSourceSort(true);
}

PluginInfoDialog::~PluginInfoDialog() noexcept
{
  delete ui;
}

int PluginInfoDialog::exec()
{
  const auto settings = Settings::instance();
  GUI::GeometrySaver gs{*settings, this};

  GUI::StateSaver ssRecordSplitter{*settings, ui->pluginRecordView->splitter()};
  GUI::StateSaver ssRecordPickHeader{*settings,
                                     ui->pluginRecordView->pickRecordView()->header()};

  GUI::StateSaver ssArchiveWinningExpander{*settings, &m_WinningExpander};
  GUI::StateSaver ssArchiveLosingExpander{*settings, &m_LosingExpander};
  GUI::StateSaver ssArchiveNoConflictExpander{*settings, &m_NoConflictExpander};
  GUI::StateSaver ssArchiveWinningHeader{*settings, ui->winningTree->header()};
  GUI::StateSaver ssArchiveLosingHeader{*settings, ui->losingTree->header()};
  GUI::StateSaver ssArchiveNoConflictHeader{*settings, ui->noConflictTree->header()};

  const int r = QDialog::exec();

  return r;
}

void PluginInfoDialog::on_close_clicked()
{
  close();
}

void PluginInfoDialog::on_nextFile_clicked()
{
  const auto current = m_PluginList->getPluginByName(m_PluginName);
  int nextPriority   = current ? current->priority() + 1 : 0;
  if (nextPriority >= m_PluginList->pluginCount()) {
    nextPriority = 0;
  }

  const auto next = m_PluginList->getPluginByPriority(nextPriority);
  if (next) {
    setCurrent(next->name());
  }
}

void PluginInfoDialog::on_previousFile_clicked()
{
  const auto current   = m_PluginList->getPluginByName(m_PluginName);
  int previousPriority = current ? current->priority() - 1 : 0;
  if (previousPriority < 0) {
    previousPriority = m_PluginList->pluginCount() - 1;
  }

  const auto previous = m_PluginList->getPluginByPriority(previousPriority);
  if (previous) {
    setCurrent(previous->name());
  }
}

void PluginInfoDialog::on_previousArchiveButton_clicked()
{
  ui->archiveFilterEdit->clear();

  const int currentIndex = ui->archivesTreeStack->currentIndex();
  const int count        = ui->archivesTreeStack->count();

  if (currentIndex == 0) {
    ui->archivesTreeStack->setCurrentIndex(count - 1);
  } else {
    ui->archivesTreeStack->setCurrentIndex(currentIndex - 1);
  }
}

void PluginInfoDialog::on_nextArchiveButton_clicked()
{
  ui->archiveFilterEdit->clear();

  const int currentIndex = ui->archivesTreeStack->currentIndex();
  const int count        = ui->archivesTreeStack->count();

  if (currentIndex == count - 1) {
    ui->archivesTreeStack->setCurrentIndex(0);
  } else {
    ui->archivesTreeStack->setCurrentIndex(currentIndex + 1);
  }
}

void PluginInfoDialog::on_archivesTreeStack_currentChanged(int index)
{
  ui->currentArchiveLabel->setText(m_Archives[index]);
}

void PluginInfoDialog::on_archiveFilterEdit_textChanged(const QString& text)
{
  const auto widget = ui->archivesTreeStack->currentWidget();
  const auto view   = qobject_cast<QAbstractItemView*>(widget);
  if (!view) {
    return;
  }

  const auto model       = view->model();
  const auto filterProxy = qobject_cast<QSortFilterProxyModel*>(model);
  if (!filterProxy) {
    return;
  }

  filterProxy->setFilterFixedString(text);
}

void PluginInfoDialog::setCurrent(const QString& pluginName)
{
  m_PluginName = pluginName;
  setWindowTitle(pluginName);
  ui->pluginRecordView->setFile(pluginName);
}

}  // namespace BSPluginInfo
