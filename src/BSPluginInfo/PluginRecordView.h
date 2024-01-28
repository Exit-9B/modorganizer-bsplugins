#ifndef BSPLUGININFO_PLUGINRECORDVIEW_H
#define BSPLUGININFO_PLUGINRECORDVIEW_H

#include "PluginRecordModel.h"
#include "RecordStructureModel.h"

#include <QWidget>

namespace Ui
{
class PluginRecordView;
}

namespace BSPluginInfo
{

class PluginRecordView final : public QWidget
{
  Q_OBJECT

public:
  PluginRecordView(MOBase::IOrganizer* organizer, TESData::PluginList* pluginList,
                   const QString& pluginName, QWidget* parent = nullptr);

  PluginRecordView(const PluginRecordView&) = delete;
  PluginRecordView(PluginRecordView&&)      = delete;

  ~PluginRecordView() noexcept;

  PluginRecordView& operator=(const PluginRecordView&) = delete;
  PluginRecordView& operator=(PluginRecordView&&)      = delete;

private slots:
  void recordPicked(const QModelIndex& current);

  void on_pickRecordView_expanded(const QModelIndex& index);

private:
  Ui::PluginRecordView* ui;
  MOBase::IOrganizer* m_Organizer;
  const TESData::FileEntry* m_ConflictEntry = nullptr;
  TESData::PluginList* m_PluginList         = nullptr;
  PluginRecordModel* m_RecordModel          = nullptr;
  RecordStructureModel* m_StructureModel    = nullptr;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGINRECORDVIEW_H
