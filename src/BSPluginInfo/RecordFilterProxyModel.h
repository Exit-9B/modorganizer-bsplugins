#ifndef BSPLUGININFO_RECORDFILTERPROXYMODEL_H
#define BSPLUGININFO_RECORDFILTERPROXYMODEL_H

#include "TESData/PluginList.h"

#include <QSortFilterProxyModel>

namespace BSPluginInfo
{

class RecordFilterProxyModel final : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  enum FilterFlags
  {
    Filter_WinningConflicts = 0x1,
    Filter_LosingConflicts  = 0x2,
    Filter_NoConflicts      = 0x4,

    Filter_AllConflicts = Filter_WinningConflicts | Filter_LosingConflicts,
    Filter_AllRecords   = Filter_AllConflicts | Filter_NoConflicts,
  };

  RecordFilterProxyModel(const TESData::PluginList* pluginList,
                         const QString& pluginName);

  [[nodiscard]] FilterFlags filterFlags() const { return m_FilterFlags; }
  void setFilterFlags(FilterFlags filterFlags);

  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;

private:
  const TESData::PluginList* m_PluginList;
  const QString m_PluginName;
  FilterFlags m_FilterFlags;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_RECORDFILTERPROXYMODEL_H
