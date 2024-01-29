#include "PluginRecordView.h"
#include "ui_pluginrecordview.h"

#include <QMenu>

#include <ranges>

namespace BSPluginInfo
{

PluginRecordView::PluginRecordView(QWidget* parent)
    : QWidget(parent), ui{new Ui::PluginRecordView()}
{
  ui->setupUi(this);
}

void PluginRecordView::setup(MOBase::IOrganizer* organizer,
                             TESData::PluginList* pluginList, const QString& pluginName)
{
  m_Organizer  = organizer;
  m_PluginList = pluginList;

  setFile(pluginName);

  ui->pickRecordView->header()->resizeSection(PluginRecordModel::COL_ID, 220);
}

void PluginRecordView::setFile(const QString& pluginName)
{
  const auto recordModel    = m_RecordModel;
  const auto structureModel = m_StructureModel;

  m_RecordModel =
      new PluginRecordModel(m_Organizer, m_PluginList, pluginName.toStdString());
  ui->pickRecordView->setModel(m_RecordModel);

  m_StructureModel = nullptr;
  ui->recordStructureView->setModel(nullptr);
  on_pickRecordView_expanded(QModelIndex());

  connect(ui->pickRecordView->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &PluginRecordView::recordPicked);

  m_ConflictEntry = m_PluginList->findEntryByName(pluginName.toStdString());

  delete recordModel;
  delete structureModel;
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
  const auto selectedRows   = selectionModel->selectedRows();
  const auto items = std::ranges::transform_view(selectedRows, [](auto&& index) {
    using Item      = TESData::FileEntry::TreeItem;
    const auto item = index.data(Qt::UserRole).value<const Item*>();
    return item;
  });

  if (items.empty() || !std::ranges::all_of(items, [](auto&& item) {
        return item && item->record;
      })) {
    return;
  }

  QMenu menu;

  QAction* ignoreRecord;
  ignoreRecord = menu.addAction(tr("Ignore Record"), [&] {
    for (auto&& item : items) {
      item->record->setIgnored(ignoreRecord->isChecked());

      for (const auto handle : item->record->alternatives()) {
        const auto entry = m_PluginList->findEntryByHandle(handle);
        const auto info =
            m_PluginList->getPluginByName(QString::fromStdString(entry->name()));
        if (info) {
          info->invalidateConflicts();
        }
      }
    }
  });

  ignoreRecord->setCheckable(true);
  ignoreRecord->setChecked(std::ranges::all_of(items, [](auto&& item) {
    return item->record->ignored();
  }));

  const QPoint p = ui->pickRecordView->viewport()->mapToGlobal(pos);
  menu.exec(p);
}

}  // namespace BSPluginInfo
