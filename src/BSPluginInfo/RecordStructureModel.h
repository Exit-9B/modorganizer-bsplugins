#ifndef BSPLUGININFO_RECORDSTRUCTUREMODEL_H
#define BSPLUGININFO_RECORDSTRUCTUREMODEL_H

#include "TESData/DataItem.h"
#include "TESData/PluginList.h"
#include "TESData/RecordPath.h"

#include <QAbstractItemModel>
#include <QList>

namespace BSPluginInfo
{

class RecordStructureModel final : public QAbstractItemModel
{
  Q_OBJECT

public:
  RecordStructureModel(TESData::PluginList* pluginList, TESData::Record* record,
                       const TESData::RecordPath& path, MOBase::IOrganizer* organizer);

  void refresh();

  [[nodiscard]] const QString& file(int index) const { return m_Files[index]; }

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

private:
  using Item = TESData::DataItem;

  void readFile(const TESData::RecordPath& path, const QString& filePath,
                int index = 0);

  QList<QString> m_Files;
  MOBase::IOrganizer* m_Organizer   = nullptr;
  TESData::PluginList* m_PluginList = nullptr;
  TESData::Record* m_Record         = nullptr;
  TESData::RecordPath m_Path;
  std::shared_ptr<Item> m_Root;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_RECORDSTRUCTUREMODEL_H
