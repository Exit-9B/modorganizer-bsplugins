#include "PluginRecordModel.h"
#include "TESData/BranchConflictParser.h"
#include "TESData/TypeStringNames.h"
#include "TESFile/Reader.h"

#include <algorithm>
#include <iterator>
#include <ranges>

using namespace Qt::Literals::StringLiterals;

namespace BSPluginInfo
{

PluginRecordModel::PluginRecordModel(MOBase::IOrganizer* organizer,
                                     TESData::PluginList* pluginList,
                                     const QString& pluginName)
    : m_PluginName{pluginName}, m_Organizer{organizer}, m_PluginList{pluginList},
      m_FileEntry{pluginList->findEntryByName(pluginName.toStdString())}
{
  if (m_FileEntry) {
    m_DataRoot = m_FileEntry->dataRoot();
  }
}

TESData::RecordPath PluginRecordModel::getPath(const QModelIndex& index) const
{
  TESData::RecordPath path;

  std::vector<const Item*> parents;
  const auto last = static_cast<const Item*>(index.internalPointer());
  for (auto item = last; item->parent; item = item->parent) {
    parents.push_back(item);
  }

  for (const auto& item : std::ranges::reverse_view(parents)) {
    if (item->group.has_value()) {
      path.push(item->group.value(), m_FileEntry->files(), m_PluginName.toStdString());
    }
  }

  if (last->record) {
    if (last->group.has_value()) {
      path.pop();
    }
    path.setIdentifier(last->record->identifier(), {&last->record->file(), 1});
  }

  return path;
}

QModelIndex PluginRecordModel::index(int row, int column,
                                     const QModelIndex& parent) const
{
  if (row < 0 || column < 0)
    return QModelIndex();

  const auto parentItem = parent.isValid()
                              ? static_cast<const Item*>(parent.internalPointer())
                              : m_DataRoot;

  if (!parentItem || row >= parentItem->children.size())
    return QModelIndex();

  const auto item = parentItem->children.nth(row)->second.get();
  return createIndex(row, column, item);
}

QModelIndex PluginRecordModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  const auto item = static_cast<const Item*>(index.internalPointer());
  if (!item->parent || !item->parent->parent) {
    return QModelIndex();
  }

  const auto& siblings = item->parent->parent->children;
  const int row        = static_cast<int>(
      siblings.index_of(std::ranges::find(siblings, item->parent, [&](auto&& pair) {
        return pair.second.get();
      })));

  return createIndex(row, 0, item->parent);
}

bool PluginRecordModel::hasChildren(const QModelIndex& parent) const
{
  if (!parent.isValid()) {
    return m_DataRoot && !m_DataRoot->children.empty();
  }

  const auto parentItem = static_cast<const Item*>(parent.internalPointer());
  return parentItem ? parentItem->group.has_value() : false;
}

int PluginRecordModel::rowCount(const QModelIndex& parent) const
{
  const auto parentItem =
      parent.isValid() ? static_cast<Item*>(parent.internalPointer()) : m_DataRoot;

  return parentItem ? static_cast<int>(parentItem->children.size()) : 0;
}

int PluginRecordModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return COL_COUNT;
}

bool PluginRecordModel::canFetchMore(const QModelIndex& parent) const
{
  if (!parent.isValid()) {
    return false;
  }

  const auto parentItem = static_cast<const Item*>(parent.internalPointer());
  return parentItem && parentItem->group.has_value() && parentItem->children.empty();
}

void PluginRecordModel::fetchMore(const QModelIndex& parent)
{
  const auto parentItem =
      parent.isValid() ? static_cast<Item*>(parent.internalPointer()) : nullptr;

  if (!parentItem || !parentItem->children.empty() || !parentItem->group.has_value()) {
    return;
  }

  QList<QString> names;
  if (parentItem->record) {
    for (const auto handle : parentItem->record->alternatives()) {
      const auto entry = m_PluginList->findEntryByHandle(handle);
      names.append(QString::fromStdString(entry->name()));
    }
  } else {
    names.append(m_PluginName);
  }

  for (const auto& name : names) {
    const auto vfsEntry =
        m_Organizer->virtualFileTree()->find(name, MOBase::FileTreeEntry::FILE);

    const auto filePath = m_Organizer->resolvePath(name);

    try {
      TESData::BranchConflictParser handler{m_PluginList, name.toStdString(),
                                            getPath(parent)};
      TESFile::Reader<TESData::BranchConflictParser> reader{};
      reader.parse(std::filesystem::path(filePath.toStdWString()), handler);
    } catch (const std::exception& e) {
      MOBase::log::error("Error parsing \"{}\": {}", filePath, e.what());
    }
  }

  if (parentItem->children.empty()) {
    beginRemoveRows(parent, 0, 0);
    parentItem->group = std::nullopt;
    endRemoveRows();
  }
}

QVariant PluginRecordModel::data(const QModelIndex& index, int role) const
{
  const auto item = static_cast<const Item*>(index.internalPointer());

  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole: {
    switch (index.column()) {
    case COL_ID: {
      if (item->record) {
        if (item->record->hasFormId()) {
          std::uint32_t formId = item->record->formId() & 0xFFFFFFU;

          const auto plugin =
              m_PluginList ? m_PluginList->getPluginByName(m_PluginName) : nullptr;

          if (plugin) {
            const auto file = QString::fromStdString(item->record->file());
            const auto localIndex =
                std::distance(std::begin(plugin->masters()),
                              std::ranges::find(plugin->masters(), file));
            formId |= ((localIndex & 0xFF) << 24U);
          }

          QString str = u"%1"_s.arg(formId, 8, 16, QChar(u'0')).toUpper();

          for (const Item* p = item->parent; p; p = p->parent) {
            if (p->group && p->group->hasFormType()) {
              if (item->formType != p->group->formType()) {
                QString suffix = TESData::getFormName(item->formType).toString();
                if (suffix.isEmpty()) {
                  suffix = QString::fromLocal8Bit(item->formType.data(),
                                                  item->formType.size());
                }
                str = u"%1 %2"_s.arg(str).arg(suffix);
              }
              break;
            }
          }

          return str;

        } else if (item->record->hasEditorId()) {
          return QString::fromStdString(item->record->editorId());

        } else if (item->record->hasTypeId()) {
          const auto type    = item->record->typeId();
          const auto typestr = QString::fromLocal8Bit(type.data(), type.size());
          const auto name    = TESData::getDefaultObjectName(type);
          if (!name.isEmpty()) {
            return u"%1 - %2"_s.arg(typestr).arg(name);
          } else {
            return typestr;
          }
        }
      }

      if (item->group.has_value()) {
        return makeGroupName(item->group.value());
      }

      return QVariant();
    }

    case COL_OWNER: {
      if (item->record && item->record->hasFormId()) {
        const auto file = item->record->file();
        return QString::fromStdString(file);
      }
      return QVariant();
    }

    case COL_NAME: {
      if (item->record && item->record->hasFormId()) {
        return QString::fromStdString(item->name);
      }
      return QVariant();
    }
    }

    return QVariant();
  }

  case Qt::BackgroundRole: {
    if (!m_PluginList || !m_FileEntry || !item->record) {
      return QVariant();
    }

    const auto info = m_PluginList->getPluginByName(m_PluginName);
    if (!info) {
      return QVariant();
    }

    bool isConflicted = false;
    for (const auto alternative : item->record->alternatives()) {
      const auto altEntry = m_PluginList->findEntryByHandle(alternative);
      if (!altEntry || altEntry == m_FileEntry)
        continue;

      const auto altInfo =
          m_PluginList->getPluginByName(QString::fromStdString(altEntry->name()));
      if (!altInfo || !altInfo->enabled())
        continue;

      isConflicted = true;
      if (altInfo->priority() > info->priority()) {
        return QColor(255, 0, 0, 64);
      }
    }

    if (isConflicted) {
      return QColor(0, 255, 0, 64);
    } else {
      return QVariant();
    }
  }

  case Qt::UserRole: {
    return QVariant::fromValue(item);
  }
  }

  return QVariant();
}

QString PluginRecordModel::makeGroupName(TESFile::GroupData group)
{
  using GroupType = TESFile::GroupType;

  switch (group.type()) {
  case GroupType::Top: {
    const auto type    = group.formType();
    const auto typestr = QString::fromLocal8Bit(type.data(), type.size());
    const auto name    = TESData::getFormName(type);
    if (!name.isEmpty()) {
      return u"%1 - %2"_s.arg(typestr).arg(name);
    } else {
      return typestr;
    }
  }
  case GroupType::WorldChildren:
    return tr("Children");
  case GroupType::InteriorCellBlock:
    return tr("Block %1").arg(group.block());
  case GroupType::InteriorCellSubBlock:
    return tr("Sub-Block %1").arg(group.block());
  case GroupType::ExteriorCellBlock: {
    const auto [x, y] = group.gridCell();
    return tr("Block %1, %2").arg(x).arg(y);
  }
  case GroupType::ExteriorCellSubBlock: {
    const auto [x, y] = group.gridCell();
    return tr("Sub-Block %1, %2").arg(x).arg(y);
  }
  case GroupType::CellChildren:
    return tr("Children");
  case GroupType::TopicChildren:
    return tr("Children");
  case GroupType::CellPersistentChildren:
    return tr("Persistent");
  case GroupType::CellTemporaryChildren:
    return tr("Temporary");
  case GroupType::CellVisibleDistantChildren:
    return tr("Visible Distant");
  default:
    return tr("Children");
  }
}

QVariant PluginRecordModel::headerData(int section, Qt::Orientation orientation,
                                       int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return QVariant();

  switch (section) {
  case COL_ID:
    return tr("Form");
  case COL_OWNER:
    return tr("Origin");
  case COL_NAME:
    return tr("Editor ID");
  }

  return QVariant();
}

}  // namespace BSPluginInfo
