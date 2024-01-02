#ifndef BSPLUGINLIST_PLUGINSORTFILTERPROXYMODEL_H
#define BSPLUGINLIST_PLUGINSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace BSPluginList
{

class PluginSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  void hideForceEnabledFiles(bool doHide);

  [[nodiscard]] bool filterMatchesPlugin(const QString& plugin) const;

  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                       int column, const QModelIndex& parent) const override;

  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;

public slots:
  void updateFilter(const QString& filter);

private:
  QString m_CurrentFilter;
  bool m_HideForceEnabledFiles = false;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINSORTFILTERPROXYMODEL_H
