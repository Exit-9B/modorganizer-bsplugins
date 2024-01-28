#ifndef BSPLUGININFO_PLUGINRECORDMODEL_H
#define BSPLUGININFO_PLUGINRECORDMODEL_H

#include "TESData/PluginList.h"

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

  PluginRecordModel(TESData::PluginList* pluginList, const std::string& pluginName);

  [[nodiscard]] TESData::RecordPath getPath(const QModelIndex& index) const;

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

private:
  using Item = TESData::FileEntry::TreeItem;

  static QString makeGroupName(TESFile::GroupData group);

  std::string m_PluginName;
  TESData::PluginList* m_PluginList = nullptr;
  TESData::FileEntry* m_FileEntry   = nullptr;
  Item* m_DataRoot                  = nullptr;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_PLUGINRECORDMODEL_H
