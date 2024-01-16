#ifndef GUI_LISTDIALOG_H
#define GUI_LISTDIALOG_H

#include "IGeometrySettings.h"

#include <QDialog>

namespace Ui
{
class ListDialog;
}

namespace GUI
{

class ListDialog : public QDialog
{
  Q_OBJECT

public:
  ListDialog(IGeometrySettings<QDialog>& settings, QWidget* parent = nullptr);
  ~ListDialog() noexcept;

  // also saves and restores geometry
  //
  int exec() override;

  void setChoices(QStringList choices);
  [[nodiscard]] QString getChoice() const;

public slots:
  void on_filterEdit_textChanged(QString filter);

private:
  Ui::ListDialog* ui;
  IGeometrySettings<QDialog>& m_Settings;
  QStringList m_Choices;
};

}  // namespace GUI

#endif  // GUI_LISTDIALOG_H
