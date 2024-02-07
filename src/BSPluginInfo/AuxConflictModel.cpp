#include "AuxConflictModel.h"

#include <utility>

namespace BSPluginInfo
{

AuxConflictModel::AuxConflictModel(const TESData::PluginList* pluginList,
                                   QObject* parent)
    : QAbstractItemModel(parent), m_PluginList{pluginList}
{}

void AuxConflictModel::appendItem(const QString& name,
                                  QList<TESData::TESFileHandle>&& handles)
{
  m_Items.append({name, std::move(handles)});
}

QModelIndex AuxConflictModel::index(int row, int column,
                                    const QModelIndex& parent) const
{
  if (parent.isValid()) {
    return QModelIndex();
  }

  return createIndex(row, column, row);
}

QModelIndex AuxConflictModel::parent([[maybe_unused]] const QModelIndex& index) const
{
  return QModelIndex();
}

int AuxConflictModel::rowCount(const QModelIndex& parent) const
{
  return !parent.isValid() ? m_Items.length() : 0;
}

int AuxWinnerModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return COL_COUNT;
}

int AuxLoserModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return COL_COUNT;
}

int AuxNonConflictModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return COL_COUNT;
}

QVariant AuxWinnerModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case COL_NAME:
      return m_Items[index.row()].name;
    case COL_LOSERS: {
      QStringList names;
      const auto& handles = m_Items[index.row()].sortedHandles;
      for (int i = 0, count = handles.length() - 1; i < count; ++i) {
        const auto handle = handles[i];
        const auto entry  = m_PluginList->findEntryByHandle(handle);
        if (entry) {
          names.append(QString::fromStdString(entry->name()));
        }
      }
      return names.join(QStringLiteral(", "));
    }
    }
  }
  return QVariant();
}

QVariant AuxLoserModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case COL_NAME:
      return m_Items[index.row()].name;
    case COL_WINNER: {
      const auto& handles = m_Items[index.row()].sortedHandles;
      const auto entry    = m_PluginList->findEntryByHandle(handles.last());
      return entry ? QString::fromStdString(entry->name()) : QString();
    }
    }
  }
  return QVariant();
}

QVariant AuxNonConflictModel::data(const QModelIndex& index, int role) const
{
  if (role == Qt::DisplayRole) {
    switch (index.column()) {
    case COL_NAME:
      return m_Items[index.row()].name;
    }
  }
  return QVariant();
}

QVariant AuxWinnerModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  switch (section) {
  case COL_NAME:
    return tr("File");
  case COL_LOSERS:
    return tr("Overwritten Mods");
  default:
    return QVariant();
  }
}

QVariant AuxLoserModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  switch (section) {
  case COL_NAME:
    return tr("File");
  case COL_WINNER:
    return tr("Providing Mod");
  default:
    return QVariant();
  }
}

QVariant AuxNonConflictModel::headerData(int section, Qt::Orientation orientation,
                                         int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  switch (section) {
  case COL_NAME:
    return tr("File");
  default:
    return QVariant();
  }
}

}  // namespace BSPluginInfo
