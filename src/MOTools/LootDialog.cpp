#include "LootDialog.h"
#include "ILootCache.h"
#include "Loot.h"
#include "ui_lootdialog.h"

#include <utility.h>

#include <QCloseEvent>
#include <QWebChannel>

using namespace MOBase;

namespace MOTools
{

QString progressToString(lootcli::Progress p)
{
  using P = lootcli::Progress;

  static const std::map<P, QString> map = {
      {P::CheckingMasterlistExistence, QObject::tr("Checking masterlist existence")},
      {P::UpdatingMasterlist, QObject::tr("Updating masterlist")},
      {P::LoadingLists, QObject::tr("Loading lists")},
      {P::ReadingPlugins, QObject::tr("Reading plugins")},
      {P::SortingPlugins, QObject::tr("Sorting plugins")},
      {P::WritingLoadorder, QObject::tr("Writing loadorder.txt")},
      {P::ParsingLootMessages, QObject::tr("Parsing loot messages")},
      {P::Done, QObject::tr("Done")}};

  const auto it = map.find(p);
  if (it == map.end()) {
    return QString("unknown progress %1").arg(static_cast<int>(p));
  } else {
    return it->second;
  }
}

MarkdownDocument::MarkdownDocument(QObject* parent) : QObject(parent) {}

void MarkdownDocument::setText(const QString& text)
{
  if (m_Text == text)
    return;

  m_Text = text;
  emit textChanged(m_Text);
}

MarkdownPage::MarkdownPage(QObject* parent) : QWebEnginePage(parent) {}

bool MarkdownPage::acceptNavigationRequest(const QUrl& url, NavigationType, bool)
{
  static const QStringList allowed = {"qrc", "data"};

  if (!allowed.contains(url.scheme())) {
    shell::Open(url);
    return false;
  }

  return true;
}

LootDialog::LootDialog(QWidget* parent, MOBase::IOrganizer* organizer, Loot& loot,
                       ILootCache* lootCache, lootcli::LogLevels logLevel,
                       GUI::IGeometrySettings<QDialog>& geomSettings)
    : QDialog(parent, Qt::WindowMaximizeButtonHint), ui(new Ui::LootDialog),
      m_Organizer{organizer}, m_Loot{loot}, m_LootCache{lootCache}, m_Finished{false},
      m_Cancelling{false}, m_LogLevel{logLevel}, m_GeomSettings{geomSettings}
{
  createUI();

  QObject::connect(
      &m_Loot, &Loot::output, this,
      [&](auto&& s) {
        addOutput(s);
      },
      Qt::QueuedConnection);

  QObject::connect(
      &m_Loot, &Loot::progress, this,
      [&](auto&& p) {
        setProgress(p);
      },
      Qt::QueuedConnection);

  QObject::connect(
      &m_Loot, &Loot::log, this,
      [&](auto&& lv, auto&& s) {
        log(lv, s);
      },
      Qt::QueuedConnection);

  QObject::connect(
      &m_Loot, &Loot::finished, this,
      [&] {
        onFinished();
      },
      Qt::QueuedConnection);
}

LootDialog::~LootDialog() noexcept = default;

void LootDialog::setText(const QString& s)
{
  ui->progressText->setText(s);
}

void LootDialog::setProgress(lootcli::Progress p)
{
  // don't overwrite the "stopping loot" message even if lootcli generates a new
  // progress message
  if (!m_Cancelling) {
    setText(progressToString(p));
  }

  if (p == lootcli::Progress::Done) {
    ui->progressBar->setRange(0, 1);
    ui->progressBar->setValue(1);
  }
}

void LootDialog::addOutput(const QString& s)
{
  if (m_LogLevel > lootcli::LogLevels::Debug) {
    return;
  }

  const auto lines = s.split(QRegularExpression("[\\r\\n]"), Qt::SkipEmptyParts);

  for (auto&& line : lines) {
    if (line.isEmpty()) {
      continue;
    }

    addLineOutput(line);
  }
}

bool LootDialog::result() const
{
  return m_Loot.result();
}

void LootDialog::cancel()
{
  if (!m_Finished && !m_Cancelling) {
    log::debug("loot dialog: cancelling");
    m_Loot.cancel();

    setText(tr("Stopping LOOT..."));
    addLineOutput("stopping loot");

    ui->buttons->setEnabled(false);
    m_Cancelling = true;
  }
}

void LootDialog::openReport()
{
  const auto path = m_Loot.outPath();
  shell::Open(path);
}

int LootDialog::exec()
{
  GUI::GeometrySaver gs{m_GeomSettings, this};

  const auto r = QDialog::exec();

  return r;
}

void LootDialog::accept()
{
  // no-op
}

void LootDialog::reject()
{
  if (m_Finished) {
    log::debug("loot dialog reject: loot finished, closing");
    QDialog::reject();
  } else {
    log::debug("loot dialog reject: not finished, cancelling");
    cancel();
  }
}

void LootDialog::createUI()
{
  ui->setupUi(this);
  ui->progressBar->setMaximum(0);

  auto* page = new MarkdownPage(this);
  ui->report->setPage(page);

  auto* channel = new QWebChannel(this);
  channel->registerObject("content", &m_Report);
  page->setWebChannel(channel);

  const QString path = QApplication::applicationDirPath() + "/resources/markdown.html";
  QFile f(path);

  if (f.open(QFile::ReadOnly)) {
    const QString html = f.readAll();
    if (!html.isEmpty()) {
      ui->report->setHtml(html);
    } else {
      log::error("failed to read '{}', {}", path, f.errorString());
    }
  } else {
    log::error("can't open '{}', {}", path, f.errorString());
  }

#ifdef LOOT_STDOUT_AVAILABLE
  m_Expander.set(ui->details, ui->detailsPanel);
#endif
  ui->openJsonReport->setEnabled(false);
  connect(ui->openJsonReport, &QPushButton::clicked, [&] {
    openReport();
  });

  ui->buttons->setStandardButtons(QDialogButtonBox::Cancel);

  m_Report.setText(tr("Running LOOT..."));

  resize(650, 450);
  setSizeGripEnabled(true);
}

void LootDialog::closeEvent(QCloseEvent* e)
{
  if (m_Finished) {
    log::debug("loot dialog close event: finished, closing");
    QDialog::closeEvent(e);
  } else {
    log::debug("loot dialog close event: not finished, cancelling");
    cancel();
    e->ignore();
  }
}

void LootDialog::addLineOutput([[maybe_unused]] const QString& line)
{
#ifdef LOOT_STDOUT_AVAILABLE
  ui->output->appendPlainText(line);
#endif
}

void LootDialog::onFinished()
{
  log::debug("loot dialog: loot is finished");

  m_Finished = true;

  if (m_Cancelling) {
    log::debug("loot dialog: was cancelling, closing");
    close();
  } else {
    log::debug("loot dialog: showing report");

    showReport();

    ui->openJsonReport->setEnabled(true);
    ui->buttons->setStandardButtons(QDialogButtonBox::Close);

    // if loot failed, the Done progress won't be received; this makes sure
    // the progress bar is stopped
    setProgress(lootcli::Progress::Done);
  }
}

void LootDialog::log(log::Levels lv, const QString& s)
{
  if (lv >= log::Levels::Warning) {
    log::log(lv, "{}", s);
  }

  if (m_LogLevel > lootcli::LogLevels::Debug) {
    addLineOutput(QString("[%1] %2").arg(log::levelToString(lv)).arg(s));
  }
}

void LootDialog::showReport()
{
  const auto& lootReport = m_Loot.report();

  if (m_Loot.result() && m_LootCache != nullptr) {
    m_LootCache->clearAdditionalInformation();
    for (auto&& p : lootReport.plugins) {
      m_LootCache->addLootReport(p.name, p);
    }
  }

  m_Report.setText(lootReport.toMarkdown());
}

}  // namespace MOTools
