#ifndef BSPLUGININFO_AUXTREEMODEL_H
#define BSPLUGININFO_AUXTREEMODEL_H

#include "TESData/AssociatedEntry.h"

#include <QAbstractItemModel>

namespace BSPluginInfo
{

class AuxTreeModel final : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit AuxTreeModel(const TESData::AssociatedEntry* entry,
                        QObject* parent = nullptr);

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

private:
  using Item = TESData::AuxItem;

  const TESData::AssociatedEntry* m_Entry;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_AUXTREEMODEL_H
