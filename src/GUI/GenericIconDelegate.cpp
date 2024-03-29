#include "GenericIconDelegate.h"

namespace GUI
{

GenericIconDelegate::GenericIconDelegate(QTreeView* parent, int role, int logicalIndex,
                                         int compactSize)
    : IconDelegate(parent, logicalIndex, compactSize), m_Role{role}
{}

QList<QString> GenericIconDelegate::getIcons(const QModelIndex& index) const
{
  QList<QString> result;
  if (index.isValid()) {
    for (const QVariant& var : index.data(m_Role).toList()) {
      if (!compact() || !var.toString().isEmpty()) {
        result.append(var.toString());
      }
    }
  }
  return result;
}

int GenericIconDelegate::getNumIcons(const QModelIndex& index) const
{
  return index.data(m_Role).toList().count();
}

}  // namespace GUI
