#include "FlagIconDelegate.h"

#include <bit>

namespace BSPluginList
{

using enum TESData::FileInfo::EFlag;

FlagIconDelegate::FlagIconDelegate(PluginListView* view)
    : GUI::IconDelegate(view), m_View{view}
{}

QList<QString> FlagIconDelegate::getIcons(const QModelIndex& index) const
{
  const auto flags = m_View->fileFlags(index);

  QList<QString> icons;

  if (flags & FLAG_PROBLEMATIC) {
    icons.append(":/MO/gui/warning");
  }

  if (flags & FLAG_INFORMATION) {
    icons.append(":/MO/gui/information");
  }

  if (flags & FLAG_INI) {
    icons.append(":/MO/gui/attachment");
  }

  if (flags & FLAG_BSA) {
    icons.append(":/MO/gui/archive_conflict_neutral");
  }

  if (flags & FLAG_MASTER) {
    icons.append(":/bsplugins/star");
  }

  if (flags & FLAG_LIGHT) {
    icons.append(":/bsplugins/feather");
  }

  if (flags & FLAG_MEDIUM) {
    icons.append(":/MO/gui/run");
    if (flags & FLAG_LIGHT) {
      icons.append(":/MO/gui/warning");
    }
  }

  if (flags & FLAG_BLUEPRINT) {
    icons.append(":/MO/gui/resources/go-down.png");
  }

  if (flags & FLAG_CLEAN) {
    icons.append(":/MO/gui/edit_clear");
  }

  return icons;
}

int FlagIconDelegate::getNumIcons(const QModelIndex& index) const
{
  return std::popcount(static_cast<uint>(m_View->fileFlags(index)));
}

}  // namespace BSPluginList
