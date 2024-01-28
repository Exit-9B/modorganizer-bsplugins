#include "PluginInfoDialog.h"
#include "PluginRecordView.h"

#include <QVBoxLayout>

namespace BSPluginInfo
{

PluginInfoDialog::PluginInfoDialog(MOBase::IOrganizer* organizer,
                                   TESData::PluginList* pluginList,
                                   const QString& pluginName, QWidget* parent)
    : QDialog(parent)
{
  setWindowTitle(pluginName);
  resize(1024, 768);
  auto verticalLayout = new QVBoxLayout(this);
  verticalLayout->setObjectName("verticalLayout");
  auto pluginRecordView = new PluginRecordView(organizer, pluginList, pluginName, this);
  verticalLayout->addWidget(pluginRecordView);
}

}  // namespace BSPluginInfo
