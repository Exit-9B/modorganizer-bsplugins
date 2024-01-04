#ifndef BSPLUGINLIST_PLUGINSWIDGET_H
#define BSPLUGINLIST_PLUGINSWIDGET_H

#include "MOPlugin/IPanelInterface.h"
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

  [[nodiscard]] TESData::PluginList* getPluginList() { return pluginList; }

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

  void on_espFilterEdit_textChanged(const QString& filter);
  void on_sortButton_clicked();
  void on_restoreButton_clicked();
  void on_saveButton_clicked();

private:
  [[nodiscard]] QMenu* listOptionsMenu();

  // HACK: Attempt to keep our custom plugin list synchronized with the built-in panel
  void synchronizePluginLists(MOBase::IOrganizer* organizer);

  Ui_PluginsWidget* ui;
  TESData::PluginList* pluginList        = nullptr;
  PluginListModel* pluginListModel       = nullptr;
  PluginSortFilterProxyModel* proxyModel = nullptr;
  IPanelInterface* m_PanelInterface;

  QMenu* optionsMenu          = nullptr;
  QAction* toggleForceEnabled = nullptr;

  MOBase::IOrganizer* m_Organizer;

  bool m_DidUpdateMasterList = false;
};

}  // namespace BSPluginList

#endif  // TESDATA_PLUGINSWIDGET_H
