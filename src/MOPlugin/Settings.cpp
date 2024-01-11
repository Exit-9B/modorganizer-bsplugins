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
{}

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
