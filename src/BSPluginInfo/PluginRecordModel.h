#ifndef BSPLUGININFO_PLUGINRECORDMODEL_H
#define BSPLUGININFO_PLUGINRECORDMODEL_H

#include "TESData/PluginList.h"

#include <imoinfo.h>

#include <QAbstractItemModel>

namespace BSPluginInfo
{

class PluginRecordModel final : public QAbstractItemModel
{
  Q_OBJECT

public:
  enum EColumn
  {
    COL_ID,
    COL_OWNER,
    COL_NAME,

    COL_COUNT
  };

  PluginRecordModel(MOBase::IOrganizer* organizer, TESData::PluginList* pluginList,
                    const QString& pluginName);

  [[nodiscard]] TESData::RecordPath getPath(const QModelIndex& index) const;

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  bool canFetchMore(const QModelIndex& parent) const override;
  void fetchMore(const QModelIndex& parent) override;

  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

private:
  using Item = TESData::FileEntry::TreeItem;

  static QString makeGroupName(TESFile::GroupData group);

  QString m_PluginName;
  MOBase::IOrganizer* m_Organizer   = nullptr;
  TESData::PluginList* m_PluginList = nullptr;
  TESData::FileEntry* m_FileEntry   = nullptr;
  Item* m_DataRoot                  = nullptr;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGINRECORDMODEL_H
