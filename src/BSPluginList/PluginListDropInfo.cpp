#include "PluginListDropInfo.h"

#include <QMap>
#include <QMimeData>
#include <QModelIndex>
#include <QVariant>

namespace BSPluginList
{

PluginListDropInfo::PluginListDropInfo(const QMimeData* data, int insertId,
                                       const QModelIndex& parent,
                                       const TESData::PluginList* plugins)
{
  QByteArray encoded = data->data("application/x-qabstractitemmodeldatalist");
  QDataStream stream(&encoded, QIODevice::ReadOnly);

  while (!stream.atEnd()) {
    int sourceRow;
    int col;
    QMap<int, QVariant> roleDataMap;
    stream >> sourceRow >> col >> roleDataMap;
    if (col == 0) {
      m_SourceRows.push_back(sourceRow);
    }
  }

  if (insertId == -1) {
    insertId = parent.row();
  }

  if (plugins) {
    if (insertId < 0 || insertId >= plugins->pluginCount()) {
      m_Destination = plugins->pluginCount();
    } else {
      const auto plugin = plugins->getPlugin(insertId);
      m_Destination     = plugin ? plugin->priority() : plugins->pluginCount();
    }
  }
}

}  // namespace BSPluginList
