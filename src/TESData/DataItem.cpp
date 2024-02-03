#include "DataItem.h"

using namespace Qt::Literals::StringLiterals;

namespace TESData
{

QString DataItem::makeName(TESFile::Type signature, const QString& name)
{
  const QByteArray sig =
      QByteArray(signature.data(), signature.size()).toPercentEncoding("@"_ba);

  if (!name.isEmpty()) {
    return QStringLiteral("%1 - %2").arg(QString::fromLatin1(sig), name);
  } else {
    return QString::fromLatin1(sig);
  }
}

QVariant DataItem::displayData(int fileIndex) const
{
  if (fileIndex < m_DisplayData.size()) {
    return m_DisplayData[fileIndex];
  }

  return QVariant();
}

bool DataItem::isLosingConflict(int fileIndex, int fileCount) const
{
  if (m_ConflictType == ConflictType::Ignore) {
    return false;
  }

  if (!m_DisplayData.isEmpty()) {
    if (fileCount > m_DisplayData.length()) {
      return true;
    }

    for (int i = fileIndex + 1; i < m_DisplayData.length(); ++i) {
      if (m_DisplayData[i] != m_DisplayData[fileIndex]) {
        return true;
      }
    }
  }

  for (auto& child : m_Children) {
    if (child->isLosingConflict(fileIndex, fileCount)) {
      return true;
    }
  }

  return false;
}

bool DataItem::isOverriding(int fileIndex) const
{
  if (m_ConflictType == ConflictType::Ignore) {
    return false;
  }

  if (!m_DisplayData.isEmpty()) {
    if (fileIndex > m_DisplayData.length()) {
      return true;
    }

    for (int i = 0; i < fileIndex; ++i) {
      if (m_DisplayData[i] != m_DisplayData[fileIndex]) {
        return true;
      }
    }
  }

  for (auto& child : m_Children) {
    if (child->isOverriding(fileIndex)) {
      return true;
    }
  }

  return false;
}

bool DataItem::isConflicted(int fileCount) const
{
  if (m_ConflictType == ConflictType::Ignore) {
    return false;
  }

  if (!m_DisplayData.isEmpty()) {
    if (fileCount > m_DisplayData.length()) {
      return true;
    }

    for (int i = 1; i < m_DisplayData.length(); ++i) {
      if (m_DisplayData[i] != m_DisplayData[0]) {
        return true;
      }
    }
  }

  for (auto& child : m_Children) {
    if (child->isConflicted(fileCount)) {
      return true;
    }
  }

  return false;
}

QVariant DataItem::childData(TESFile::Type signature, int fileIndex) const
{
  const auto it = std::ranges::find_if(m_Children, [&](auto&& child) {
    return child->signature() == signature;
  });

  if (it == std::end(m_Children)) {
    return QVariant();
  }

  return (*it)->displayData(fileIndex);
}

QVariant DataItem::childData(const QString& name, int fileIndex) const
{
  const auto it = std::ranges::find_if(m_Children, [&](auto&& child) {
    return child->name() == name;
  });

  if (it == std::end(m_Children)) {
    return QVariant();
  }

  return (*it)->displayData(fileIndex);
}

int DataItem::indexOf(const DataItem* child) const
{
  const auto it = std::ranges::find(m_Children, child, &std::shared_ptr<DataItem>::get);
  if (it == std::end(m_Children)) {
    return -1;
  }
  return std::distance(std::begin(m_Children), it);
}

DataItem* DataItem::getOrInsertChild(int index, const QString& name,
                                     ConflictType conflictType)
{
  if (index < m_Children.size()) {
    const auto& child = m_Children[index];
    if (child->name() == name) {
      return child.get();
    }
  }
  return insertChild(index, name, conflictType);
}

DataItem* DataItem::getOrInsertChild(int index, TESFile::Type signature,
                                     const QString& name, ConflictType conflictType)
{
  if (index < m_Children.size()) {
    const auto& child = m_Children[index];
    if (child->signature() == signature) {
      return child.get();
    }
  }
  return insertChild(index, signature, name, conflictType);
}

void DataItem::setDisplayData(int fileIndex, const QVariant& data)
{
  if (m_DisplayData.size() <= fileIndex) {
    m_DisplayData.resize(fileIndex + 1);
  }
  m_DisplayData[fileIndex] = data;
}

}  // namespace TESData
