#include "PluginListStyledItemDelegate.h"
#include "PluginListModel.h"
#include "PluginListView.h"

#include <QApplication>

namespace BSPluginList
{

PluginListStyledItemDelegate::PluginListStyledItemDelegate(PluginListView* view)
    : QStyledItemDelegate(view), m_View{view}
{}

void PluginListStyledItemDelegate::paint(QPainter* painter,
                                         const QStyleOptionViewItem& option,
                                         const QModelIndex& index) const
{
  QStyleOptionViewItem opt(option);

  if (index.column() == 0) {
    if (!index.model()->hasChildren(index) || !index.data().isValid()) {
      opt.rect.adjust(-m_View->indentation(), 0, 0, 0);
    }
  }

  const auto color    = m_View->markerColor(index);
  opt.backgroundBrush = color;

  if (!index.siblingAtColumn(0).data().isValid()) {
    const auto widget = option.widget;
    const auto style  = widget ? widget->style() : QApplication::style();

    QStyleOptionFrame optFrame;
    optFrame.initFrom(widget);
    optFrame.rect         = opt.rect;
    optFrame.frameShape   = QFrame::HLine;
    optFrame.lineWidth    = 0;
    optFrame.midLineWidth = 1;
    optFrame.state |= QStyle::State_Sunken;

    style->drawControl(QStyle::CE_ShapedFrame, &optFrame, painter, widget);
  } else {
    QStyledItemDelegate::paint(painter, opt, index);
  }
}

void PluginListStyledItemDelegate::initStyleOption(QStyleOptionViewItem* option,
                                                   const QModelIndex& index) const
{
  const auto backgroundColor = option->backgroundBrush.color();
  QStyledItemDelegate::initStyleOption(option, index);

  // HACK: we can't normally have a disabled checkbox on a selectable row, so override
  // the style to make it look disabled
  if (index.column() == 0) {
    const auto plugin =
        index.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();

    if (plugin && !plugin->canBeToggled()) {
      option->state &= ~QStyle::State_Enabled;
    }
  }

  if (backgroundColor.isValid()) {
    option->backgroundBrush = backgroundColor;
  }
}

}  // namespace BSPluginList
