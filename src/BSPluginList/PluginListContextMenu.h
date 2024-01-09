#ifndef BSPLUGINLIST_PLUGINLISTCONTEXTMENU_H
#define BSPLUGINLIST_PLUGINLISTCONTEXTMENU_H

#include <imodlist.h>
#include <ipluginlist.h>

#include <QMenu>
#include <QModelIndex>

namespace BSPluginList
{

class PluginListModel;
class PluginListView;

class PluginListContextMenu final : public QMenu
{
  Q_OBJECT

public:
  PluginListContextMenu(const QModelIndex& index, PluginListModel* model,
                        PluginListView* view, MOBase::IModList* modList,
                        MOBase::IPluginList* pluginList);

signals:
  void openModInformation(const QModelIndex& index);

private:
  QModelIndex m_Index;
  PluginListModel* m_Model;
  PluginListView* m_View;
  QModelIndexList m_Selected;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINLISTCONTEXTMENU_H
