#include "Loot.h"
#include "ILootCache.h"
#include "LootDialog.h"
#include "json.h"

#include <iplugingame.h>
#include <ipluginlist.h>
#include <utility.h>

#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>

#include <algorithm>
#include <cstring>
#include <iterator>

using namespace MOBase;
using namespace json;

static QString LootReportPath      = QDir::temp().absoluteFilePath("lootreport.json");
static constexpr DWORD PipeTimeout = 500;

namespace MOTools
{

class AsyncPipe final
{
public:
  AsyncPipe() : m_IoPending{false}
  {
    std::fill(std::begin(m_Buffer), std::end(m_Buffer), 0);
    std::memset(&m_Ov, 0, sizeof(m_Ov));
  }

  env::HandlePtr create()
  {
    // creating pipe
    env::HandlePtr out{createPipe()};
    if (out.get() == INVALID_HANDLE_VALUE) {
      return {};
    }

    HANDLE readEventHandle = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);

    if (readEventHandle == NULL) {
      const auto e = ::GetLastError();
      log::error("CreateEvent failed for loot, {}", formatSystemMessage(e));
      return {};
    }

    m_Ov.hEvent = readEventHandle;
    m_ReadEvent.reset(readEventHandle);

    return out;
  }

  std::string read()
  {
    if (m_IoPending) {
      return checkPending();
    } else {
      return tryRead();
    }
  }

private:
  static constexpr std::size_t bufferSize = 65536;

  env::HandlePtr m_Stdout;
  env::HandlePtr m_ReadEvent;
  char m_Buffer[bufferSize];
  OVERLAPPED m_Ov;
  bool m_IoPending;

  HANDLE createPipe()
  {
    static constexpr const TCHAR* PipeName = TEXT(R"(\\.\pipe\lootcli_pipe)");

    SECURITY_ATTRIBUTES sa = {
        .nLength        = sizeof(SECURITY_ATTRIBUTES),
        .bInheritHandle = TRUE,
    };

    env::HandlePtr pipe;

    // creating pipe
    {
      HANDLE pipeHandle =
          ::CreateNamedPipe(PipeName, PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 65'536,
                            65'536, PipeTimeout, &sa);

      if (pipeHandle == INVALID_HANDLE_VALUE) {
        const auto e = ::GetLastError();
        log::error("CreateNamedPipe failed, {}", formatSystemMessage(e));
        return INVALID_HANDLE_VALUE;
      }

      pipe.reset(pipeHandle);
    }

    {
      // duplicating the handle to read from it
      HANDLE outputRead = INVALID_HANDLE_VALUE;

      const auto r =
          ::DuplicateHandle(::GetCurrentProcess(), pipe.get(), ::GetCurrentProcess(),
                            &outputRead, 0, TRUE, DUPLICATE_SAME_ACCESS);

      if (!r) {
        const auto e = ::GetLastError();
        log::error("DuplicateHandle for pipe failed, {}", formatSystemMessage(e));
        return INVALID_HANDLE_VALUE;
      }

      m_Stdout.reset(outputRead);
    }

    // creating handle to pipe which is passed to CreateProcess()
    HANDLE outputWrite = ::CreateFile(PipeName, FILE_WRITE_DATA | SYNCHRONIZE, 0, &sa,
                                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

    if (outputWrite == INVALID_HANDLE_VALUE) {
      const auto e = ::GetLastError();
      log::error("CreateFile for pipe failed, {}", formatSystemMessage(e));
      return INVALID_HANDLE_VALUE;
    }

    return outputWrite;
  }

  std::string tryRead()
  {
    DWORD bytesRead = 0;

    if (!::ReadFile(m_Stdout.get(), m_Buffer, bufferSize, &bytesRead, &m_Ov)) {
      const auto e = ::GetLastError();

      switch (e) {
      case ERROR_IO_PENDING: {
        m_IoPending = true;
        break;
      }

      case ERROR_BROKEN_PIPE: {
        // broken pipe probably means lootcli is finished
        break;
      }

      default: {
        log::error("{}", formatSystemMessage(e));
        break;
      }
      }

      return {};
    }

    log::debug("read {} bytes", bytesRead);
    return {m_Buffer, m_Buffer + bytesRead};
  }

  std::string checkPending()
  {
    DWORD bytesRead = 0;

    const auto r = ::WaitForSingleObject(m_ReadEvent.get(), PipeTimeout);

    if (r == WAIT_FAILED) {
      const auto e = ::GetLastError();
      log::error("WaitForSingleObject in AsyncPipe failed, {}", formatSystemMessage(e));
      return {};
    }

    if (!::GetOverlappedResult(m_Stdout.get(), &m_Ov, &bytesRead, FALSE)) {
      const auto e = ::GetLastError();

      switch (e) {
      case ERROR_IO_INCOMPLETE: {
        break;
      }

      case WAIT_TIMEOUT: {
        break;
      }

      case ERROR_BROKEN_PIPE: {
        // broken pipe probably means lootcli is finished
        break;
      }

      default: {
        log::error("GetOverlappedResult failed, {}", formatSystemMessage(e));
        break;
      }
      }

      return {};
    }

    ::ResetEvent(m_ReadEvent.get());
    m_IoPending = false;

    return {m_Buffer, m_Buffer + bytesRead};
  }
};

static log::Levels levelFromLoot(lootcli::LogLevels level)
{
  using LC = lootcli::LogLevels;

  switch (level) {
  case LC::Trace:  // fall-through
  case LC::Debug:
    return log::Debug;

  case LC::Info:
    return log::Info;

  case LC::Warning:
    return log::Warning;

  case LC::Error:
    return log::Error;

  default:
    return log::Info;
  }
}

QString Loot::Report::toMarkdown() const
{
  QString s;

  if (!okay) {
    s += "## " + tr("Loot failed to run") + "\n";

    if (errors.empty() && warnings.empty()) {
// FIXME: we can't pass an stdout handle to lootcli, so we can't get errors
#ifdef LOOT_STDOUT_AVAILABLE
      s += tr("No errors were reported. The log below might have more information.\n");
#else
      s += tr("Get more information by using the LOOT application to sort your load "
              "order.\n");
#endif
    }
  }

  s += errorsMarkdown();

  if (okay) {
    s += "\n" + successMarkdown();
  }

  return s;
}

QString Loot::Report::successMarkdown() const
{
  QString s;

  if (!messages.empty()) {
    s += "### " + QObject::tr("General messages") + "\n";

    for (auto&& m : messages) {
      s += " - " + m.toMarkdown() + "\n";
    }
  }

  if (!plugins.empty()) {
    if (!s.isEmpty()) {
      s += "\n";
    }

    s += "### " + QObject::tr("Plugins") + "\n";

    for (auto&& p : plugins) {
      const auto ps = p.toMarkdown();
      if (!ps.isEmpty()) {
        s += ps + "\n";
      }
    }
  }

  if (s.isEmpty()) {
    s += "**" + QObject::tr("No messages.") + "**\n";
  }

  s += stats.toMarkdown();

  return s;
}

QString Loot::Report::errorsMarkdown() const
{
  QString s;

  if (!errors.empty()) {
    s += "### " + tr("Errors") + ":\n";

    for (auto&& e : errors) {
      s += " - " + e + "\n";
    }
  }

  if (!warnings.empty()) {
    if (!s.isEmpty()) {
      s += "\n";
    }

    s += "### " + tr("Warnings") + ":\n";

    for (auto&& w : warnings) {
      s += " - " + w + "\n";
    }
  }

  return s;
}

QString Loot::Stats::toMarkdown() const
{
  return QString("`stats: %1s, lootcli %2, loot %3`")
      .arg(QString::number(time / 1000.0, 'f', 2))
      .arg(lootcliVersion)
      .arg(lootVersion);
}

QString Loot::Plugin::toMarkdown() const
{
  QString s;

  if (!incompatibilities.empty()) {
    s += " - **" + QObject::tr("Incompatibilities") + ": ";

    QString fs;
    for (auto&& f : incompatibilities) {
      if (!fs.isEmpty()) {
        fs += ", ";
      }

      fs += f.displayName.isEmpty() ? f.name : f.displayName;
    }

    s += fs + "**\n";
  }

  if (!missingMasters.empty()) {
    s += " - **" + QObject::tr("Missing masters") + ": ";

    QString ms;
    for (auto&& m : missingMasters) {
      if (!ms.isEmpty()) {
        ms += ", ";
      }

      ms += m;
    }

    s += ms + "**\n";
  }

  for (auto&& m : messages) {
    s += " - " + m.toMarkdown() + "\n";
  }

  for (auto&& d : dirty) {
    s += " - " + d.toMarkdown(false) + "\n";
  }

  if (!s.isEmpty()) {
    s = "#### " + name + "\n" + s;
  }

  return s;
}

QString Loot::Dirty::toString(bool isClean) const
{
  if (isClean) {
    return QObject::tr("Verified clean by %1")
        .arg(cleaningUtility.isEmpty() ? "?" : cleaningUtility);
  }

  QString s = cleaningString();

  if (!info.isEmpty()) {
    s += " " + info;
  }

  return s;
}

QString Loot::Dirty::toMarkdown(bool isClean) const
{
  return toString(isClean);
}

QString Loot::Dirty::cleaningString() const
{
  return QObject::tr("%1 found %2 ITM record(s), %3 deleted reference(s) and %4 "
                     "deleted navmesh(es).")
      .arg(cleaningUtility.isEmpty() ? "?" : cleaningUtility)
      .arg(itm)
      .arg(deletedReferences)
      .arg(deletedNavmesh);
}

QString Loot::Message::toMarkdown() const
{
  QString s;

  switch (type) {
  case log::Error: {
    s += "**" + QObject::tr("Error") + "**: ";
    break;
  }

  case log::Warning: {
    s += "**" + QObject::tr("Warning") + "**: ";
    break;
  }

  default: {
    break;
  }
  }

  s += text;

  return s;
}

Loot::Loot(MOBase::IOrganizer* organizer, lootcli::LogLevels logLevel)
    : m_Organizer{organizer}, m_LogLevel{logLevel}, m_Thread{nullptr}, m_Cancel{false},
      m_Result{false}
{}

Loot::~Loot()
{
  if (m_Thread) {
    m_Thread->wait();
  }

  deleteReportFile();
}

bool Loot::start(QWidget* parent, bool didUpdateMasterList)
{
  deleteReportFile();

  log::debug("starting loot");

  m_Pipe.reset(new AsyncPipe);
  env::HandlePtr stdoutHandle = m_Pipe->create();

  // spawning
  if (!spawnLootcli(parent, didUpdateMasterList, std::move(stdoutHandle))) {
    return false;
  }

  // starting thread
  log::debug("starting loot thread");
  m_Thread.reset(QThread::create([this] {
    lootThread();
  }));
  m_Thread->start();

  return true;
}

bool Loot::spawnLootcli(QWidget* parent, bool didUpdateMasterList,
                        [[maybe_unused]] env::HandlePtr stdoutHandle)
{
  QStringList parameters;
  parameters << "--game" << m_Organizer->managedGame()->gameShortName()

             << "--gamePath"
             << QString("\"%1\"").arg(
                    m_Organizer->managedGame()->gameDirectory().absolutePath())

             << "--pluginListPath"
             << QString("\"%1/loadorder.txt\"").arg(m_Organizer->profilePath())

             << "--logLevel"
             << QString::fromStdString(lootcli::logLevelToString(m_LogLevel))

             << "--out" << QString("\"%1\"").arg(LootReportPath)

             << "--language" << parent->locale().name();

  if (didUpdateMasterList) {
    parameters << "--skipUpdateMasterlist";
  }

  QString binary           = qApp->applicationDirPath() + "/loot/lootcli.exe";
  QString currentDirectory = qApp->applicationDirPath() + "/loot";

  // FIXME: IOrganizer doesn't allow us to capture stdout
  HANDLE lootHandle =
      m_Organizer->startApplication(binary, parameters, currentDirectory);

  if (lootHandle == INVALID_HANDLE_VALUE) {
    emit log(log::Levels::Error, tr("failed to start loot"));
    return false;
  }

  m_LootProcess.reset(lootHandle);

  return true;
}

void Loot::cancel()
{
  if (!m_Cancel) {
    log::debug("loot received cancel request");
    m_Cancel = true;
  }
}

bool Loot::result() const
{
  return m_Result;
}

const QString& Loot::outPath() const
{
  return LootReportPath;
}

const Loot::Report& Loot::report() const
{
  return m_Report;
}

const std::vector<QString>& Loot::errors() const
{
  return m_Errors;
}

const std::vector<QString>& Loot::warnings() const
{
  return m_Warnings;
}

void Loot::lootThread()
{
  try {
    m_Result = false;

    if (waitForCompletion()) {
      m_Result = true;
    }

    m_Report = createReport();
  } catch (...) {
    log::error("unhandled exception in loot thread");
  }

  log::debug("finishing loot thread");
  emit finished();
}

bool Loot::waitForCompletion()
{
  log::debug("loot thread waiting for completion on lootcli");

  for (;;) {
    DWORD res = ::WaitForSingleObject(m_LootProcess.get(), 100);

    if (res == WAIT_OBJECT_0) {
      log::debug("lootcli has completed");
      // done
      break;
    }

    if (res == WAIT_FAILED) {
      const auto e = ::GetLastError();
      log::error("failed to wait on loot process, {}", formatSystemMessage(e));
      return false;
    }

    if (m_Cancel) {
      log::debug("terminating lootcli process");
      ::TerminateProcess(m_LootProcess.get(), 1);

      log::debug("waiting for loocli process to terminate");
      ::WaitForSingleObject(m_LootProcess.get(), INFINITE);

      log::debug("lootcli terminated");
      return false;
    }

    if (m_Pipe) {
      processStdout(m_Pipe->read());
    }
  }

  if (m_Cancel) {
    return false;
  }

  if (m_Pipe) {
    processStdout(m_Pipe->read());
  }

  // checking exit code
  DWORD exitCode = 0;

  if (!::GetExitCodeProcess(m_LootProcess.get(), &exitCode)) {
    const auto e = ::GetLastError();
    log::error("failed to get exit code for loot, {}", formatSystemMessage(e));
    return false;
  }

  if (exitCode != 0UL) {
    emit log(log::Levels::Error,
             tr("Loot failed. Exit code was: 0x%1").arg(exitCode, 0, 16));
    return false;
  }

  return true;
}

void Loot::processStdout(const std::string& lootOut)
{
  emit output(QString::fromStdString(lootOut));

  m_OutputBuffer += lootOut;
  if (m_OutputBuffer.empty()) {
    return;
  }

  std::size_t start = 0;

  for (;;) {
    const auto newline = m_OutputBuffer.find("\n", start);
    if (newline == std::string::npos) {
      break;
    }

    const std::string_view line(m_OutputBuffer.c_str() + start, newline - start);
    const auto m = lootcli::parseMessage(line);

    if (m.type == lootcli::MessageType::None) {
      log::error("unrecognised loot output: '{}'", line);
      continue;
    }

    processMessage(m);

    start = newline + 1;
  }

  m_OutputBuffer.erase(0, start);
}

void Loot::processMessage(const lootcli::Message& m)
{
  switch (m.type) {
  case lootcli::MessageType::Log: {
    const auto level = levelFromLoot(m.logLevel);

    if (level == log::Error) {
      m_Errors.push_back(QString::fromStdString(m.log));
    } else if (level == log::Warning) {
      m_Warnings.push_back(QString::fromStdString(m.log));
    }

    emit log(level, QString::fromStdString(m.log));
    break;
  }

  case lootcli::MessageType::Progress: {
    emit progress(m.progress);
    break;
  }
  }
}

Loot::Report Loot::createReport() const
{
  Report r;

  r.okay     = m_Result;
  r.errors   = m_Errors;
  r.warnings = m_Warnings;

  if (m_Result) {
    processOutputFile(r);
  }

  return r;
}

void Loot::deleteReportFile()
{
  if (QFile::exists(LootReportPath)) {
    log::debug("deleting temporary loot report '{}'", LootReportPath);
    const auto r = shell::Delete(QFileInfo(LootReportPath));

    if (!r) {
      log::error("failed to remove temporary loot json report '{}': {}", LootReportPath,
                 r.toString());
    }
  }
}

void Loot::processOutputFile(Report& r) const
{
  log::debug("parsing json output file at '{}'", LootReportPath);

  QFile outFile(LootReportPath);
  if (!outFile.open(QIODevice::ReadOnly)) {
    emit log(MOBase::log::Error, QString("failed to open file, %1 (error %2)")
                                     .arg(outFile.errorString())
                                     .arg(outFile.error()));

    return;
  }

  QJsonParseError e;
  const QJsonDocument doc = QJsonDocument::fromJson(outFile.readAll(), &e);
  if (doc.isNull()) {
    emit log(MOBase::log::Error,
             QString("invalid json, %1 (error %2)").arg(e.errorString()).arg(e.error));

    return;
  }

  requireObject(doc, "root");

  const QJsonObject object = doc.object();

  r.messages = reportMessages(getOpt<QJsonArray>(object, "messages"));
  r.plugins  = reportPlugins(getOpt<QJsonArray>(object, "plugins"));
  r.stats    = reportStats(getWarn<QJsonObject>(object, "stats"));
}

std::vector<Loot::Plugin> Loot::reportPlugins(const QJsonArray& plugins) const
{
  std::vector<Loot::Plugin> v;

  for (auto pluginValue : plugins) {
    const auto o = convertWarn<QJsonObject>(pluginValue, "plugin");
    if (o.isEmpty()) {
      continue;
    }

    auto p = reportPlugin(o);
    if (!p.name.isEmpty()) {
      v.emplace_back(std::move(p));
    }
  }

  return v;
}

Loot::Plugin Loot::reportPlugin(const QJsonObject& plugin) const
{
  Plugin p;

  p.name = getWarn<QString>(plugin, "name");
  if (p.name.isEmpty()) {
    return {};
  }

  // ignore disabled plugins; lootcli doesn't know if a plugin is enabled or not
  // and will report information on any plugin that's in the filesystem
  if (m_Organizer->pluginList()->state(p.name) != IPluginList::STATE_ACTIVE) {
    return {};
  }

  if (plugin.contains("incompatibilities")) {
    p.incompatibilities = reportFiles(getOpt<QJsonArray>(plugin, "incompatibilities"));
  }

  if (plugin.contains("messages")) {
    p.messages = reportMessages(getOpt<QJsonArray>(plugin, "messages"));
  }

  if (plugin.contains("dirty")) {
    p.dirty = reportDirty(getOpt<QJsonArray>(plugin, "dirty"));
  }

  if (plugin.contains("clean")) {
    p.clean = reportDirty(getOpt<QJsonArray>(plugin, "clean"));
  }

  if (plugin.contains("missingMasters")) {
    p.missingMasters = reportStringArray(getOpt<QJsonArray>(plugin, "missingMasters"));
  }

  p.loadsArchive  = getOpt(plugin, "loadsArchive", false);
  p.isMaster      = getOpt(plugin, "isMaster", false);
  p.isLightMaster = getOpt(plugin, "isLightMaster", false);

  return p;
}

Loot::Stats Loot::reportStats(const QJsonObject& stats) const
{
  Stats s;

  s.time           = getWarn<qint64>(stats, "time");
  s.lootcliVersion = getWarn<QString>(stats, "lootcliVersion");
  s.lootVersion    = getWarn<QString>(stats, "lootVersion");

  return s;
}

std::vector<Loot::Message> Loot::reportMessages(const QJsonArray& array) const
{
  std::vector<Loot::Message> v;

  for (auto messageValue : array) {
    const auto o = convertWarn<QJsonObject>(messageValue, "message");
    if (o.isEmpty()) {
      continue;
    }

    Message m;

    const auto type = getWarn<QString>(o, "type");

    if (type == "info") {
      m.type = log::Info;
    } else if (type == "warn") {
      m.type = log::Warning;
    } else if (type == "error") {
      m.type = log::Error;
    } else {
      log::error("unknown message type '{}'", type);
      m.type = log::Info;
    }

    m.text = getWarn<QString>(o, "text");

    if (!m.text.isEmpty()) {
      v.emplace_back(std::move(m));
    }
  }

  return v;
}

std::vector<Loot::File> Loot::reportFiles(const QJsonArray& array) const
{
  std::vector<Loot::File> v;

  for (auto&& fileValue : array) {
    const auto o = convertWarn<QJsonObject>(fileValue, "file");
    if (o.isEmpty()) {
      continue;
    }

    File f;

    f.name        = getWarn<QString>(o, "name");
    f.displayName = getOpt<QString>(o, "displayName");

    if (!f.name.isEmpty()) {
      v.emplace_back(std::move(f));
    }
  }

  return v;
}

std::vector<Loot::Dirty> Loot::reportDirty(const QJsonArray& array) const
{
  std::vector<Loot::Dirty> v;

  for (auto&& dirtyValue : array) {
    const auto o = convertWarn<QJsonObject>(dirtyValue, "dirty");

    Dirty d;

    d.crc               = getWarn<qint64>(o, "crc");
    d.itm               = getOpt<qint64>(o, "itm");
    d.deletedReferences = getOpt<qint64>(o, "deletedReferences");
    d.deletedNavmesh    = getOpt<qint64>(o, "deletedNavmesh");
    d.cleaningUtility   = getOpt<QString>(o, "cleaningUtility");
    d.info              = getOpt<QString>(o, "info");

    v.emplace_back(std::move(d));
  }

  return v;
}

std::vector<QString> Loot::reportStringArray(const QJsonArray& array) const
{
  std::vector<QString> v;

  for (auto&& sv : array) {
    auto s = convertWarn<QString>(sv, "string");
    if (s.isEmpty()) {
      continue;
    }

    v.emplace_back(std::move(s));
  }

  return v;
}

bool runLoot(QWidget* parent, MOBase::IOrganizer* organizer, ILootCache* lootCache,
             lootcli::LogLevels logLevel, bool didUpdateMasterList,
             GUI::IGeometrySettings<QDialog>& geomSettings)
{
  try {
    Loot loot{organizer, logLevel};
    LootDialog dialog{parent, organizer, loot, lootCache, logLevel, geomSettings};

    if (!loot.start(parent, didUpdateMasterList)) {
      return false;
    }

    dialog.exec();

    return dialog.result();
  } catch (const std::exception& e) {
    log::error(QObject::tr("failed to run loot: %1").arg(e.what()).toStdString());
    return false;
  }
}

}  // namespace MOTools
