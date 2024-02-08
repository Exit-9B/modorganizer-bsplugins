#include "Settings.h"
#include "BSPlugins.h"

#include <QDir>
#include <QStandardPaths>

Settings* Instance = nullptr;

static QString findIniPath(MOBase::IOrganizer* organizer)
{
  // FIXME: we can't find non-portable instance paths
  return QDir(organizer->basePath()).filePath("ModOrganizer.ini");
}

Settings::Settings(MOBase::IOrganizer* organizer)
    : Organizer{organizer}, MOSettings{findIniPath(organizer), QSettings::IniFormat}
{
  organizer->onPluginSettingChanged(
      std::bind_front(&Settings::onPluginSettingChanged, this));
}

void Settings::init(MOBase::IOrganizer* organizer)
{
  Instance = new Settings(organizer);
}

Settings* Settings::instance()
{
  return Instance;
}

void Settings::set(const QString& setting, const QVariant& value)
{
  Organizer->setPersistent(BSPlugins::NAME, setting, value);
}

[[nodiscard]] QVariant Settings::get(const QString& setting, const QVariant& def) const
{
  return Organizer->persistent(BSPlugins::NAME, setting, def);
}

bool Settings::onSettingChanged(
    const std::function<void(const QString&, const QVariant&, const QVariant&)>& func)
{
  auto connection = SettingChanged.connect(func);
  return connection.connected();
}

QColor Settings::overwrittenLooseFilesColor() const
{
  return MOSettings.value("Settings/overwrittenLooseFilesColor", QColor(0, 255, 0, 64))
      .value<QColor>();
}

QColor Settings::overwritingLooseFilesColor() const
{
  return MOSettings.value("Settings/overwritingLooseFilesColor", QColor(255, 0, 0, 64))
      .value<QColor>();
}

QColor Settings::overwrittenArchiveFilesColor() const
{
  return MOSettings
      .value("Settings/overwrittenArchiveFilesColor", QColor(0, 255, 255, 64))
      .value<QColor>();
}

QColor Settings::overwritingArchiveFilesColor() const
{
  return MOSettings
      .value("Settings/overwritingArchiveFilesColor", QColor(255, 0, 255, 64))
      .value<QColor>();
}

QColor Settings::containedColor() const
{
  return MOSettings.value("Settings/containedColor", QColor(0, 0, 255, 64))
      .value<QColor>();
}

bool Settings::offlineMode() const
{
  return MOSettings.value("Settings/offline_mode", false).value<bool>();
}

lootcli::LogLevels Settings::lootLogLevel() const
{
  return MOSettings
      .value("Settings/loot_log_level", static_cast<int>(lootcli::LogLevels::Info))
      .value<lootcli::LogLevels>();
}

bool Settings::externalChangeWarning() const
{
  return Organizer->pluginSetting(BSPlugins::NAME, "external_change_warning")
      .value<bool>();
}

bool Settings::enableSortButton() const
{
  return Organizer->pluginSetting(BSPlugins::NAME, "enable_sort_button").value<bool>();
}

bool Settings::lootShowDirty() const
{
  return Organizer->pluginSetting(BSPlugins::NAME, "loot_show_dirty").value<bool>();
}

bool Settings::lootShowMessages() const
{
  return Organizer->pluginSetting(BSPlugins::NAME, "loot_show_messages").value<bool>();
}

bool Settings::lootShowProblems() const
{
  return Organizer->pluginSetting(BSPlugins::NAME, "loot_show_problems").value<bool>();
}

static QString stateSettingName(const QHeaderView* header)
{
  return header->parent()->objectName() + "_header";
}

void Settings::saveState(const QHeaderView* header)
{
  Organizer->setPersistent(BSPlugins::NAME, stateSettingName(header),
                           header->saveState());
}

void Settings::restoreState(QHeaderView* header) const
{
  const auto state =
      Organizer->persistent(BSPlugins::NAME, stateSettingName(header)).toByteArray();
  if (!state.isEmpty()) {
    header->restoreState(state);
  }
}

static QString stateSettingName(const QSplitter* splitter)
{
  return splitter->parentWidget()->objectName() + "_splitter";
}

void Settings::saveState(const QSplitter* splitter)
{
  Organizer->setPersistent(BSPlugins::NAME, stateSettingName(splitter),
                           splitter->saveState());
}

void Settings::restoreState(QSplitter* splitter) const
{
  const auto state =
      Organizer->persistent(BSPlugins::NAME, stateSettingName(splitter)).toByteArray();
  if (!state.isEmpty()) {
    splitter->restoreState(state);
  }
}

static QString stateSettingName(const MOBase::ExpanderWidget* expander)
{
  return expander->button()->objectName() + "_state";
}

void Settings::saveState(const MOBase::ExpanderWidget* expander)
{
  Organizer->setPersistent(BSPlugins::NAME, stateSettingName(expander),
                           expander->saveState());
}

void Settings::restoreState(MOBase::ExpanderWidget* expander) const
{
  const auto state =
      Organizer->persistent(BSPlugins::NAME, stateSettingName(expander)).toByteArray();
  if (!state.isEmpty()) {
    expander->restoreState(state);
  }
}

static QString geometrySettingName(const QDialog* dialog)
{
  return dialog->objectName() + "_geometry";
}

void Settings::saveGeometry(const QDialog* dialog)
{
  Organizer->setPersistent(BSPlugins::NAME, geometrySettingName(dialog),
                           dialog->saveGeometry());
}

void Settings::restoreGeometry(QDialog* dialog) const
{
  const auto geometry =
      Organizer->persistent(BSPlugins::NAME, geometrySettingName(dialog)).toByteArray();
  if (!geometry.isEmpty()) {
    dialog->restoreGeometry(geometry);
  }
}

void Settings::onPluginSettingChanged(const QString& pluginName, const QString& key,
                                      const QVariant& oldValue,
                                      const QVariant& newValue)
{
  if (pluginName != BSPlugins::NAME)
    return;

  SettingChanged(key, oldValue, newValue);
}
