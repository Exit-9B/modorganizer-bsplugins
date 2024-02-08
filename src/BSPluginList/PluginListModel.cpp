#include "PluginListModel.h"
#include "MOPlugin/Settings.h"
#include "PluginListDropInfo.h"

#include <QGuiApplication>
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

QModelIndex PluginListModel::parent([[maybe_unused]] const QModelIndex& index) const
{
  return QModelIndex();
}

Qt::ItemFlags PluginListModel::flags(const QModelIndex& index) const
{
  const int id = index.row();

  Qt::ItemFlags result = QAbstractItemModel::flags(index);

  if (index.isValid()) {
    const auto plugin = m_Plugins->getPlugin(id);
    if (plugin && (!plugin->forceLoaded() && !plugin->forceDisabled())) {
      if (index.column() == COL_PRIORITY)
        result |= Qt::ItemIsEditable;
      result |= Qt::ItemIsUserCheckable;
    }
    result |= Qt::ItemIsDragEnabled;
    result &= ~Qt::ItemIsDropEnabled;
  } else {
    result |= Qt::ItemIsDropEnabled;
  }

  return result;
}

static QVariantList
conflictListData(const TESData::PluginList* pluginList, const TESData::FileInfo* plugin,
                 const QSet<int>& (TESData::FileInfo::*getConflicts)() const)
{
  if (!plugin || !plugin->enabled()) {
    return QVariantList();
  }

  QVariantList list;
  for (const int otherId : (plugin->*getConflicts)()) {
    const auto other = pluginList->getPlugin(otherId);
    if (other && other->enabled()) {
      list.append(otherId);
    }
  }
  return list;
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
  case GroupingRole: {
    const auto id     = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    return plugin ? plugin->group() : QVariant();
  }
  case IndexRole:
    return index.row();
  case InfoRole: {
    const auto id     = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    return QVariant::fromValue(plugin);
  }
  case ConflictsIconRole:
    return conflictData(index);
  case FlagsIconRole:
    return iconData(index);
  case OriginRole: {
    const int id = index.row();
    return m_Plugins->getOriginName(id);
  }
  case OverridingRole: {
    const int id      = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    return conflictListData(m_Plugins, plugin, &TESData::FileInfo::getPluginOverriding);
  }
  case OverriddenRole: {
    const int id      = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    return conflictListData(m_Plugins, plugin, &TESData::FileInfo::getPluginOverridden);
  }
  case OverwritingAuxRole: {
    const int id      = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    return conflictListData(m_Plugins, plugin,
                            &TESData::FileInfo::getPluginOverwritingArchive);
  }
  case OverwrittenAuxRole: {
    const int id      = index.row();
    const auto plugin = m_Plugins->getPlugin(id);
    return conflictListData(m_Plugins, plugin,
                            &TESData::FileInfo::getPluginOverwrittenArchive);
  }
  }
  return QVariant();
}

QVariant PluginListModel::displayData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);

  if (!plugin) {
    return QVariant();
  }

  switch (index.column()) {
  case COL_NAME:
    return plugin->name();
  case COL_PRIORITY:
    return plugin->priority();
  case COL_MODINDEX:
    return plugin->index();
  default:
    return QVariant();
  }
}

QVariant PluginListModel::checkstateData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);

  if (!plugin) {
    return QVariant();
  }

  if (plugin->isAlwaysEnabled()) {
    // HACK: PluginListStyledItemDelegate draws the checkbox separately
    return QVariant();
  } else if (plugin->forceDisabled()) {
    return QVariant();
  } else {
    return plugin->enabled() ? Qt::Checked : Qt::Unchecked;
  }
}

QVariant PluginListModel::foregroundData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);

  if (!plugin) {
    return QVariant();
  }

  if (plugin->hasNoRecords()) {
    if (index.column() == COL_NAME) {
      return QBrush(Qt::gray);
    }
  }

  if (plugin->forceDisabled()) {
    if (index.column() == COL_NAME) {
      return QBrush(Qt::darkRed);
    }
  }

  return QVariant();
}

QVariant
PluginListModel::backgroundData([[maybe_unused]] const QModelIndex& index) const
{
  return QVariant();
}

QVariant PluginListModel::fontData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);

  QFont result;

  if (index.column() == COL_NAME) {
    if (plugin && plugin->hasNoRecords()) {
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
  if (Settings::instance()->lootShowProblems()) {
    for (const auto& f : lootInfo.incompatibilities) {
      s += "<li>" +
           QObject::tr("Incompatible with %1")
               .arg(!f.displayName.isEmpty() ? f.displayName : f.name) +
           "</li>";
    }
  }

  if (Settings::instance()->lootShowMessages()) {
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
  }

  if (Settings::instance()->lootShowDirty()) {
    for (const auto& d : lootInfo.dirty) {
      s += "<li>" + d.toString(false) + "</li>";
    }

    for (const auto& c : lootInfo.clean) {
      s += "<li>" + c.toString(true) + "</li>";
    }
  }

  if (s.isEmpty()) {
    return s;
  }

  return "<ul style=\"margin-top:0px; padding-top:0px; margin-left:15px; "
         "-qt-list-indent: 0;\">" +
         s + "</ul>";
}

QVariant PluginListModel::tooltipData(const QModelIndex& index) const
{
  const int id        = index.row();
  const auto plugin   = m_Plugins->getPlugin(id);
  const auto lootInfo = m_Plugins->getLootReport(plugin->name());

  if (!plugin) {
    return QVariant();
  }

  switch (index.column()) {
  case COL_NAME: {
    QString toolTip;

    toolTip += "<b>" + tr("Origin") + "</b>: " + m_Plugins->getOriginName(id);

    if (plugin->forceLoaded()) {
      toolTip += "<br><b><i>" +
                 tr("This plugin can't be disabled or moved (enforced by the game).") +
                 "</i></b>";
    } else if (plugin->forceEnabled()) {
      toolTip += "<br><b><i>" +
                 tr("This plugin can't be disabled (enforced by the game).") +
                 "</i></b>";
    }

    if (!plugin->author().isEmpty()) {
      toolTip += "<br><b>" + tr("Author") + "</b>: " + truncateString(plugin->author());
    }

    if (plugin->description().size() > 0) {
      toolTip += "<br><b>" + tr("Description") +
                 "</b>: " + truncateString(plugin->description());
    }

    if (plugin->enabled() && plugin->missingMasters().size() > 0) {
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
      QString archiveString =
          plugin->archives().size() < 6
              ? truncateString(
                    QStringList(plugin->archives().begin(), plugin->archives().end())
                        .join(", "))
              : "";
      toolTip += "<br><b>" + tr("Loads Archives") + "</b>: " + archiveString;
    }

    if (plugin->hasIni()) {
      toolTip += "<br><b>" + tr("Loads INI settings") +
                 "</b>: " + QFileInfo(plugin->name()).baseName() + ".ini";
    }

    if (plugin->hasNoRecords()) {
      toolTip +=
          "<br><br>" + tr("This is a dummy plugin. It contains no records and is "
                          "typically used to load a paired archive file.");
    }

    return toolTip;
  }
  case COL_CONFLICTS: {
    const bool overriding = !data(index, OverridingRole).toList().empty();
    const bool overridden = !data(index, OverriddenRole).toList().empty();

    if (overriding && overridden) {
      return tr("Overrides & has overridden records");
    } else if (overriding) {
      return tr("Overrides records");
    } else if (overridden) {
      return tr("Has overridden records");
    }
    return QVariant();
  }
  case COL_FLAGS: {
    // HACK: insert some HTML to enable multiline tooltips
    QString toolTip       = "<nobr/>";
    const QString spacing = "<br><br>";

    if (plugin->enabled() && plugin->missingMasters().size() > 0) {
      toolTip += "<b>" + tr("Missing Masters") + "</b>: " + "<b>" +
                 truncateString(QStringList(plugin->missingMasters().begin(),
                                            plugin->missingMasters().end())
                                    .join(", ")) +
                 "</b>" + spacing;
    }

    if (plugin->hasIni()) {
      toolTip +=
          tr("There is an ini file connected to this plugin. Its settings will "
             "be added to your game settings, overwriting in case of conflicts.") +
          "<br><br>";
    }

    if (!plugin->archives().empty()) {
      toolTip +=
          tr("There are Archives connected to this plugin. Their assets will be "
             "added to your game, overwriting in case of conflicts following the "
             "plugin order. Loose files will always overwrite assets from "
             "Archives.") +
          spacing;
    }

    if (plugin->isSmallFile()) {
      QString type = plugin->isMasterFile() ? "ESM" : "ESP";
      toolTip += tr("This %1 is flagged as an ESL. It will adhere to the %1 load "
                    "order but the records will be loaded in ESL space.")
                     .arg(type) +
                 spacing;
    }

    if (plugin->isOverlayFlagged()) {
      toolTip += tr("This plugin is flagged as an overlay plugin. It contains only "
                    "modified records and will overlay those changes onto the "
                    "existing records in memory. It takes no memory space.") +
                 spacing;
    }

    if (plugin->forceDisabled()) {
      toolTip += tr("This game does not currently permit custom plugin "
                    "loading. There may be manual workarounds.");
    }

    if (toolTip.endsWith(spacing)) {
      toolTip.chop(spacing.length());
    }

    if (lootInfo) {
      const auto lootToolTip = makeLootTooltip(*lootInfo);
      if (toolTip.length() > 7 && !lootToolTip.isEmpty()) {
        toolTip += "<hr>";
      }
      toolTip += lootToolTip;
    }

    return toolTip;
  }
  default:
    return QVariant();
  }
}

QVariant PluginListModel::conflictData(const QModelIndex& index) const
{
  const int id      = index.row();
  const auto plugin = m_Plugins->getPlugin(id);
  return plugin->conflictState();
}

static bool isProblematic(const TESData::FileInfo* plugin,
                          const MOTools::Loot::Plugin* lootInfo)
{
  if (plugin && plugin->enabled() && plugin->hasMissingMasters()) {
    return true;
  }

  if (lootInfo && Settings::instance()->lootShowProblems()) {
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

  if (!plugin) {
    return QVariant();
  }

  using enum TESData::FileInfo::EFlag;
  uint flag = 0;

  if (isProblematic(plugin, lootInfo)) {
    flag |= FLAG_PROBLEMATIC;
  }

  if (lootInfo && !lootInfo->messages.empty() &&
      Settings::instance()->lootShowMessages()) {
    flag |= FLAG_INFORMATION;
  }

  if (plugin->hasIni()) {
    flag |= FLAG_INI;
  }

  if (!plugin->archives().empty()) {
    flag |= FLAG_BSA;
  }

  if (plugin->isSmallFile()) {
    flag |= FLAG_LIGHT;
  }

  if (plugin->isOverlayFlagged()) {
    flag |= FLAG_OVERLAY;
  }

  if (lootInfo && !lootInfo->dirty.empty() && Settings::instance()->lootShowDirty()) {
    flag |= FLAG_CLEAN;
  }

  return flag;
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
                     {Qt::EditRole, Qt::CheckStateRole});
    emit pluginStatesChanged({index});
    return true;
  } else if (role == Qt::EditRole) {
    if (index.column() == COL_PRIORITY) {
      bool ok;
      const int newPriority = value.toInt(&ok);
      if (ok) {
        int destination = newPriority;
        if (newPriority > index.data(Qt::EditRole).toInt()) {
          ++destination;
        }
        m_Plugins->moveToPriority({index.row()}, destination);
        emit dataChanged(this->index(0, 0),
                         this->index(rowCount() - 1, columnCount() - 1),
                         {Qt::EditRole, GroupingRole});
        emit pluginOrderChanged();
        return true;
      }
    }
  }

  return false;
}

Qt::DropActions PluginListModel::supportedDropActions() const
{
  return Qt::MoveAction;
}

bool PluginListModel::canDropMimeData(const QMimeData* data, Qt::DropAction action,
                                      int row, [[maybe_unused]] int column,
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
                                   int row, [[maybe_unused]] int column,
                                   const QModelIndex& parent)
{
  if (action == Qt::IgnoreAction) {
    return true;
  }

  if (action != Qt::MoveAction) {
    return false;
  }

  PluginListDropInfo dropInfo{data, row, parent, m_Plugins};
  m_Plugins->moveToPriority(dropInfo.sourceRows(), dropInfo.destination());
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1),
                   {Qt::DisplayRole, GroupingRole});
  emit pluginOrderChanged();

  return true;
}

QStringList
PluginListModel::groups(std::function<bool(const TESData::FileInfo*)> pred) const
{
  boost::container::flat_set<QString> groupSet;
  QStringList groups;
  QString lastGroup;

  for (int priority = 0, count = m_Plugins->pluginCount(); priority < count;
       ++priority) {
    const auto plugin = m_Plugins->getPluginByPriority(priority);

    if (pred && !pred(plugin)) {
      continue;
    }

    const auto& group = plugin ? plugin->group() : QString();
    if (group.isEmpty() || group == lastGroup) {
      continue;
    }

    auto [it, inserted] = groupSet.insert(group);
    if (inserted) {
      groups.append(group);
    }
    lastGroup = group;
  }

  return groups;
}

QStringList PluginListModel::masterGroups() const
{
  return groups([](auto&& plugin) {
    return !plugin->forceLoaded() && plugin->isMasterFile();
  });
}

QStringList PluginListModel::regularGroups() const
{
  return groups([](auto&& plugin) {
    return !plugin->forceLoaded() && !plugin->isMasterFile();
  });
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

void PluginListModel::movePlugin(const QString& name, [[maybe_unused]] int oldPriority,
                                 int newPriority)
{
  m_Plugins->setPriority(name, newPriority);
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1),
                   {Qt::DisplayRole, GroupingRole});
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
  if (indices.empty()) {
    return;
  }

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
  if (indices.empty()) {
    return;
  }

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
  if (indices.empty()) {
    return;
  }

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
  if (indices.empty()) {
    return;
  }

  std::vector<int> ids;
  ids.reserve(indices.size());
  std::ranges::transform(indices, std::back_inserter(ids), [](auto&& idx) {
    return idx.row();
  });
  m_Plugins->toggleState(std::move(ids));
  emit pluginStatesChanged(indices);
}

void PluginListModel::setGroup(const QModelIndexList& indices, const QString& group)
{
  if (indices.empty()) {
    return;
  }

  std::vector<int> ids;
  ids.reserve(indices.size());
  std::ranges::transform(indices, std::back_inserter(ids), [](auto&& idx) {
    return idx.row();
  });
  m_Plugins->setGroup(std::move(ids), group);
  emit dataChanged(this->index(0, 0), this->index(rowCount() - 1, COL_MODINDEX),
                   {GroupingRole});
}

void PluginListModel::sendToGroup(const QModelIndexList& indices, const QString& group,
                                  bool isESM)
{
  int destination = -1;
  for (int priority = 0, count = m_Plugins->pluginCount(); priority < count;
       ++priority) {
    const auto plugin = m_Plugins->getPluginByPriority(priority);
    if (plugin && plugin->isMasterFile() == isESM && plugin->group() == group) {
      destination = priority + 1;
    }
  }

  if (destination == -1)
    return;

  std::vector<int> ids;
  ids.reserve(indices.size());
  std::ranges::transform(indices, std::back_inserter(ids), [](auto&& idx) {
    return idx.row();
  });
  m_Plugins->setGroup(ids, group);
  m_Plugins->moveToPriority(std::move(ids), destination);
  emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1),
                   {Qt::DisplayRole, GroupingRole});
  emit pluginOrderChanged();
}

}  // namespace BSPluginList
