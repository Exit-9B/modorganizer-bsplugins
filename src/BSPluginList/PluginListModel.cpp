#include "PluginListModel.h"
#include "PluginListDropInfo.h"

#include <QMimeData>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

namespace BSPluginList
{

PluginListModel::PluginListModel(TESData::PluginList* plugins) : m_Plugins{plugins} {}

QModelIndex PluginListModel::index(int row, int column,
                                   [[maybe_unused]] const QModelIndex& parent) const
{
  if ((row < 0) || (row >= rowCount()) || (column < 0) || (column >= columnCount())) {
    return QModelIndex();
  }
  return createIndex(row, column, row);
}

QModelIndex PluginListModel::parent([[maybe_unused]] const QModelIndex& parent) const
{
  return QModelIndex();
}

Qt::ItemFlags PluginListModel::flags(const QModelIndex& index) const
{
  const int id = index.row();

  Qt::ItemFlags result = QAbstractItemModel::flags(index);

  if (index.isValid()) {
    const auto plugin = m_Plugins->getPlugin(id);
    if (!plugin ||
        !plugin->forceLoaded() && !plugin->forceEnabled() && !plugin->forceDisabled())
      result |= Qt::ItemIsUserCheckable;
    if (index.column() == COL_PRIORITY)
      result |= Qt::ItemIsEditable;
    result |= Qt::ItemIsDragEnabled;
    result &= ~Qt::ItemIsDropEnabled;
  } else {
    result |= Qt::ItemIsDropEnabled;
  }

  return result;
}

QVariant PluginListModel::data(const QModelIndex& index, int role) const
{
  switch (role) {
  case Qt::DisplayRole:
  case Qt::EditRole:
    return displayData(index);
  case Qt::CheckStateRole:
    if (index.column() == 0) {
      return checkstateData(index);
    }
    break;
  case Qt::ForegroundRole:
    return foregroundData(index);
  case Qt::BackgroundRole:
    return backgroundData(index);
  case Qt::FontRole:
    return fontData(index);
  case Qt::TextAlignmentRole:
    return alignmentData(index);
  case Qt::ToolTipRole:
    return tooltipData(index);
  case ConflictsIconRole:
    return conflictData(index);
  case FlagsIconRole:
    return iconData(index);
  case IndexRole:
    return index.row();
  case InfoRole: {
    const auto id     = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    return QVariant::fromValue(plugin);
  }
  case OriginRole: {
    const int id = index.row();
    return m_Plugins->getOriginName(id);
  }
  case OverridingRole: {
    const int id      = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    if (!plugin || !plugin->enabled()) {
      return QVariantList();
    }

    QVariantList list;
    for (const int otherId : plugin->getPluginOverriding()) {
      const auto other = m_Plugins->getPlugin(otherId);
      if (other && other->enabled()) {
        list.append(otherId);
      }
    }
    return list;
  }
  case OverriddenRole: {
    const int id      = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    if (!plugin || !plugin->enabled()) {
      return QVariantList();
    }

    QVariantList list;
    for (const int otherId : plugin->getPluginOverridden()) {
      const auto other = m_Plugins->getPlugin(otherId);
      if (other && other->enabled()) {
        list.append(otherId);
      }
    }
    return list;
  }
  }
  return QVariant();
}

QVariant PluginListModel::displayData(const QModelIndex& index) const
{
  const int id = index.row();
  switch (index.column()) {
  case COL_NAME:
    return m_Plugins->getPlugin(id)->name();
  case COL_PRIORITY:
    return m_Plugins->getPlugin(id)->priority();
  case COL_MODINDEX:
    return m_Plugins->getPlugin(id)->index();
  default:
    return QVariant();
  }
}

QVariant PluginListModel::checkstateData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);

  if (plugin->forceLoaded() || plugin->forceEnabled()) {
    return QVariant();
  } else if (plugin->forceDisabled()) {
    return QVariant();
  }

  return m_Plugins->getPlugin(id)->enabled() ? Qt::Checked : Qt::Unchecked;
}

QVariant PluginListModel::foregroundData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);

  if (index.column() == COL_NAME) {
    if (plugin->hasNoRecords()) {
      return QBrush(Qt::gray);
    }
    if (plugin->forceDisabled()) {
      return QBrush(Qt::darkRed);
    }
  }

  return QVariant();
}

QVariant PluginListModel::backgroundData(const QModelIndex& index) const
{
  return QVariant();
}

QVariant PluginListModel::fontData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);

  QFont result;

  if (index.column() == COL_NAME) {
    if (plugin->hasNoRecords()) {
      result.setItalic(true);
    }
  }

  return result;
}

QVariant PluginListModel::alignmentData(const QModelIndex& index) const
{
  if (index.column() == 0) {
    return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
  } else {
    return QVariant(Qt::AlignHCenter | Qt::AlignVCenter);
  }
}

static QString truncateString(const QString& text, int length = 1024)
{
  QString new_text = text;

  if (new_text.length() > length) {
    new_text.truncate(length);
    new_text += "...";
  }

  return new_text;
}

static QString makeLootTooltip(const MOTools::Loot::Plugin& lootInfo)
{
  QString s;
  for (const auto& f : lootInfo.incompatibilities) {
    s += "<li>" +
         QObject::tr("Incompatible with %1")
             .arg(!f.displayName.isEmpty() ? f.displayName : f.name) +
         "</li>";
  }

  for (const auto& m : lootInfo.messages) {
    s += "<li>";

    switch (m.type) {
    case MOBase::log::Warning:
      s += QObject::tr("Warning") + ": ";
      break;

    case MOBase::log::Error:
      s += QObject::tr("Error") + ": ";
      break;

    case MOBase::log::Info:
    case MOBase::log::Debug:
      break;
    }

    s += m.text + "</li>";
  }

  for (const auto& d : lootInfo.dirty) {
    s += "<li>" + d.toString(false) + "</li>";
  }

  for (const auto& c : lootInfo.clean) {
    s += "<li>" + c.toString(true) + "</li>";
  }

  if (s.isEmpty()) {
    return s;
  }

  return "<hr>"
         "<ul style=\"margin-top:0px; padding-top:0px; margin-left:15px; "
         "-qt-list-indent: 0;\">" +
         s + "</ul>";
}

QVariant PluginListModel::tooltipData(const QModelIndex& index) const
{
  const int id        = index.row();
  const auto plugin   = m_Plugins->getPlugin(id);
  const auto lootInfo = m_Plugins->getLootReport(plugin->name());

  QString toolTip;

  toolTip += "<b>" + tr("Origin") + "</b>: " + m_Plugins->getOriginName(id);

  if (plugin->forceLoaded()) {
    toolTip += "<br><b><i>" +
               tr("This plugin can't be disabled or moved (enforced by the game).") +
               "</i></b>";
  }

  if (plugin->forceEnabled()) {
    toolTip += "<br><b><i>" +
               tr("This plugin can't be disabled (enforced by the game).") + "</i></b>";
  }

  if (!plugin->author().isEmpty()) {
    toolTip += "<br><b>" + tr("Author") + "</b>: " + truncateString(plugin->author());
  }

  if (plugin->description().size() > 0) {
    toolTip += "<br><b>" + tr("Description") +
               "</b>: " + truncateString(plugin->description());
  }

  if (plugin->missingMasters().size() > 0) {
    toolTip += "<br><b>" + tr("Missing Masters") + "</b>: " + "<b>" +
               truncateString(QStringList(plugin->missingMasters().begin(),
                                          plugin->missingMasters().end())
                                  .join(", ")) +
               "</b>";
  }

  std::set<QString> enabledMasters;
  std::ranges::set_difference(plugin->masters(), plugin->missingMasters(),
                              std::inserter(enabledMasters, enabledMasters.end()),
                              MOBase::FileNameComparator{});

  if (!enabledMasters.empty()) {
    toolTip += "<br><b>" + tr("Enabled Masters") +
               "</b>: " + truncateString(MOBase::SetJoin(enabledMasters, ", "));
  }

  if (!plugin->archives().empty()) {
    QString archiveString = plugin->archives().size() < 6
                                ? truncateString(QStringList(plugin->archives().begin(),
                                                             plugin->archives().end())
                                                     .join(", ")) +
                                      "<br>"
                                : "";
    toolTip += "<br><b>" + tr("Loads Archives") + "</b>: " + archiveString +
               tr("There are Archives connected to this plugin. Their assets will be "
                  "added to your game, overwriting in case of conflicts following the "
                  "plugin order. Loose files will always overwrite assets from "
                  "Archives.");
  }

  if (plugin->hasIni()) {
    toolTip += "<br><b>" + tr("Loads INI settings") +
               "</b>: "
               "<br>" +
               tr("There is an ini file connected to this plugin. Its settings will "
                  "be added to your game settings, overwriting in case of conflicts.");
  }

  if (plugin->isLightFlagged() && !plugin->isMasterFlagged() &&
      !plugin->hasLightExtension()) {
    QString type = plugin->hasMasterExtension() ? "ESM" : "ESP";
    toolTip +=
        "<br><br>" + tr("This %1 is flagged as an ESL. It will adhere to the %1 load "
                        "order but the records will be loaded in ESL space.")
                         .arg(type);
  }

  if (plugin->isOverlayFlagged()) {
    toolTip +=
        "<br><br>" + tr("This plugin is flagged as an overlay plugin. It contains only "
                        "modified records and will overlay those changes onto the "
                        "existing records in memory. It takes no memory space.");
  }

  if (plugin->hasNoRecords()) {
    toolTip += "<br><br>" + tr("This is a dummy plugin. It contains no records and is "
                               "typically used to load a paired archive file.");
  }

  if (plugin->forceDisabled()) {
    toolTip += "<br><br>" + tr("This game does not currently permit custom plugin "
                               "loading. There may be manual workarounds.");
  }

  if (lootInfo) {
    toolTip += makeLootTooltip(*lootInfo);
  }

  return toolTip;
}

QVariant PluginListModel::conflictData(const QModelIndex& index) const
{
  const bool overriding = !data(index, OverridingRole).toList().empty();
  const bool overridden = !data(index, OverriddenRole).toList().empty();

  QVariantList result;

  if (overriding && overridden) {
    result.append(":/MO/gui/emblem_conflict_mixed");
  } else if (overriding) {
    result.append(":/MO/gui/emblem_conflict_overwrite");
  } else if (overridden) {
    result.append(":/MO/gui/emblem_conflict_overwritten");
  }

  return result;
}

static bool isProblematic(const TESData::FileInfo* plugin,
                          const MOTools::Loot::Plugin* lootInfo)
{
  if (plugin && plugin->hasMissingMasters()) {
    return true;
  }

  if (lootInfo) {
    if (!lootInfo->incompatibilities.empty()) {
      return true;
    }

    if (!lootInfo->missingMasters.empty()) {
      return true;
    }
  }

  return false;
}

QVariant PluginListModel::iconData(const QModelIndex& index) const
{
  const int id        = index.row();
  const auto plugin   = m_Plugins->getPlugin(id);
  const auto lootInfo = m_Plugins->getLootReport(plugin->name());

  QVariantList result;

  if (isProblematic(plugin, lootInfo)) {
    result.append(":/MO/gui/warning");
  }

  if (plugin->hasIni()) {
    result.append(":/MO/gui/attachment");
  }

  if (!plugin->archives().empty()) {
    result.append(":/MO/gui/archive_conflict_neutral");
  }

  if (plugin->isLightFlagged() || plugin->hasLightExtension()) {
    result.append(":/MO/gui/awaiting");
  }

  return result;
}

QVariant PluginListModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
  if (orientation == Qt::Horizontal) {
    if (role == Qt::DisplayRole) {
      switch (section) {
      case COL_NAME:
        return tr("Name");
      case COL_CONFLICTS:
        return tr("Conflicts");
      case COL_FLAGS:
        return tr("Flags");
      case COL_PRIORITY:
        return tr("Priority");
      case COL_MODINDEX:
        return tr("Mod Index");
      default:
        return tr("unknown");
      }
    }
  }
  return QAbstractItemModel::headerData(section, orientation, role);
}

int PluginListModel::rowCount([[maybe_unused]] const QModelIndex& parent) const
{
  return m_Plugins->pluginCount();
}

int PluginListModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return COL_COUNT;
}

bool PluginListModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role == Qt::CheckStateRole) {
    const int id = index.row();
    m_Plugins->setEnabled(id, value.toInt() == Qt::Checked);
    emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, COL_MODINDEX),
                     {Qt::DisplayRole, Qt::CheckStateRole});
    emit pluginStatesChanged({index});
    return true;
  }

  return false;
}

Qt::DropActions PluginListModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

bool PluginListModel::canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                      int row, int column,
                                      const QModelIndex& parent) const
{
  if (action == Qt::IgnoreAction) {
    return true;
  }

  if (action != Qt::MoveAction) {
    return false;
  }

  PluginListDropInfo dropInfo{data, row, parent, m_Plugins};
  return m_Plugins->canMoveToPriority(dropInfo.sourceRows(), dropInfo.destination());
}

bool PluginListModel::dropMimeData(const QMimeData* data, Qt::DropAction action,
                                   int row, int column, const QModelIndex& parent)
{
  if (action == Qt::IgnoreAction) {
    return true;
  }

  if (action != Qt::MoveAction) {
    return false;
  }

  PluginListDropInfo dropInfo{data, row, parent, m_Plugins};
  m_Plugins->moveToPriority(dropInfo.sourceRows(), dropInfo.destination());
  emit dataChanged(index(0, COL_PRIORITY), index(rowCount() - 1, COL_MODINDEX),
                   {Qt::DisplayRole});
  emit pluginOrderChanged();

  return true;
}

void PluginListModel::refresh()
{
  emit beginResetModel();
  m_Plugins->refresh();
  emit endResetModel();
}

void PluginListModel::invalidate()
{
  emit beginResetModel();
  m_Plugins->refresh(true);
  emit endResetModel();
}

void PluginListModel::movePlugin(const QString& name, int oldPriority, int newPriority)
{
  m_Plugins->setPriority(name, newPriority);
  emit dataChanged(index(0, COL_PRIORITY), index(rowCount() - 1, COL_MODINDEX),
                   {Qt::DisplayRole});
  emit pluginOrderChanged();
}

void PluginListModel::changePluginStates(
    const std::map<QString, MOBase::IPluginList::PluginStates>& infos)
{
  QModelIndexList indices;
  for (auto& [name, state] : infos) {
    m_Plugins->setState(name, state);

    const auto idx = m_Plugins->getIndex(name);
    if (idx != -1) {
      indices.append(index(idx, 0));
    }
  }

  emit dataChanged(index(0, 0), index(rowCount() - 1, COL_MODINDEX),
                   {Qt::DisplayRole, Qt::CheckStateRole});
  emit pluginStatesChanged(indices);
}

void PluginListModel::setEnabledAll(bool enabled)
{
  QModelIndexList indices;
  indices.reserve(rowCount());
  std::generate_n(std::back_inserter(indices), rowCount(), [this, i = 0]() mutable {
    return index(i++, 0);
  });
  setEnabled(indices, enabled);
}

void PluginListModel::setEnabled(const QModelIndexList& indices, bool enabled)
{
  std::vector<int> ids;
  ids.reserve(indices.size());
  std::ranges::transform(indices, std::back_inserter(ids), [](auto&& idx) {
    return idx.row();
  });
  m_Plugins->setEnabled(std::move(ids), enabled);
  emit pluginStatesChanged(indices);
}

void PluginListModel::sendToPriority(const QModelIndexList& indices, int priority)
{
  std::vector<int> ids;
  ids.reserve(indices.size());
  std::ranges::transform(indices, std::back_inserter(ids), [](auto&& idx) {
    return idx.row();
  });
  m_Plugins->moveToPriority(std::move(ids), priority);
  emit dataChanged(index(0, COL_PRIORITY), index(rowCount() - 1, COL_MODINDEX),
                   {Qt::DisplayRole});
  emit pluginOrderChanged();
}

void PluginListModel::shiftPluginsPriority(const QModelIndexList& indices, int offset)
{
  std::vector<int> ids;
  ids.reserve(indices.size());
  std::ranges::transform(indices, std::back_inserter(ids), [](auto&& idx) {
    return idx.row();
  });
  m_Plugins->shiftPriority(std::move(ids), offset);
  emit dataChanged(index(0, COL_PRIORITY), index(rowCount() - 1, COL_MODINDEX),
                   {Qt::DisplayRole});
  emit pluginOrderChanged();
}

void PluginListModel::toggleState(const QModelIndexList& indices)
{
  std::vector<int> ids;
  ids.reserve(indices.size());
  std::ranges::transform(indices, std::back_inserter(ids), [](auto&& idx) {
    return idx.row();
  });
  m_Plugins->toggleState(std::move(ids));
  emit pluginStatesChanged(indices);
}

}  // namespace BSPluginList
