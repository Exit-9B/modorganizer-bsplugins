#include "IconDelegate.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPainter>
#include <QPixmapCache>
#include <log.h>

namespace GUI
{

using namespace MOBase;

constexpr int MAX_WIDTH = 16;
constexpr int MARGIN_X  = 4;

IconDelegate::IconDelegate(QTreeView* view, int column, int compactSize)
    : QStyledItemDelegate(view), m_Column{column}, m_CompactSize{compactSize},
      m_Compact{false}
{
  if (view) {
    connect(view->header(), &QHeaderView::sectionResized,
            [=](int logicalIndex, [[maybe_unused]] int oldSize, int newSize) {
              if (logicalIndex == m_Column) {
                m_Compact = newSize < m_CompactSize;
              }
            });
  }
}

void IconDelegate::paintIcons(QPainter* painter, const QStyleOptionViewItem& option,
                              [[maybe_unused]] const QModelIndex& index,
                              const QList<QString>& icons)
{
  int x = MARGIN_X;
  painter->save();

  int iconWidth =
      icons.size() > 0 ? ((option.rect.width() / icons.size()) - MARGIN_X) : MAX_WIDTH;
  iconWidth = std::min(MAX_WIDTH, iconWidth);

  const int margin = (option.rect.height() - iconWidth) / 2;

  painter->translate(option.rect.topLeft());
  for (const QString& iconId : icons) {
    if (iconId.isEmpty()) {
      x += iconWidth + MARGIN_X;
      continue;
    }
    QPixmap icon;
    const QString fullIconId = QString("%1_%2").arg(iconId).arg(iconWidth);
    if (!QPixmapCache::find(fullIconId, &icon)) {
      icon = QIcon(iconId).pixmap(iconWidth, iconWidth);
      if (icon.isNull()) {
        log::warn("failed to load icon {}", iconId);
      }
      QPixmapCache::insert(fullIconId, icon);
    }
    painter->drawPixmap(x, margin, iconWidth, iconWidth, icon);
    x += iconWidth + MARGIN_X;
  }

  painter->restore();
}

void IconDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const
{
  if (auto* const w = qobject_cast<QAbstractItemView*>(parent())) {
    w->itemDelegate()->paint(painter, option, index);
  } else {
    QStyledItemDelegate::paint(painter, option, index);
  }

  paintIcons(painter, option, index, getIcons(index));
}

QSize IconDelegate::sizeHint(const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
  return QSize(getNumIcons(index) * (MAX_WIDTH + MARGIN_X), option.rect.height());
}

}  // namespace GUI
