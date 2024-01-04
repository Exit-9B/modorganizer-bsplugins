#ifndef MOTOOLS_ILOOTCACHE_H
#define MOTOOLS_ILOOTCACHE_H

#include "Loot.h"

#include <QString>

namespace MOTools
{

class ILootCache
{
public:
  /**
   * @brief clear all additional information we stored on plugins
   */
  virtual void clearAdditionalInformation() = 0;

  /**
   * @brief adds information from a loot report
   * @param name name of the plugin to add information about
   * @param plugin loot report for the plugin
   */
  virtual void addLootReport(const QString& name, Loot::Plugin plugin) = 0;
};

}  // namespace MOTools

#endif  // MOTOOLS_ILOOTCACHE_H
