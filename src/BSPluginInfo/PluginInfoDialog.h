#ifndef BSPLUGININFO_PLUGININFODIALOG_H
#define BSPLUGININFO_PLUGININFODIALOG_H

#include "TESData/PluginList.h"

#include <imoinfo.h>

#include <QDialog>
#include <QString>

namespace BSPluginInfo
{

class PluginInfoDialog final : public QDialog
{
  Q_OBJECT

public:
  PluginInfoDialog(MOBase::IOrganizer* organizer, TESData::PluginList* pluginList,
                   const QString& pluginName, QWidget* parent = nullptr);
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGININFODIALOG_H
