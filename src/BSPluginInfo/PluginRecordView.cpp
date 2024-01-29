#include "PluginRecordView.h"
#include "ui_pluginrecordview.h"

#include <QMenu>

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
  ui->pickRecordView->header()->resizeSection(PluginRecordModel::COL_ID, 220);
  on_pickRecordView_expanded(QModelIndex());

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

void PluginRecordView::on_pickRecordView_expanded(const QModelIndex& index)
{
  for (int row = 0, count = m_RecordModel->rowCount(index); row < count; ++row) {
    const auto child = m_RecordModel->index(row, 0, index);

    using Item      = TESData::FileEntry::TreeItem;
    const auto item = child.data(Qt::UserRole).value<const Item*>();
    if (!item || !item->record || !item->record->hasFormId()) {
      ui->pickRecordView->setFirstColumnSpanned(row, index, true);
    }
  }
}

void PluginRecordView::on_pickRecordView_customContextMenuRequested(const QPoint& pos)
{
  const auto selectionModel = ui->pickRecordView->selectionModel();
  const auto currentIndex   = selectionModel->currentIndex();

  using Item      = TESData::FileEntry::TreeItem;
  const auto item = currentIndex.data(Qt::UserRole).value<const Item*>();
  if (!item || !item->record)
    return;

  QMenu menu;

  QAction* ignoreRecord;
  ignoreRecord = menu.addAction(tr("Ignore Record"), [&] {
    item->record->setIgnored(ignoreRecord->isChecked());
    for (const auto handle : item->record->alternatives()) {
      const auto entry = m_PluginList->findEntryByHandle(handle);
      const auto info =
          m_PluginList->getPluginByName(QString::fromStdString(entry->name()));
      if (info) {
        info->invalidateConflicts();
      }
    }
  });
  ignoreRecord->setCheckable(true);
  ignoreRecord->setChecked(item->record->ignored());

  const QPoint p = ui->pickRecordView->viewport()->mapToGlobal(pos);
  menu.exec(p);
}

}  // namespace BSPluginInfo
