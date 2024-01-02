#ifndef MOPANELINTERFACE_H
#define MOPANELINTERFACE_H

#include "IPanelInterface.h"

#include <imoinfo.h>

#include <boost/signals2.hpp>

#include <QMainWindow>
#include <QTreeView>

class MOPanelInterface final : public QObject, public IPanelInterface
{
  Q_OBJECT

public:
  using SignalPanelActivated = boost::signals2::signal<void()>;
  using SignalSelectedOriginsChanged =
      boost::signals2::signal<void(const QList<QString>&)>;

  MOPanelInterface(MOBase::IOrganizer* organizer, QMainWindow* mainWindow);

  MOPanelInterface(const MOPanelInterface&) = delete;
  MOPanelInterface(MOPanelInterface&&)      = delete;

  ~MOPanelInterface() noexcept;

  MOPanelInterface& operator=(const MOPanelInterface&) = delete;
  MOPanelInterface& operator=(MOPanelInterface&&)      = delete;

  void assignWidget(QTabWidget* tabWidget, QWidget* panel);

  void setSelectedFiles(const QList<QString>& selectedFiles) override;

  bool onPanelActivated(const std::function<void()>& func) override;

  bool onSelectedOriginsChanged(
      const std::function<void(const QList<QString>&)>& func) override;

private slots:
  void onModSelectionChanged();

private:
  MOBase::IModList* m_ModList;
  MOBase::IPluginList* m_PluginList;
  QTreeView* m_ModListView;
  QTreeView* m_PluginListView;

  SignalPanelActivated m_PanelActivated;
  SignalSelectedOriginsChanged m_SelectedOriginsChanged;
};

#endif  // MOPANELINTERFACE_H
