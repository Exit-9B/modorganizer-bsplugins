#include "PluginRecordView.h"
#include "ui_pluginrecordview.h"

#include <log.h>

namespace BSPluginInfo
{

PluginRecordView::PluginRecordView(MOBase::IOrganizer* organizer,
                                   TESData::PluginList* pluginList,
                                   const QString& pluginName, QWidget* parent)
    : QWidget(parent), ui{new Ui::PluginRecordView()}, m_Organizer{organizer},
      m_PluginList{pluginList}
{
  ui->setupUi(this);

  m_RecordModel = new PluginRecordModel(pluginList, pluginName.toStdString());
  ui->pickRecordView->setModel(m_RecordModel);

  connect(ui->pickRecordView->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &PluginRecordView::recordPicked);

  m_ConflictEntry = pluginList->findEntryByName(pluginName.toStdString());
}

PluginRecordView::~PluginRecordView() noexcept
{
  delete ui;
  delete m_RecordModel;
  delete m_StructureModel;
}

void PluginRecordView::recordPicked(const QModelIndex& current)
{
  const auto oldModel = m_StructureModel;
  m_StructureModel    = nullptr;

  if (current.isValid()) {
    if (m_ConflictEntry) {
      const auto path   = m_RecordModel->getPath(current);
      const auto record = m_ConflictEntry->findRecord(path).get();
      if (record) {
        m_StructureModel =
            new RecordStructureModel(m_PluginList, record, path, m_Organizer);
      }
    }
  }

  ui->recordStructureView->setModel(m_StructureModel);

  if (oldModel) {
    delete oldModel;
  }
}

}  // namespace BSPluginInfo
