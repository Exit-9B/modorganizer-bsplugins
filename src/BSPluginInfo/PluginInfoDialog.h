#ifndef BSPLUGININFO_PLUGININFODIALOG_H
#define BSPLUGININFO_PLUGININFODIALOG_H

#include "AuxTreeModel.h"
#include "TESData/PluginList.h"

#include <expanderwidget.h>
#include <filterwidget.h>
#include <imoinfo.h>

#include <QDialog>
#include <QString>

namespace Ui
{
class PluginInfoDialog;
}

namespace BSPluginInfo
{

class PluginInfoDialog final : public QDialog
{
  Q_OBJECT

public:
  PluginInfoDialog(MOBase::IOrganizer* organizer, TESData::PluginList* pluginList,
                   const QString& pluginName, QWidget* parent = nullptr);

  PluginInfoDialog(const PluginInfoDialog&) = delete;
  PluginInfoDialog(PluginInfoDialog&&)      = delete;

  ~PluginInfoDialog() noexcept;

  PluginInfoDialog& operator=(const PluginInfoDialog&) = delete;
  PluginInfoDialog& operator=(PluginInfoDialog&&)      = delete;

  int exec() override;

private slots:
  void on_close_clicked();
  void on_nextFile_clicked();
  void on_previousFile_clicked();

  void on_previousArchiveButton_clicked();
  void on_nextArchiveButton_clicked();
  void on_archivesTreeStack_currentChanged(int index);
  void on_archiveFilterEdit_textChanged(const QString& text);

private:
  void setCurrent(const QString& pluginName);

  Ui::PluginInfoDialog* ui;

  TESData::PluginList* m_PluginList;
  QString m_PluginName;

  QStringList m_Archives;
  MOBase::ExpanderWidget m_WinningExpander;
  MOBase::ExpanderWidget m_LosingExpander;
  MOBase::ExpanderWidget m_NoConflictExpander;
  MOBase::FilterWidget m_FilterWinning;
  MOBase::FilterWidget m_FilterLosing;
  MOBase::FilterWidget m_FilterNoConflicts;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGININFODIALOG_H
