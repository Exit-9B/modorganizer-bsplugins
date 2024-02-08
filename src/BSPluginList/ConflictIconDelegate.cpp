#include "ConflictIconDelegate.h"

namespace BSPluginList
{

using enum TESData::FileInfo::EConflictFlag;

ConflictIconDelegate::ConflictIconDelegate(PluginListView* view)
    : GUI::IconDelegate(view), m_View{view}
{}

QList<QString> ConflictIconDelegate::getIcons(const QModelIndex& index) const
{
  const auto flags = m_View->conflictFlags(index);

  QList<QString> icons;

  if ((flags & CONFLICT_MIXED) == CONFLICT_MIXED) {
    icons.append(":/MO/gui/emblem_conflict_mixed");
  } else if (flags & CONFLICT_OVERRIDE) {
    icons.append(":/MO/gui/emblem_conflict_overwrite");
  } else if (flags & CONFLICT_OVERRIDDEN) {
    icons.append(":/MO/gui/emblem_conflict_overwritten");
  }

  if ((flags & CONFLICT_ARCHIVE_MIXED) == CONFLICT_ARCHIVE_MIXED) {
    icons.append(":/MO/gui/archive_conflict_mixed");
  } else if (flags & CONFLICT_ARCHIVE_OVERWRITE) {
    icons.append(":/MO/gui/archive_conflict_winner");
  } else if (flags & CONFLICT_ARCHIVE_OVERWRITTEN) {
    icons.append(":/MO/gui/archive_conflict_loser");
  }

  return icons;
}

[[nodiscard]] static int numIcons(uint flags)
{
  return ((flags & CONFLICT_MIXED) ? 1 : 0) +
         ((flags & CONFLICT_ARCHIVE_MIXED) ? 1 : 0);
}

int ConflictIconDelegate::getNumIcons(const QModelIndex& index) const
{
  return numIcons(m_View->conflictFlags(index));
}

}  // namespace BSPluginList
