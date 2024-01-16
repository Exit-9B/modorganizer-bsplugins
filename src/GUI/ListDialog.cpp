#include "ListDialog.h"
#include "ui_listdialog.h"

namespace GUI
{

ListDialog::ListDialog(IGeometrySettings<QDialog>& settings, QWidget* parent)
    : QDialog(parent), ui{new Ui::ListDialog}, m_Settings{settings}
{
  ui->setupUi(this);
  ui->filterEdit->setFocus();
  connect(ui->choiceList, &QListWidget::itemDoubleClicked, this, &QDialog::accept);
}

ListDialog::~ListDialog() noexcept
{
  delete ui;
}

int ListDialog::exec()
{
  GeometrySaver gs{m_Settings, this};
  return QDialog::exec();
}

void ListDialog::setChoices(QStringList choices)
{
  m_Choices = choices;
  ui->choiceList->clear();
  ui->choiceList->addItems(m_Choices);
}

QString ListDialog::getChoice() const
{
  if (ui->choiceList->selectedItems().length()) {
    return ui->choiceList->currentItem()->text();
  } else {
    return "";
  }
}

void ListDialog::on_filterEdit_textChanged(QString filter)
{
  QStringList newChoices;
  for (auto choice : m_Choices) {
    if (choice.contains(filter, Qt::CaseInsensitive)) {
      newChoices << choice;
    }
  }
  ui->choiceList->clear();
  ui->choiceList->addItems(newChoices);

  if (newChoices.length() == 1) {
    QListWidgetItem* item = ui->choiceList->item(0);
    item->setSelected(true);
    ui->choiceList->setCurrentItem(item);
  }

  if (!filter.isEmpty()) {
    ui->choiceList->setStyleSheet("QListWidget { border: 2px ridge #f00; }");
  } else {
    ui->choiceList->setStyleSheet("");
  }
}

}  // namespace GUI
