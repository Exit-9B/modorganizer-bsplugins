#ifndef GUI_SELECTIONDIALOG_H
#define GUI_SELECTIONDIALOG_H

#include <QAbstractButton>
#include <QDialog>

namespace Ui
{
class SelectionDialog;
}

namespace GUI
{

class SelectionDialog : public QDialog
{
  Q_OBJECT

public:
  explicit SelectionDialog(const QString& description, QWidget* parent = 0,
                           const QSize& iconSize = QSize());

  SelectionDialog(const SelectionDialog&) = delete;
  SelectionDialog(SelectionDialog&&)      = delete;

  ~SelectionDialog() noexcept;

  SelectionDialog& operator=(const SelectionDialog&) = delete;
  SelectionDialog& operator=(SelectionDialog&&)      = delete;

  /**
   * @brief add a choice to the dialog
   * @param buttonText the text to be displayed on the button
   * @param description the description that shows up under in small letters inside the
   * button
   * @param data data to be stored with the button. Please note that as soon as one
   * choice has data associated with it (non-invalid QVariant) all buttons that contain
   * no data will be treated as "cancel" buttons
   */
  void addChoice(const QString& buttonText, const QString& description,
                 const QVariant& data);

  void addChoice(const QIcon& icon, const QString& buttonText,
                 const QString& description, const QVariant& data);

  int numChoices() const;

  QVariant getChoiceData();
  QString getChoiceString();
  QString getChoiceDescription();

  void disableCancel();

private slots:

  void on_buttonBox_clicked(QAbstractButton* button);

  void on_cancelButton_clicked();

private:
  Ui::SelectionDialog* ui;
  QAbstractButton* m_Choice;
  bool m_ValidateByData;
  QSize m_IconSize;
};

}  // namespace GUI

#endif  // GUI_SELECTIONDIALOG_H
