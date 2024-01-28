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
  void openPluginInformation(const QModelIndex& index);

private:
  void addAllItemsMenu();
  void addSelectedFilesActions();
  void addSelectedGroupActions();
  void addSelectionActions();
  void addSendToMenu();
  void addOriginActions(MOBase::IModList* modList, MOBase::IPluginList* pluginList);

  void sendSelectedToGroup();

  QModelIndex m_Index;
  PluginListModel* m_Model;
  PluginListView* m_View;
  QModelIndexList m_ViewSelected;
  QModelIndexList m_ModelSelected;
  bool m_FilesSelected  = false;
  bool m_GroupsSelected = false;
  bool m_FilesTogglable = false;
  bool m_FilesESM = false;
  bool m_FilesESP = false;
  bool m_FilesMovable = false;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINLISTCONTEXTMENU_H
