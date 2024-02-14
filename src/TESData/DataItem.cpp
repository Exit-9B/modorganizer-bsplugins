#include "DataItem.h"

#include <algorithm>
#include <iterator>

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

QVariant DataItem::data(int fileIndex) const
{
  if (fileIndex < m_Data.size()) {
    return m_Data[fileIndex];
  }

  return QVariant();
}

QVariant DataItem::displayData(int fileIndex) const
{
  if (fileIndex < m_DisplayData.size()) {
    const auto& displayData = m_DisplayData[fileIndex];
    if (displayData.isValid()) {
      return displayData;
    }
  }

  return data(fileIndex);
}

bool DataItem::isLosingConflict(int fileIndex, int fileCount) const
{
  if (m_ConflictType == ConflictType::Ignore) {
    return false;
  }

  if (!m_Data.isEmpty()) {
    if (fileIndex >= m_Data.length()) {
      return false;
    } else if (fileCount > m_Data.length() &&
               m_ConflictType != ConflictType::NormalIgnoreEmpty) {
      return true;
    }

    for (int i = fileIndex + 1; i < m_Data.length(); ++i) {
      if (hasConflict(m_Data[i], m_Data[fileIndex])) {
        return true;
      }
    }
  }

  for (const auto& child : m_Children) {
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

  if (!m_Data.isEmpty()) {
    if (fileIndex >= m_Data.length() &&
        m_ConflictType != ConflictType::NormalIgnoreEmpty) {
      return true;
    }

    for (int i = 0; i < std::min(fileIndex, static_cast<int>(m_Data.length())); ++i) {
      if (hasConflict(m_Data[i], m_Data[fileIndex])) {
        return true;
      }
    }
  }

  for (const auto& child : m_Children) {
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

  if (!m_Data.isEmpty()) {
    if (fileCount > m_Data.length() &&
        m_ConflictType != ConflictType::NormalIgnoreEmpty) {
      return true;
    }

    for (int i = 1; i < m_Data.length(); ++i) {
      if (hasConflict(m_Data[i], m_Data[0])) {
        return true;
      }
    }
  }

  for (const auto& child : m_Children) {
    if (child->isConflicted(fileCount)) {
      return true;
    }
  }

  return false;
}

DataItem* DataItem::findChild(TESFile::Type signature) const
{
  const auto it = std::ranges::find_if(m_Children, [&](auto&& child) {
    return child->signature() == signature;
  });

  return it != std::end(m_Children) ? it->get() : nullptr;
}

QVariant DataItem::childData(TESFile::Type signature, int fileIndex) const
{
  const auto it = std::ranges::find_if(m_Children, [&](auto&& child) {
    return child->signature() == signature;
  });

  if (it == std::end(m_Children)) {
    return QVariant();
  }

  return (*it)->data(fileIndex);
}

QVariant DataItem::childData(const QString& name, int fileIndex) const
{
  const auto it = std::ranges::find_if(m_Children, [&](auto&& child) {
    return child->name() == name;
  });

  if (it == std::end(m_Children)) {
    return QVariant();
  }

  return (*it)->data(fileIndex);
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

void DataItem::setData(int fileIndex, const QVariant& data, bool caseSensitive)
{
  if (m_Data.size() <= fileIndex) {
    m_Data.resize(fileIndex + 1);
  }
  m_Data[fileIndex] = data;
  m_CaseSensitive   = caseSensitive;
}

void DataItem::setDisplayData(int fileIndex, const QVariant& data)
{
  if (m_DisplayData.size() <= fileIndex) {
    m_DisplayData.resize(fileIndex + 1);
  }
  m_DisplayData[fileIndex] = data;
}

bool DataItem::hasConflict(const QVariant& var1, const QVariant& var2) const
{
  if (!m_CaseSensitive && var1.userType() == QMetaType::QString &&
      var2.userType() == QMetaType::QString) {
    return var1.toString().compare(var2.toString(), Qt::CaseInsensitive) != 0;
  } else {
    return var1 != var2;
  }
}

}  // namespace TESData
