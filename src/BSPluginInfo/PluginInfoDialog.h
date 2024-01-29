#ifndef BSPLUGININFO_PLUGININFODIALOG_H
#define BSPLUGININFO_PLUGININFODIALOG_H

#include "TESData/PluginList.h"

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
  PluginInfoDialog(PluginInfoDialog&&) = delete;

  ~PluginInfoDialog() noexcept;

  PluginInfoDialog& operator=(const PluginInfoDialog&) = delete;
  PluginInfoDialog& operator=(PluginInfoDialog&&) = delete;

private slots:
  void on_close_clicked();
  void on_nextFile_clicked();
  void on_previousFile_clicked();

private:
  void setCurrent(const QString& pluginName);

  Ui::PluginInfoDialog* ui;

  TESData::PluginList* m_PluginList;
  QString m_PluginName;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGININFODIALOG_H
