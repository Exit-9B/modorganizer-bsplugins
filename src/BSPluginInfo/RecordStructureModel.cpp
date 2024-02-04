#include "RecordStructureModel.h"
#include "TESData/SingleRecordParser.h"
#include "TESFile/Reader.h"

#include <log.h>

#include <boost/container/flat_map.hpp>

namespace BSPluginInfo
{

RecordStructureModel::RecordStructureModel(TESData::PluginList* pluginList,
                                           TESData::Record* record,
                                           const TESData::RecordPath& path,
                                           MOBase::IOrganizer* organizer)
    : m_Organizer{organizer}, m_PluginList{pluginList}, m_Record{record}, m_Path{path},
      m_Root{std::make_shared<Item>()}
{
  refresh();
}

void RecordStructureModel::refresh()
{
  boost::container::flat_map<int, const TESData::FileEntry*> entries;
  for (const auto handle : m_Record->alternatives()) {
    const auto entry = m_PluginList->findEntryByHandle(handle);
    const auto info =
        m_PluginList->getPluginByName(QString::fromStdString(entry->name()));
    if (info && info->enabled()) {
      entries.emplace(info->priority(), entry);
    }
  }

  m_Files.resize(entries.size());

  for (int index = static_cast<int>(entries.size()) - 1; index >= 0; --index) {
    const auto entry = entries.nth(index)->second;
    const auto name  = QString::fromStdString(entry->name());
    const auto vfsEntry =
        m_Organizer->virtualFileTree()->find(name, MOBase::FileTreeEntry::FILE);

    const auto filePath = m_Organizer->resolvePath(name);
    m_Files[index]      = QFileInfo(filePath).fileName();

    readFile(m_Path, filePath, index);
  }
}

void RecordStructureModel::readFile(const TESData::RecordPath& path,
                                    const QString& filePath, int index)
try {
  const auto fileName = QFileInfo(filePath).fileName().toStdString();
  const auto gameName = m_Organizer->managedGame()->gameName();
  TESData::SingleRecordParser handler(gameName, path, fileName, m_Root.get(), index);
  TESFile::Reader<TESData::SingleRecordParser> reader{};
  reader.parse(std::filesystem::path(filePath.toStdWString()), handler);
} catch (const std::exception& e) {
  MOBase::log::error("Error parsing \"{}\": {}", filePath, e.what());
}

QModelIndex RecordStructureModel::index(int row, int column,
                                        const QModelIndex& parent) const
{
  if (row < 0 || column < 0) {
    return QModelIndex();
  }

  const auto parentItem = parent.isValid()
                              ? static_cast<const Item*>(parent.internalPointer())
                              : m_Root.get();
  if (parentItem && row < parentItem->numChildren()) {
    const auto childItem = parentItem->childAt(row);
    return createIndex(row, column, childItem);
  }

  return QModelIndex();
}

QModelIndex RecordStructureModel::parent(const QModelIndex& index) const
{
  const auto item =
      index.isValid() ? static_cast<const Item*>(index.internalPointer()) : nullptr;
  const auto parentItem = item ? item->parent() : nullptr;
  if (!parentItem) {
    return QModelIndex();
  }

  const int row = parentItem->index();
  return createIndex(row, 0, parentItem);
}

int RecordStructureModel::rowCount(const QModelIndex& parent) const
{
  const auto item = parent.isValid()
                        ? static_cast<const Item*>(parent.internalPointer())
                        : m_Root.get();
  return item ? item->numChildren() : 0;
}

int RecordStructureModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return 1 + static_cast<int>(m_Files.size());
}

QVariant RecordStructureModel::data(const QModelIndex& index, int role) const
{
  const auto item =
      index.isValid() ? static_cast<const Item*>(index.internalPointer()) : nullptr;
  if (!item) {
    return QVariant();
  }

  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    if (index.column() == 0) {
      return item->rowHeader();
    } else {
      return item->displayData(index.column() - 1);
    }

  case Qt::BackgroundRole: {
    if (index.column() == 0) {
      return QVariant();
    }

    const int count     = columnCount() - 1;
    const int fileIndex = index.column() - 1;
    if (item->isLosingConflict(fileIndex, count)) {
      return QColor(255, 0, 0, 64);
    } else if (item->isOverriding(fileIndex)) {
      return QColor(0, 255, 0, 64);
    }

    return QVariant();
  }
  }

  return QVariant();
}

QVariant RecordStructureModel::headerData(int section, Qt::Orientation orientation,
                                          int role) const
{
  if (orientation != Qt::Horizontal)
    return QVariant();

  switch (role) {
  case Qt::DisplayRole: {
    if (section == 0) {
      // Name
      return QVariant();
    }

    if (section - 1 < m_Files.length()) {
      return m_Files[section - 1];
    }
  }
  }

  return QVariant();
}

}  // namespace BSPluginInfo
