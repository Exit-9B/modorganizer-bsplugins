#ifndef GUI_ICONDELEGATE_H
#define GUI_ICONDELEGATE_H

#include <QAbstractItemView>
#include <QAbstractProxyModel>
#include <QStyledItemDelegate>
#include <QTreeView>

namespace GUI
{

class IconDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit IconDelegate(QTreeView* view, int column = -1, int compactSize = 100);

  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;

  QSize sizeHint(const QStyleOptionViewItem& option,
                 const QModelIndex& index) const override;

protected:
  // check if icons should be compacted or not
  //
  [[nodiscard]] bool compact() const { return m_Compact; }

  static void paintIcons(QPainter* painter, const QStyleOptionViewItem& option,
                         const QModelIndex& index, const QList<QString>& icons);

  [[nodiscard]] virtual QList<QString> getIcons(const QModelIndex& index) const = 0;
  [[nodiscard]] virtual size_t getNumIcons(const QModelIndex& index) const      = 0;

private:
  int m_Column;
  int m_CompactSize;
  bool m_Compact;
};

}  // namespace GUI

#endif  // GUI_ICONDELEGATE_H
