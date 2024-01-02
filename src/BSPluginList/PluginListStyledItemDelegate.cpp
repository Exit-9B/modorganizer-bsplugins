#include "PluginListStyledItemDelegate.h"
#include "PluginListView.h"

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

  const auto color    = m_View->markerColor(index);
  opt.backgroundBrush = color;

  QStyledItemDelegate::paint(painter, opt, index);
}

void PluginListStyledItemDelegate::initStyleOption(QStyleOptionViewItem* option,
                                                   const QModelIndex& index) const
{
  const auto backgroundColor = option->backgroundBrush.color();
  QStyledItemDelegate::initStyleOption(option, index);

  if (backgroundColor.isValid()) {
    option->backgroundBrush = backgroundColor;
  }
}

}  // namespace BSPluginList
