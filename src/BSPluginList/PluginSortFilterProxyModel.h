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

  bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row,
                       int column, const QModelIndex& parent) const override;

protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;

private:
  bool m_HideForceEnabledFiles = false;
};

}  // namespace BSPluginList

#endif  // BSPLUGINLIST_PLUGINSORTFILTERPROXYMODEL_H
