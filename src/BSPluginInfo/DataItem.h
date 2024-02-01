#ifndef BSPLUGININFO_DATAITEM_H
#define BSPLUGININFO_DATAITEM_H

#include "TESFile/Type.h"

#include <QList>
#include <QString>
#include <QVariant>

#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>

namespace BSPluginInfo
{

class DataItem final
{
public:
  DataItem() : m_Parent{nullptr} {}

  DataItem(TESFile::Type signature, const QString& name, DataItem* parent)
      : m_Signature{signature}, m_Name{name}, m_Parent{parent}
  {}

  [[nodiscard]] TESFile::Type signature() const { return m_Signature; }
  [[nodiscard]] QString name() const { return m_Name; }
  [[nodiscard]] DataItem* parent() const { return m_Parent; }
  [[nodiscard]] int numChildren() const { return static_cast<int>(m_Children.size()); }
  [[nodiscard]] DataItem* childAt(int index) const { return m_Children[index].get(); }
  [[nodiscard]] int index() const { return m_Parent ? m_Parent->indexOf(this) : 0; }

  [[nodiscard]] QVariant rowHeader() const
  {
    const auto view = m_Signature.view();
    if (!m_Name.isEmpty()) {
      return QStringLiteral("%1 - %2").arg(
          QString::fromLocal8Bit(view.data(), view.size()), m_Name);
    } else {
      return QString::fromLocal8Bit(view.data(), view.size());
    }
  }

  [[nodiscard]] QVariant displayData(int index) const
  {
    if (index < m_DisplayData.size()) {
      return m_DisplayData[index];
    }

    return QVariant();
  }

  [[nodiscard]] int indexOf(const DataItem* child) const
  {
    const auto it =
        std::ranges::find(m_Children, child, &std::shared_ptr<DataItem>::get);
    if (it == std::end(m_Children)) {
      return -1;
    }
    return std::distance(std::begin(m_Children), it);
  }

  DataItem* insertChild(int index, TESFile::Type signature, const QString& name)
  {
    const auto it = m_Children.insert(
        m_Children.begin() + index, std::make_shared<DataItem>(signature, name, this));
    return it->get();
  }

  DataItem* getOrInsertChild(int index, TESFile::Type signature, const QString& name)
  {
    if (index < m_Children.size()) {
      const auto& child = m_Children[index];
      if (child->signature() == signature) {
        return child.get();
      }
    }
    return insertChild(index, signature, name);
  }

  void setDisplayData(int index, const QVariant& data)
  {
    if (m_DisplayData.size() <= index) {
      m_DisplayData.resize(index + 1);
    }
    m_DisplayData[index] = data;
  }

private:
  TESFile::Type m_Signature;
  QString m_Name;
  QList<QVariant> m_DisplayData;
  DataItem* m_Parent;
  std::vector<std::shared_ptr<DataItem>> m_Children;
};

}  // namespace BSPluginInfo

#endif  // BSPLUGININFO_DATAITEM_H
