#ifndef SETTINGS_H
#define SETTINGS_H

#include <imoinfo.h>
#include <lootcli/lootcli.h>

#include <QColor>
#include <QHeaderView>
#include <QSettings>

class Settings final
{
public:
  static void init(MOBase::IOrganizer* organizer);

  [[nodiscard]] static Settings* instance();

  void set(const QString& setting, const QVariant& value);

  [[nodiscard]] QVariant get(const QString& setting, const QVariant& def) const;

  template <typename T>
  [[nodiscard]] T get(const QString& setting, const QVariant& def) const
  {
    return get(setting, def).value<T>();
  }

  [[nodiscard]] QColor overwrittenLooseFilesColor() const;
  [[nodiscard]] QColor overwritingLooseFilesColor() const;
  [[nodiscard]] QColor overwrittenArchiveFilesColor() const;
  [[nodiscard]] QColor overwritingArchiveFilesColor() const;
  [[nodiscard]] QColor containedColor() const;
  [[nodiscard]] bool offlineMode() const;
  [[nodiscard]] lootcli::LogLevels lootLogLevel() const;

  void saveState(const QHeaderView* header);
  void restoreState(QHeaderView* header) const;

private:
  explicit Settings(MOBase::IOrganizer* organizer);

  MOBase::IOrganizer* Organizer;
  QSettings MOSettings;
};

#endif  // SETTINGS_H