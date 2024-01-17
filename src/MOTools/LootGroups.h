#ifndef MOTOOLS_LOOTGROUPS_H
#define MOTOOLS_LOOTGROUPS_H

#include "TESData/PluginList.h"

#include <QString>

namespace MOTools
{

void importLootGroups(const TESData::PluginList* pluginList,
                      const QString& plugingroups, const QString& masterlist,
                      const QString& userlist);

}

#endif  // MOTOOLS_LOOTGROUPS_H
