#include "ViewMarkingScrollBar.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOptionSlider>

namespace GUI
{

static QModelIndexList visibleIndexImpl(QTreeView* view, int column,
                                        const QModelIndex& parent)
{
  if (parent.isValid() && !view->isExpanded(parent)) {
    return {};
  }

  const auto* const model = view->model();
  QModelIndexList index;
  for (int i = 0; i < model->rowCount(parent); ++i) {
    index.append(model->index(i, column, parent));
    index.append(visibleIndexImpl(view, column, index.back()));
  }
  return index;
}

static QModelIndexList visibleIndex(QTreeView* view, int column)
{
  return visibleIndexImpl(view, column, QModelIndex());
}

ViewMarkingScrollBar::ViewMarkingScrollBar(QTreeView* view, int role)
    : QScrollBar(view), m_View{view}, m_Role{role}
{
  // not implemented for horizontal sliders
  Q_ASSERT(this->orientation() == Qt::Vertical);
}

QColor ViewMarkingScrollBar::color(const QModelIndex& index) const
{
  const auto data = index.data(m_Role);
  if (data.canConvert<QColor>()) {
    return data.value<QColor>();
  }
  return QColor();
}

void ViewMarkingScrollBar::paintEvent(QPaintEvent* event)
{
  if (m_View->model() == nullptr) {
    return;
  }
  QScrollBar::paintEvent(event);

  QStyleOptionSlider styleOption;
  initStyleOption(&styleOption);

  QPainter painter(this);

  QRect handleRect = style()->subControlRect(QStyle::CC_ScrollBar, &styleOption,
                                             QStyle::SC_ScrollBarSlider, this);
  QRect innerRect  = style()->subControlRect(QStyle::CC_ScrollBar, &styleOption,
                                             QStyle::SC_ScrollBarGroove, this);

  auto indices = visibleIndex(m_View, 0);

  painter.translate(innerRect.topLeft() + QPoint(0, 3));
  const qreal scale =
      static_cast<qreal>(innerRect.height() - 3) / static_cast<qreal>(indices.size());

  for (int i = 0; i < indices.size(); ++i) {
    const QColor color = this->color(indices[i]);
    if (color.isValid()) {
      painter.setPen(color);
      painter.setBrush(color);
      painter.drawRect(QRect(2, i * scale - 2, handleRect.width() - 5, 3));
    }
  }
}

}  // namespace GUI
