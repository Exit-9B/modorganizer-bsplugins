#ifndef MOTOOLS_LOOT_H
#define MOTOOLS_LOOT_H

#include "GUI/IGeometrySettings.h"

#include <imoinfo.h>
#include <log.h>
#include <lootcli/lootcli.h>

#include <QObject>
#include <QString>

#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace MOTools
{

namespace env
{
  struct HandleCloser
  {
    using pointer = HANDLE;

    void operator()(HANDLE h)
    {
      if (h != INVALID_HANDLE_VALUE) {
        ::CloseHandle(h);
      }
    }
  };

  using HandlePtr = std::unique_ptr<HANDLE, HandleCloser>;
}  // namespace env

class AsyncPipe;
class ILootCache;

class Loot final : public QObject
{
  Q_OBJECT

public:
  struct Message
  {
    MOBase::log::Levels type;
    QString text;

    QString toMarkdown() const;
  };

  struct File
  {
    QString name;
    QString displayName;
  };

  struct Dirty
  {
    qint64 crc               = 0;
    qint64 itm               = 0;
    qint64 deletedReferences = 0;
    qint64 deletedNavmesh    = 0;
    QString cleaningUtility;
    QString info;

    QString toString(bool isClean) const;
    QString toMarkdown(bool isClean) const;
    QString cleaningString() const;
  };

  struct Plugin
  {
    QString name;
    std::vector<File> incompatibilities;
    std::vector<Message> messages;
    std::vector<Dirty> dirty, clean;
    std::vector<QString> missingMasters;
    bool loadsArchive  = false;
    bool isMaster      = false;
    bool isLightMaster = false;

    QString toMarkdown() const;
  };

  struct Stats
  {
    qint64 time = 0;
    QString lootcliVersion;
    QString lootVersion;

    QString toMarkdown() const;
  };

  struct Report
  {
    bool okay = false;
    std::vector<QString> errors, warnings;
    std::vector<Message> messages;
    std::vector<Plugin> plugins;
    Stats stats;

    QString toMarkdown() const;

  private:
    QString successMarkdown() const;
    QString errorsMarkdown() const;
  };

  Loot(MOBase::IOrganizer* organizer, lootcli::LogLevels logLevel);

  Loot(const Loot&) = delete;
  Loot(Loot&&)      = delete;

  ~Loot();

  Loot& operator=(const Loot&) = delete;
  Loot& operator=(Loot&&)      = delete;

  bool start(QWidget* parent, bool didUpdateMasterList);
  void cancel();
  bool result() const;

  const QString& outPath() const;
  const Report& report() const;
  const std::vector<QString>& errors() const;
  const std::vector<QString>& warnings() const;

signals:
  void output(const QString& s);
  void progress(const lootcli::Progress p);
  void log(MOBase::log::Levels level, const QString& s) const;
  void finished();

private:
  MOBase::IOrganizer* m_Organizer;
  lootcli::LogLevels m_LogLevel;
  std::unique_ptr<QThread> m_Thread;
  std::atomic<bool> m_Cancel;
  std::atomic<bool> m_Result;
  env::HandlePtr m_LootProcess;
  std::unique_ptr<AsyncPipe> m_Pipe;
  std::string m_OutputBuffer;
  std::vector<QString> m_Errors, m_Warnings;
  Report m_Report;

  bool spawnLootcli(QWidget* parent, bool didUpdateMasterList,
                    env::HandlePtr stdoutHandle);

  void lootThread();
  bool waitForCompletion();

  void processStdout(const std::string& lootOut);
  void processMessage(const lootcli::Message& m);

  Report createReport() const;
  void processOutputFile(Report& r) const;
  void deleteReportFile();

  Message reportMessage(const QJsonObject& message) const;
  std::vector<Plugin> reportPlugins(const QJsonArray& plugins) const;
  Loot::Plugin reportPlugin(const QJsonObject& plugin) const;
  Loot::Stats reportStats(const QJsonObject& stats) const;

  std::vector<Message> reportMessages(const QJsonArray& array) const;
  std::vector<Loot::File> reportFiles(const QJsonArray& array) const;
  std::vector<Loot::Dirty> reportDirty(const QJsonArray& array) const;
  std::vector<QString> reportStringArray(const QJsonArray& array) const;
};

bool runLoot(QWidget* parent, MOBase::IOrganizer* organizer, ILootCache* lootCache,
             lootcli::LogLevels logLevel, bool didUpdateMasterList,
             GUI::IGeometrySettings<QDialog>& geomSettings);

}  // namespace MOTools

#endif  // MOTOOLS_LOOT_H
