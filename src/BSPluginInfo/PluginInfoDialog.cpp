#include "PluginInfoDialog.h"
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

  if (const auto plugin = pluginList->getPluginByName(pluginName)) {
    for (const auto& archive : plugin->archives()) {
      const auto archiveEntry = m_PluginList->findArchive(archive);
      if (archiveEntry) {
        m_Archives.append(archive);

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
}

PluginInfoDialog::~PluginInfoDialog() noexcept
{
  delete ui;
}

int PluginInfoDialog::exec()
{
  const auto settings = Settings::instance();
  GUI::GeometrySaver gs{*settings, this};
  settings->restoreState(ui->pluginRecordView->splitter());
  settings->restoreState(ui->pluginRecordView->pickRecordView()->header());

  const int r = QDialog::exec();
  settings->saveState(ui->pluginRecordView->splitter());
  settings->saveState(ui->pluginRecordView->pickRecordView()->header());

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
