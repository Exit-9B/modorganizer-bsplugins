#ifndef GUI_VIEWMARKINGSCROLLBAR_H
#define GUI_VIEWMARKINGSCROLLBAR_H

#include <QColor>
#include <QModelIndex>
#include <QPaintEvent>
#include <QScrollBar>
#include <QTreeView>

namespace GUI
{

class ViewMarkingScrollBar : public QScrollBar
{
public:
  ViewMarkingScrollBar(QTreeView* view, int role);

protected:
  void paintEvent(QPaintEvent* event) override;

  // retrieve the color of the marker for the given index
  //
  [[nodiscard]] virtual QColor color(const QModelIndex& index) const;

protected:
  QTreeView* m_View;

private:
  int m_Role;
};

}  // namespace GUI

#endif  // GUI_VIEWMARKINGSCROLLBAR_H
