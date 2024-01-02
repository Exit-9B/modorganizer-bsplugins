#ifndef BSPLUGINLIST_PLUGINLISTSTYLEDITEMDELEGATE_H
#define BSPLUGINLIST_PLUGINLISTSTYLEDITEMDELEGATE_H

#include <QStyledItemDelegate>

namespace BSPluginList
{

class PluginListView;

class PluginListStyledItemDelegate final : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit PluginListStyledItemDelegate(PluginListView* view);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

protected:
  void initStyleOption(QStyleOptionViewItem* option,
                       const QModelIndex& index) const override;

private:
  PluginListView* m_View;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINLISTSTYLEDITEMDELEGATE_H
