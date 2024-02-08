#ifndef SETTINGS_H
#define SETTINGS_H

#include "GUI/IGeometrySettings.h"
#include "GUI/IStateSettings.h"

#include <expanderwidget.h>
#include <imoinfo.h>
#include <lootcli/lootcli.h>

#include <boost/signals2.hpp>

#include <QColor>
#include <QDialog>
#include <QHeaderView>
#include <QSettings>
#include <QSplitter>

class Settings final : public GUI::IGeometrySettings<QDialog>,
                       public GUI::IStateSettings<QHeaderView>,
                       public GUI::IStateSettings<QSplitter>,
                       public GUI::IStateSettings<MOBase::ExpanderWidget>
{
public:
  using SignalSettingChanged =
      boost::signals2::signal<void(const QString&, const QVariant&, const QVariant&)>;

  static void init(MOBase::IOrganizer* organizer);

  [[nodiscard]] static Settings* instance();

  void set(const QString& setting, const QVariant& value);

  [[nodiscard]] QVariant get(const QString& setting, const QVariant& def) const;

  template <typename T>
  [[nodiscard]] T get(const QString& setting, const QVariant& def) const
  {
    return get(setting, def).value<T>();
  }

  bool onSettingChanged(const std::function<void(const QString&, const QVariant&,
                                                 const QVariant&)>& func);

  [[nodiscard]] QColor overwrittenLooseFilesColor() const;
  [[nodiscard]] QColor overwritingLooseFilesColor() const;
  [[nodiscard]] QColor overwrittenArchiveFilesColor() const;
  [[nodiscard]] QColor overwritingArchiveFilesColor() const;
  [[nodiscard]] QColor containedColor() const;
  [[nodiscard]] bool offlineMode() const;
  [[nodiscard]] lootcli::LogLevels lootLogLevel() const;

  [[nodiscard]] bool externalChangeWarning() const;
  [[nodiscard]] bool enableSortButton() const;
  [[nodiscard]] bool lootShowDirty() const;
  [[nodiscard]] bool lootShowMessages() const;
  [[nodiscard]] bool lootShowProblems() const;

  void saveTreeExpandState(const QTreeView* view);
  void restoreTreeExpandState(QTreeView* view) const;

  void saveState(const QHeaderView* header) override;
  void restoreState(QHeaderView* header) const override;

  void saveState(const QSplitter* splitter) override;
  void restoreState(QSplitter* splitter) const override;

  void saveState(const MOBase::ExpanderWidget* expander) override;
  void restoreState(MOBase::ExpanderWidget* expander) const override;

  // IGeometrySettings<QDialog>

  void saveGeometry(const QDialog* dialog) override;
  void restoreGeometry(QDialog* dialog) const override;

private:
  void onPluginSettingChanged(const QString& pluginName, const QString& key,
                              const QVariant& oldValue, const QVariant& newValue);

  explicit Settings(MOBase::IOrganizer* organizer);

  MOBase::IOrganizer* Organizer;
  QSettings MOSettings;
  SignalSettingChanged SettingChanged;
};

#endif  // SETTINGS_H
