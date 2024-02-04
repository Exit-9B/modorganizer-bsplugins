#ifndef BSPLUGININFO_PLUGINRECORDVIEW_H
#define BSPLUGININFO_PLUGINRECORDVIEW_H

#include "PluginRecordModel.h"
#include "RecordFilterProxyModel.h"
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
  enum ConflictFilter
  {
    Filter_AllConflicts,
    Filter_WinningConflicts,
    Filter_LosingConflicts,
  };

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
  void onRecordPicked(const QModelIndex& current);
  void onFileHeaderMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);

  void on_pickRecordView_expanded(const QModelIndex& index);
  void on_pickRecordView_customContextMenuRequested(const QPoint& pos);
  void on_recordStructureView_expanded(const QModelIndex& index);
  void on_filterCombo_currentIndexChanged(int index);

private:
  void expandStructureConflicts(const QModelIndex& parent = QModelIndex());

  Ui::PluginRecordView* ui;
  MOBase::IOrganizer* m_Organizer           = nullptr;
  const TESData::FileEntry* m_ConflictEntry = nullptr;
  TESData::PluginList* m_PluginList         = nullptr;
  PluginRecordModel* m_RecordModel          = nullptr;
  RecordFilterProxyModel* m_FilterProxy     = nullptr;
  RecordStructureModel* m_StructureModel    = nullptr;
  bool m_MovingSection                      = false;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGINRECORDVIEW_H
