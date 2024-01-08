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

  const auto color    = m_View->markerColor(index);
  opt.backgroundBrush = color;

  const auto widget = opt.widget;
  const auto style  = widget ? widget->style() : QApplication::style();

  // HACK: we can't normally have a disabled checkbox on a selectable row, so create a
  // margin for it and then draw one manually
  bool drawCheck = false;
  bool enabled   = false;
  if (index.column() == 0 && !index.data(Qt::CheckStateRole).isValid()) {
    const auto plugin =
        index.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();

    if (plugin) {
      drawCheck = true;
      if (plugin->forceLoaded() || plugin->forceEnabled()) {
        enabled = true;
      }
    }

    // set decoration size to use as text margin
    opt.decorationSize.setWidth(
        style->pixelMetric(QStyle::PM_IndicatorWidth, &opt, widget));
  }

  QStyledItemDelegate::paint(painter, opt, index);

  // draw check on top
  if (drawCheck) {
    opt.features |= QStyleOptionViewItem::HasCheckIndicator;

    QRect checkRect =
        style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &opt, widget);
    opt.rect = checkRect;

    opt.state &= ~QStyle::State_Enabled;
    if (enabled) {
      opt.state |= QStyle::State_On;
    }

    style->drawPrimitive(QStyle::PE_IndicatorItemViewItemCheck, &opt, painter, widget);
  }
}

void PluginListStyledItemDelegate::initStyleOption(QStyleOptionViewItem* option,
                                                   const QModelIndex& index) const
{
  const auto backgroundColor = option->backgroundBrush.color();
  QStyledItemDelegate::initStyleOption(option, index);

  // HACK: create a text margin where the checkbox should be
  if (index.column() == 0 && !index.data(Qt::CheckStateRole).isValid()) {
    if (index.data(PluginListModel::InfoRole).isValid()) {
      option->features |= QStyleOptionViewItem::HasDecoration;
    }
  }

  if (backgroundColor.isValid()) {
    option->backgroundBrush = backgroundColor;
  }
}

}  // namespace BSPluginList
