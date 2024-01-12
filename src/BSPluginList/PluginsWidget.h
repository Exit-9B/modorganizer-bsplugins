#ifndef BSPLUGINLIST_PLUGINSWIDGET_H
#define BSPLUGINLIST_PLUGINSWIDGET_H

#include "MOPlugin/IPanelInterface.h"
#include "PluginGroupProxyModel.h"
#include "PluginListModel.h"
#include "PluginSortFilterProxyModel.h"
#include "TESData/PluginList.h"

#include <QSortFilterProxyModel>
#include <QWidget>

class Ui_PluginsWidget;

namespace BSPluginList
{

class PluginsWidget final : public QWidget
{
  Q_OBJECT

public:
  PluginsWidget(MOBase::IOrganizer* organizer, IPanelInterface* panelInterface,
                QWidget* parent = nullptr);

  PluginsWidget(const PluginsWidget&) = delete;
  PluginsWidget(PluginsWidget&&)      = delete;

  ~PluginsWidget() noexcept;

  PluginsWidget& operator=(const PluginsWidget&) = delete;
  PluginsWidget& operator=(PluginsWidget&&)      = delete;

  [[nodiscard]] TESData::PluginList* getPluginList() { return m_PluginList; }

public slots:
  void updatePluginCount();

protected:
  void changeEvent(QEvent* e) override;

private slots:
  void onSelectionChanged(const QItemSelection& selected,
                          const QItemSelection& deselected);

  void onPanelActivated();

  void onSelectedOriginsChanged(const QList<QString>& origins);

  void toggleHideForceEnabled();

  void on_pluginList_customContextMenuRequested(const QPoint& pos);
  void on_pluginList_doubleClicked(const QModelIndex& index);
  void on_pluginList_openOriginExplorer(const QModelIndex& index);
  void on_espFilterEdit_textChanged(const QString& filter);
  void on_sortButton_clicked();
  void on_restoreButton_clicked();
  void on_saveButton_clicked();

private:
  [[nodiscard]] QMenu* listOptionsMenu();
  void restoreState();

  // HACK: Attempt to keep our custom plugin list synchronized with the built-in panel
  void synchronizePluginLists(MOBase::IOrganizer* organizer);

  Ui_PluginsWidget* ui;

  QMenu* optionsMenu          = nullptr;
  QAction* toggleForceEnabled = nullptr;

  TESData::PluginList* m_PluginList       = nullptr;
  PluginListModel* m_PluginListModel      = nullptr;
  PluginSortFilterProxyModel* m_SortProxy = nullptr;
  PluginGroupProxyModel* m_GroupProxy     = nullptr;
  IPanelInterface* m_PanelInterface;

  MOBase::IOrganizer* m_Organizer;

  bool m_DidUpdateMasterList = false;
};

}  // namespace BSPluginList

#endif  // TESDATA_PLUGINSWIDGET_H
