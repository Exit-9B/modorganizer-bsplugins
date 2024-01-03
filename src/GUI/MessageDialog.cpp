#include "MessageDialog.h"
#include "ui_messagedialog.h"

#include <log.h>

#include <Windows.h>

#include <QMainWindow>
#include <QResizeEvent>
#include <QTimer>

using namespace MOBase;

namespace GUI
{

MessageDialog::MessageDialog(const QString& text, QWidget* reference)
    : QDialog(reference), ui{new Ui::MessageDialog}
{
  ui->setupUi(this);

  // very crude way to ensure no single word in the test is wider than the message
  // window. ellide in the center if necessary
  QFontMetrics metrics(ui->message->font());
  QString restrictedText;
  QStringList lines = text.split("\n");
  foreach (const QString& line, lines) {
    QString newLine;
    QStringList words = line.split(" ");
    foreach (const QString& word, words) {
      if (word.length() > 10) {
        newLine +=
            "<span style=\"nobreak\">" +
            metrics.elidedText(word, Qt::ElideMiddle, ui->message->maximumWidth()) +
            "</span>";
      } else {
        newLine += word;
      }
      newLine += " ";
    }
    restrictedText += newLine + "\n";
  }

  ui->message->setText(restrictedText);
  this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  this->setFocusPolicy(Qt::NoFocus);
  this->setAttribute(Qt::WA_ShowWithoutActivating);
  QTimer::singleShot(1000 + (text.length() * 40), this, SLOT(hide()));
  if (reference != nullptr) {
    QPoint position =
        reference->mapToGlobal(QPoint(reference->width() / 2, reference->height()));
    position.rx() -= this->width() / 2;
    position.ry() -= this->height() + 5;
    move(position);
  }
}

MessageDialog::~MessageDialog()
{
  delete ui;
}

void MessageDialog::resizeEvent(QResizeEvent* event)
{
  QWidget* par = parentWidget();
  if (par != nullptr) {
    QPoint position = par->mapToGlobal(QPoint(par->width() / 2, par->height()));
    position.rx() -= event->size().width() / 2;
    position.ry() -= event->size().height() + 5;
    move(position);
  }
}

void MessageDialog::showMessage(const QString& text, QWidget* reference,
                                bool bringToFront)
{
  log::debug("{}", text);

  if (!reference) {
    for (QWidget* w : qApp->topLevelWidgets()) {
      if (dynamic_cast<QMainWindow*>(w)) {
        reference = w;
        break;
      }
    }
  }

  if (!reference) {
    return;
  }

  MessageDialog* dialog = new MessageDialog(text, reference);
  dialog->show();

  if (bringToFront) {
    reference->activateWindow();
  }
}

}  // namespace GUI
