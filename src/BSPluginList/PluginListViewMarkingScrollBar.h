#ifndef BSPLUGINLIST_PLUGINLISTVIEWMARKINGSCROLLBAR_H
#define BSPLUGINLIST_PLUGINLISTVIEWMARKINGSCROLLBAR_H

#include "GUI/ViewMarkingScrollBar.h"
#include "PluginListView.h"

namespace BSPluginList
{

class PluginListViewMarkingScrollBar : public GUI::ViewMarkingScrollBar
{
public:
  explicit PluginListViewMarkingScrollBar(PluginListView* view)
      : GUI::ViewMarkingScrollBar(view, PluginListModel::ScrollMarkRole)
  {}

  [[nodiscard]] QColor color(const QModelIndex& index) const override
  {
    auto color = static_cast<PluginListView*>(m_View)->markerColor(index);
    if (!color.isValid()) {
      color = ViewMarkingScrollBar::color(index);
    }
    return color;
  }
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINLISTVIEWMARKINGSCROLLBAR_H
