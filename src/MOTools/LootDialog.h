#ifndef MODORGANIZER_LOOTDIALOG_H
#define MODORGANIZER_LOOTDIALOG_H

#include <expanderwidget.h>
#include <imoinfo.h>
#include <log.h>
#include <lootcli/lootcli.h>

#include <QDialog>
#include <QObject>
#include <QWebEnginePage>

namespace Ui
{
class LootDialog;
}

namespace MOTools
{

class MarkdownDocument : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QString text MEMBER m_Text NOTIFY textChanged FINAL);

public:
  explicit MarkdownDocument(QObject* parent = nullptr);
  void setText(const QString& text);

signals:
  void textChanged(const QString& text);

private:
  QString m_Text;
};

class MarkdownPage : public QWebEnginePage
{
  Q_OBJECT

public:
  explicit MarkdownPage(QObject* parent = nullptr);

protected:
  bool acceptNavigationRequest(const QUrl& url, NavigationType, bool) override;
};

class ILootCache;
class Loot;

class LootDialog : public QDialog
{
  Q_OBJECT

public:
  LootDialog(QWidget* parent, MOBase::IOrganizer* organizer, Loot& loot,
             ILootCache* lootCache, lootcli::LogLevels logLevel);

  LootDialog(const LootDialog&) = delete;
  LootDialog(LootDialog&&)      = delete;

  ~LootDialog() noexcept;

  LootDialog& operator=(const LootDialog&) = delete;
  LootDialog& operator=(LootDialog&&)      = delete;

  void setText(const QString& s);
  void setProgress(lootcli::Progress p);

  void addOutput(const QString& s);
  bool result() const;
  void cancel();
  void openReport();

  int exec() override;
  void accept() override;
  void reject() override;

private:
  std::unique_ptr<Ui::LootDialog> ui;
  MOBase::ExpanderWidget m_Expander;
  MOBase::IOrganizer* m_Organizer;
  Loot& m_Loot;
  ILootCache* m_LootCache;
  lootcli::LogLevels m_LogLevel;
  bool m_Finished;
  bool m_Cancelling;
  MarkdownDocument m_Report;

  void createUI();
  void closeEvent(QCloseEvent* e) override;
  void addLineOutput(const QString& line);
  void onFinished();
  void log(MOBase::log::Levels lv, const QString& s);
  void showReport();
};

}  // namespace MOTools

#endif  // MODORGANIZER_LOOTDIALOG_H
