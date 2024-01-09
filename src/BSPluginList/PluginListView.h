#ifndef BSPLUGINLIST_PLUGINLISTVIEW_H
#define BSPLUGINLIST_PLUGINLISTVIEW_H

#include <QSet>
#include <QTreeView>

namespace BSPluginList
{

struct MarkerInfos
{
  QSet<uint> overriding;
  QSet<uint> overridden;
  QSet<uint> overwritingAux;
  QSet<uint> overwrittenAux;
  QSet<uint> highlight;
};

class PluginListModel;
class PluginSortFilterProxyModel;

class PluginListView final : public QTreeView
{
  Q_OBJECT

public:
  explicit PluginListView(QWidget* parent = nullptr);

  void setup();

  void setModel(QAbstractItemModel* model) override;

  [[nodiscard]] QColor markerColor(const QModelIndex& index) const;

public slots:
  void setHighlightedOrigins(const QStringList& origins);
  void clearOverwriteMarkers();
  void updateOverwriteMarkers();

signals:
  void openOriginExplorer(const QModelIndex& index);

protected:
  bool event(QEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

  bool moveSelection(int key);
  bool toggleSelectionState();

private:
  friend class PluginListContextMenu;

  QModelIndex indexViewToModel(const QModelIndex& index,
                               const QAbstractItemModel* model) const;

  QModelIndexList indexViewToModel(const QModelIndexList& indices,
                                   const QAbstractItemModel* model) const;

private:
  bool m_FirstPaint = true;
  MarkerInfos m_Markers;
  PluginListModel* m_PluginModel;
  PluginSortFilterProxyModel* m_SortProxy;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINLISTVIEW_H
