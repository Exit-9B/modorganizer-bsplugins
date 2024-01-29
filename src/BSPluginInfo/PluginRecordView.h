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
  explicit PluginRecordView(QWidget* parent = nullptr);

  PluginRecordView(const PluginRecordView&) = delete;
  PluginRecordView(PluginRecordView&&)      = delete;

  ~PluginRecordView() noexcept;

  PluginRecordView& operator=(const PluginRecordView&) = delete;
  PluginRecordView& operator=(PluginRecordView&&)      = delete;

  void setup(MOBase::IOrganizer* organizer, TESData::PluginList* pluginList,
             const QString& pluginName);

  void setFile(const QString& pluginName);

private slots:
  void recordPicked(const QModelIndex& current);

  void on_pickRecordView_expanded(const QModelIndex& index);
  void on_pickRecordView_customContextMenuRequested(const QPoint& pos);

private:
  Ui::PluginRecordView* ui;
  MOBase::IOrganizer* m_Organizer           = nullptr;
  const TESData::FileEntry* m_ConflictEntry = nullptr;
  TESData::PluginList* m_PluginList         = nullptr;
  PluginRecordModel* m_RecordModel          = nullptr;
  RecordStructureModel* m_StructureModel    = nullptr;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGINRECORDVIEW_H
