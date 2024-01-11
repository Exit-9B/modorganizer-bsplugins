#ifndef SETTINGS_H
#define SETTINGS_H

#include <imoinfo.h>
#include <lootcli/lootcli.h>

#include <QColor>
#include <QSettings>

class Settings final
{
public:
  static void init(MOBase::IOrganizer* organizer);

  [[nodiscard]] static Settings* instance();

  [[nodiscard]] QColor overwrittenLooseFilesColor() const;
  [[nodiscard]] QColor overwritingLooseFilesColor() const;
  [[nodiscard]] QColor overwrittenArchiveFilesColor() const;
  [[nodiscard]] QColor overwritingArchiveFilesColor() const;
  [[nodiscard]] QColor containedColor() const;
  [[nodiscard]] bool offlineMode() const;
  [[nodiscard]] lootcli::LogLevels lootLogLevel() const;

private:
  explicit Settings(MOBase::IOrganizer* organizer);

  MOBase::IOrganizer* Organizer;
  QSettings MOSettings;
};

#endif  // SETTINGS_H
