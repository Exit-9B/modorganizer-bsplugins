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

  connect(ui->recordStructureView->header(), &QHeaderView::sectionMoved, this,
          &PluginRecordView::onFileHeaderMoved);
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
          this, &PluginRecordView::onRecordPicked);

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

void PluginRecordView::onRecordPicked(const QModelIndex& current)
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

void PluginRecordView::onFileHeaderMoved(int logicalIndex, int oldVisualIndex,
                                         int newVisualIndex)
{
  if (!m_PluginList || m_MovingSection) {
    return;
  }

  m_MovingSection = true;

  const auto& file = m_StructureModel->file(logicalIndex - 1);
  const int index  = m_PluginList->getIndex(file);
  const auto info  = m_PluginList->getPlugin(index);
  if (!info) {
    return;
  }

  const auto& otherFile = m_StructureModel->file(newVisualIndex - 1);
  const auto otherInfo  = m_PluginList->getPluginByName(otherFile);
  if (otherInfo) {
    int destination = otherInfo->priority();
    if (newVisualIndex > oldVisualIndex)
      ++destination;

    if (m_PluginList->canMoveToPriority({index}, destination)) {
      m_PluginList->moveToPriority({index}, destination);
    }
  }

  ui->recordStructureView->header()->moveSection(newVisualIndex, oldVisualIndex);
  onRecordPicked(ui->pickRecordView->selectionModel()->currentIndex());
  m_MovingSection = false;
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
