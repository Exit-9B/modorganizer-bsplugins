#include "PluginInfoDialog.h"
#include "PluginRecordView.h"
#include "ui_plugininfodialog.h"

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
  ui->pluginRecordView->setup(organizer, pluginList, pluginName);
  setWindowTitle(pluginName);
}

PluginInfoDialog::~PluginInfoDialog()
{
  delete ui;
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

void PluginInfoDialog::setCurrent(const QString& pluginName)
{
  m_PluginName = pluginName;
  setWindowTitle(pluginName);
  ui->pluginRecordView->setFile(pluginName);
}

}  // namespace BSPluginInfo
