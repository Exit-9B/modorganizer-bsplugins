#include "CopyEventFilter.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QKeyEvent>

namespace GUI
{

CopyEventFilter::CopyEventFilter(QAbstractItemView* view, int column, int role)
    : CopyEventFilter(view, [=](const auto& index) {
        return index.sibling(index.row(), column).data(role).toString();
      })
{}

CopyEventFilter::CopyEventFilter(QAbstractItemView* view,
                                 std::function<QString(const QModelIndex&)> format)
    : QObject(view), m_View{view}, m_Format{format}
{}

void CopyEventFilter::copySelection() const
{
  if (!m_View->selectionModel()->hasSelection()) {
    return;
  }

  // sort to reflect the visual order
  QModelIndexList selectedRows = m_View->selectionModel()->selectedRows();
  std::ranges::sort(selectedRows, [this](const auto& lidx, const auto& ridx) {
    return m_View->visualRect(lidx).top() < m_View->visualRect(ridx).top();
  });

  QStringList rows;
  for (const auto& idx : selectedRows) {
    rows.append(m_Format(idx));
  }

  QGuiApplication::clipboard()->setText(rows.join("\n"));
}

bool CopyEventFilter::eventFilter(QObject* sender, QEvent* event)
{
  if (sender == m_View && event->type() == QEvent::KeyPress) {
    QKeyEvent* const keyEvent = static_cast<QKeyEvent*>(event);
    if (keyEvent->modifiers() == Qt::ControlModifier && keyEvent->key() == Qt::Key_C) {
      copySelection();
      return true;
    }
  }
  return QObject::eventFilter(sender, event);
}

}  // namespace GUI
