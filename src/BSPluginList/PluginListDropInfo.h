#ifndef BSPLUGINLIST_PLUGINLISTDROPINFO_H
#define BSPLUGINLIST_PLUGINLISTDROPINFO_H

#include "TESData/PluginList.h"

#include <vector>

class QMimeData;

namespace BSPluginList
{

class PluginListDropInfo final
{
public:
  PluginListDropInfo(const QMimeData* data, int insertId, const QModelIndex& parent,
                     const TESData::PluginList* plugins);

  const std::vector<int>& sourceRows() const { return m_SourceRows; }
  const int destination() const { return m_Destination; }

private:
  std::vector<int> m_SourceRows;
  int m_Destination;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINLISTDROPINFO_H
