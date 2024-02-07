#ifndef BSPLUGININFO_AUXCONFLICTMODEL_H
#define BSPLUGININFO_AUXCONFLICTMODEL_H

#include "TESData/PluginList.h"

#include <QAbstractItemModel>

namespace BSPluginInfo
{

class AuxConflictModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  struct Item
  {
    QString name;
    QList<TESData::TESFileHandle> sortedHandles;
  };

  explicit AuxConflictModel(const TESData::PluginList* pluginList,
                            QObject* parent = nullptr);

  void appendItem(const QString& name, QList<TESData::TESFileHandle>&& handles);

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

protected:
  const TESData::PluginList* m_PluginList;
  QList<Item> m_Items;
};

class AuxWinnerModel final : public AuxConflictModel
{
  Q_OBJECT

public:
  enum EColumn
  {
    COL_NAME,
    COL_LOSERS,

    COL_COUNT
  };

  using AuxConflictModel::AuxConflictModel;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
};

class AuxLoserModel final : public AuxConflictModel
{
  Q_OBJECT

public:
  enum EColumn
  {
    COL_NAME,
    COL_WINNER,

    COL_COUNT
  };

  using AuxConflictModel::AuxConflictModel;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
};

class AuxNonConflictModel final : public AuxConflictModel
{
  Q_OBJECT

public:
  enum EColumn
  {
    COL_NAME,

    COL_COUNT
  };

  using AuxConflictModel::AuxConflictModel;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_AUXCONFLICTMODEL_H
