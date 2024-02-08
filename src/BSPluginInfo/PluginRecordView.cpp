#include "PluginRecordView.h"
#include "MOPlugin/Settings.h"
#include "ui_pluginrecordview.h"

#include <QMenu>

#include <ranges>

namespace BSPluginInfo
{

PluginRecordView::PluginRecordView(QWidget* parent)
    : QWidget(parent), ui{new Ui::PluginRecordView()}
{
  ui->setupUi(this);
  ui->conflictFilterRow->hide();
}

void PluginRecordView::setup(MOBase::IOrganizer* organizer,
                             TESData::PluginList* pluginList, const QString& pluginName)
{
  m_Organizer   = organizer;
  m_PluginList  = pluginList;
  m_FilterProxy = new RecordFilterProxyModel(pluginList, pluginName);
  ui->conflictFilterRow->show();

  setFile(pluginName);

  ui->pickRecordView->header()->resizeSection(PluginRecordModel::COL_ID, 222);

  connect(ui->recordStructureView->header(), &QHeaderView::sectionMoved, this,
          &PluginRecordView::onFileHeaderMoved);

  const bool ignoreMasters =
      Settings::instance()->get<bool>("ignore_master_conflicts", false);

  ui->ignoreMasterConflicts->setChecked(ignoreMasters);
}

bool PluginRecordView::hasData() const
{
  return m_RecordModel->hasChildren();
}

QSplitter* PluginRecordView::splitter() const
{
  return ui->splitter;
}

QTreeView* PluginRecordView::pickRecordView() const
{
  return ui->pickRecordView;
}

QTreeView* PluginRecordView::recordStructureView() const
{
  return ui->recordStructureView;
}

void PluginRecordView::setFile(const QString& pluginName)
{
  const auto recordModel    = m_RecordModel;
  const auto structureModel = m_StructureModel;

  m_RecordModel =
      new PluginRecordModel(m_Organizer, m_PluginList, pluginName.toStdString());
  m_FilterProxy->setFile(pluginName);
  m_FilterProxy->setSourceModel(m_RecordModel);
  ui->pickRecordView->setModel(m_FilterProxy);

  m_StructureModel = nullptr;
  ui->recordStructureView->setModel(nullptr);
  ui->recordStructureView->setVisible(false);
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
  delete m_FilterProxy;
  delete m_StructureModel;
}

void PluginRecordView::onRecordPicked(const QModelIndex& current)
{
  if (!current.isValid()) {
    return;
  }

  const auto oldModel = m_StructureModel;
  m_StructureModel    = nullptr;

  const auto sourceIndex = m_FilterProxy->mapToSource(current);
  if (m_ConflictEntry) {
    const auto path   = m_RecordModel->getPath(sourceIndex);
    const auto record = m_ConflictEntry->findRecord(path).get();
    if (record) {
      m_StructureModel =
          new RecordStructureModel(m_PluginList, record, path, m_Organizer);
    }
  }

  ui->recordStructureView->setModel(m_StructureModel);
  ui->recordStructureView->setVisible(m_StructureModel != nullptr);
  expandStructureConflicts();

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
      m_RecordModel->emit dataChanged(QModelIndex(), QModelIndex());
      m_StructureModel->refresh();
    }
  }

  ui->recordStructureView->header()->moveSection(newVisualIndex, oldVisualIndex);
  m_MovingSection = false;
}

void PluginRecordView::on_pickRecordView_expanded(const QModelIndex& index)
{
  const auto model = ui->pickRecordView->model();
  for (int row = 0, count = model->rowCount(index); row < count; ++row) {
    const auto child = model->index(row, 0, index);

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

void PluginRecordView::on_recordStructureView_expanded(const QModelIndex& index)
{
  const auto model   = ui->recordStructureView->model();
  const int rowCount = model->rowCount(index);
  if (rowCount == 1) {
    const auto child = model->index(0, 0, index);
    if (model->hasChildren(child)) {
      ui->recordStructureView->expand(child);
    }
  } else {
    for (int i = 0; i < rowCount; ++i) {
      const auto child = model->index(i, 0, index);
      if (model->hasChildren(child) && child.data().toString().isEmpty()) {
        ui->recordStructureView->expand(child);
      }
    }
  }
}

void PluginRecordView::on_filterCombo_currentIndexChanged(int index)
{
  switch (index) {
  case Filter_AllConflicts:
    m_FilterProxy->setFilterFlags(RecordFilterProxyModel::Filter_AllConflicts);
    break;
  case Filter_WinningConflicts:
    m_FilterProxy->setFilterFlags(RecordFilterProxyModel::Filter_WinningConflicts);
    break;
  case Filter_LosingConflicts:
    m_FilterProxy->setFilterFlags(RecordFilterProxyModel::Filter_LosingConflicts);
    break;
  }
}

void PluginRecordView::on_ignoreMasterConflicts_stateChanged(int state)
{
  Settings::instance()->set("ignore_master_conflicts", state == Qt::Checked);
  m_RecordModel->emit dataChanged(QModelIndex(), QModelIndex());
}

void PluginRecordView::expandStructureConflicts(const QModelIndex& parent)
{
  if (!m_StructureModel) {
    return;
  }

  const int count = m_StructureModel->rowCount(parent);
  for (int i = 0; i < count; ++i) {
    using Item          = TESData::DataItem;
    const auto idx      = m_StructureModel->index(i, 0, parent);
    const auto item     = idx.data(Qt::UserRole).value<const Item*>();
    const int fileCount = m_StructureModel->columnCount(parent) - 1;

    if (item->numChildren() > 0 && item->isConflicted(fileCount)) {
      static constexpr int EXPAND_MAX_ROWS = 80;

      if (item->numChildren() <= EXPAND_MAX_ROWS) {
        ui->recordStructureView->expand(idx);
        expandStructureConflicts(idx);
      }
    }
  }
}

}  // namespace BSPluginInfo
