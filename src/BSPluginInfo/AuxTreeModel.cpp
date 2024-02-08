#include "AuxTreeModel.h"

namespace BSPluginInfo
{

AuxTreeModel::AuxTreeModel(const TESData::AssociatedEntry* entry, QObject* parent)
    : QAbstractItemModel(parent), m_Entry{entry}
{}

QModelIndex AuxTreeModel::index(int row, int column, const QModelIndex& parent) const
{
  if (row < 0 || column != 0) {
    return QModelIndex();
  }

  const auto parentItem = parent.isValid()
                              ? static_cast<const Item*>(parent.internalPointer())
                              : m_Entry->root().get();

  const auto item = parentItem->getByIndex(row).get();
  return item ? createIndex(row, column, item) : QModelIndex();
}

QModelIndex AuxTreeModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  const auto item   = static_cast<const Item*>(index.internalPointer());
  const auto parent = item ? item->parent() : nullptr;
  if (!parent || !parent->parent()) {
    return QModelIndex();
  }

  const int row = parent->parent()->indexOf(parent);

  return createIndex(row, 0, parent);
}

int AuxTreeModel::rowCount(const QModelIndex& parent) const
{
  const auto parentItem = parent.isValid()
                              ? static_cast<const Item*>(parent.internalPointer())
                              : m_Entry->root().get();

  return parentItem ? parentItem->numChildren() : 0;
}

int AuxTreeModel::columnCount([[maybe_unused]] const QModelIndex& parent) const
{
  return 1;
}

QVariant AuxTreeModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) {
    return QVariant();
  }

  const auto item = static_cast<const Item*>(index.internalPointer());
  switch (role) {
  case Qt::DisplayRole:
    return QString::fromStdString(item->name());
  }

  return QVariant();
}

QVariant AuxTreeModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  if (section == 0) {
    return tr("File Name");
  } else {
    return QVariant();
  }
}

}  // namespace BSPluginInfo
