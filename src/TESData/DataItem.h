#ifndef TESDATA_DATAITEM_H
#define TESDATA_DATAITEM_H

#include "TESFile/Type.h"

#include <QList>
#include <QString>
#include <QVariant>

#include <algorithm>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

namespace TESData
{

class DataItem final
{
public:
  enum class ConflictType
  {
    Ignore,
    BenignIfAdded,
    Benign,
    Override,
    Translate,
    NormalIgnoreEmpty,
    Critical,
    FormID,
  };

  DataItem() : m_Parent{nullptr} {}

  DataItem(DataItem* parent, const QString& name, ConflictType conflictType)
      : m_ConflictType{conflictType}, m_Name{name}, m_Parent{parent}
  {}

  DataItem(DataItem* parent, TESFile::Type signature, const QString& name,
           ConflictType conflictType)
      : m_ConflictType{conflictType}, m_Signature{signature},
        m_Name{makeName(signature, name)}, m_Parent{parent}
  {}

  [[nodiscard]] static QString makeName(TESFile::Type signature, const QString& name);

  [[nodiscard]] ConflictType conflictType() const { return m_ConflictType; }
  [[nodiscard]] TESFile::Type signature() const { return m_Signature; }
  [[nodiscard]] QString name() const { return m_Name; }
  [[nodiscard]] DataItem* parent() const { return m_Parent; }
  [[nodiscard]] int numChildren() const { return static_cast<int>(m_Children.size()); }
  [[nodiscard]] DataItem* childAt(int index) const { return m_Children[index].get(); }
  [[nodiscard]] int index() const { return m_Parent ? m_Parent->indexOf(this) : 0; }

  [[nodiscard]] QVariant rowHeader() const { return name(); }

  [[nodiscard]] QVariant displayData(int fileIndex) const;

  [[nodiscard]] bool isLosingConflict(int fileIndex, int fileCount) const;
  [[nodiscard]] bool isOverriding(int fileIndex) const;
  [[nodiscard]] bool isConflicted(int fileCount) const;

  [[nodiscard]] QVariant childData(TESFile::Type signature, int fileIndex) const;
  [[nodiscard]] QVariant childData(const QString& name, int fileIndex) const;
  [[nodiscard]] int indexOf(const DataItem* child) const;

  template <typename... Args>
  DataItem* insertChild(int index, Args&&... args)
  {
    const auto it = m_Children.insert(
        m_Children.begin() + index,
        std::make_shared<DataItem>(this, std::forward<Args>(args)...));
    return it->get();
  }

  DataItem* getOrInsertChild(int index, const QString& name,
                             ConflictType conflictType = ConflictType::Override);
  DataItem* getOrInsertChild(int index, TESFile::Type signature, const QString& name,
                             ConflictType conflictType = ConflictType::Override);

  void setDisplayData(int fileIndex, const QVariant& data);

private:
  ConflictType m_ConflictType{ConflictType::Override};
  TESFile::Type m_Signature{};
  QString m_Name;
  QList<QVariant> m_DisplayData;
  DataItem* m_Parent;
  std::vector<std::shared_ptr<DataItem>> m_Children;
};

}  // namespace TESData

#endif  // BSPLUGININFO_DATAITEM_H
