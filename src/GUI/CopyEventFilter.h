#ifndef GUI_COPY_EVENT_FILTER_H
#define GUI_COPY_EVENT_FILTER_H

#include <QAbstractItemView>
#include <QModelIndex>
#include <QObject>

#include <functional>

namespace GUI
{

// this small class provides copy on Ctrl+C and also
// exposes a method to actual copy the selection
//
// the way the selection is copied can be customized by
// passing a functor to format each index, by default
// it only extracts the display role
//
// only works for view that selects whole row since it only
// considers the first cell in each row
//
class CopyEventFilter : public QObject
{
  Q_OBJECT

public:
  explicit CopyEventFilter(QAbstractItemView* view, int column = 0,
                           int role = Qt::DisplayRole);
  CopyEventFilter(QAbstractItemView* view,
                  std::function<QString(const QModelIndex&)> format);

  // copy the selection of the view associated with this
  // event filter into the clipboard
  //
  void copySelection() const;

  bool eventFilter(QObject* sender, QEvent* event) override;

private:
  QAbstractItemView* m_View;
  std::function<QString(const QModelIndex&)> m_Format;
};

}  // namespace GUI

#endif  // GUI_COPY_EVENT_FILTER_H
