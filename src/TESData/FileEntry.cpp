#include "FileEntry.h"

#include <algorithm>
#include <iterator>
#include <utility>

namespace TESData
{

FileEntry::FileEntry(TESFileHandle handle, const std::string& name)
    : m_Handle{handle}, m_Name{name}, m_Root{std::make_shared<TreeItem>()}
{}

void FileEntry::forEachRecord(
    std::function<void(const std::shared_ptr<const Record>&)> func) const
{
  std::vector<std::shared_ptr<TreeItem>> stack;
  stack.push_back(m_Root);
  while (!stack.empty()) {
    const auto item = std::move(stack.back());
    stack.pop_back();

    if (item->record) {
      func(item->record);
    }

    for (const auto& [identifier, child] : item->children) {
      stack.push_back(child);
    }
  }
}

std::shared_ptr<Record> FileEntry::createRecord(const RecordPath& path,
                                                const std::string& name,
                                                TESFile::Type formType)
{
  const auto item = createHierarchy(path);

  std::unique_lock lk{m_Mutex};

  if (!item->record) {
    item->record = std::make_shared<Record>();
    lk.unlock();

    item->record->setIdentifier(path.identifier(), path.files());
    item->record->addAlternative(m_Handle);
    item->name     = name;
    item->formType = formType;
  }

  return item->record;
}

void FileEntry::addRecord(const RecordPath& path, const std::string& name,
                          TESFile::Type formType, std::shared_ptr<Record> record)
{
  record->addAlternative(m_Handle);
  const auto item = createHierarchy(path);
  item->record    = record;
  item->name      = name;
  item->formType  = formType;
}

void FileEntry::addChildGroup(const RecordPath& path)
{
  const auto item = findItem(path);
  if (!item || !item->record) {
    // no record to add children to
    return;
  }

  TESFile::GroupData group = path.groups().back();
  if (group.hasParent()) {
    const auto& file            = path.files()[group.parent() >> 24];
    const std::uint8_t newIndex = static_cast<std::uint8_t>(
        std::distance(std::begin(m_Files), TESFile::find(m_Files, file)));

    if (newIndex == m_Files.size()) {
      m_Files.push_back(file);
    }

    group.setLocalIndex(newIndex);
  }

  item->group = group;
}

std::shared_ptr<Record> FileEntry::findRecord(const RecordPath& path) const
{
  const auto item = findItem(path);
  return item ? item->record : nullptr;
}

std::shared_ptr<FileEntry::TreeItem> FileEntry::findItem(const RecordPath& path) const
{
  std::shared_lock lk{m_Mutex};

  const auto groups = path.groups();
  auto item         = m_Root;
  for (TESFile::GroupData group : groups) {
    if (group.hasParent()) {
      const auto& file            = path.files()[group.parent() >> 24];
      const std::uint8_t newIndex = static_cast<std::uint8_t>(
          std::distance(std::begin(m_Files), TESFile::find(m_Files, file)));

      if (newIndex == m_Files.size()) {
        return nullptr;
      }

      group.setLocalIndex(newIndex);
    }

    if (group.hasDirectParent() &&
        (!item->record || item->record->formId() != group.parent())) {
      if (const auto it = item->children.find(group.parent());
          it != item->children.end()) {
        item = it->second;
      } else {
        return nullptr;
      }
    } else {
      if (const auto it = item->children.find(group); it != item->children.end()) {
        item = it->second;
      } else {
        return nullptr;
      }
    }
  }

  TreeItem::Key key;
  if (path.hasFormId()) {
    const auto& file            = path.files()[path.formId() >> 24];
    const std::uint8_t newIndex = static_cast<std::uint8_t>(
        std::distance(std::begin(m_Files), TESFile::find(m_Files, file)));

    if (newIndex == m_Files.size()) {
      return nullptr;
    }

    const std::uint32_t formId = (path.formId() & 0xFFFFFFU) | (newIndex << 24U);
    key                        = formId;
  } else if (path.hasEditorId()) {
    key = path.editorId();
  } else if (path.hasTypeId()) {
    key = path.typeId();
  } else {
    return item;
  }

  const auto it = item->children.find(key);
  return it != item->children.end() ? it->second : nullptr;
}

std::shared_ptr<FileEntry::TreeItem> FileEntry::createHierarchy(const RecordPath& path)
{
  std::unique_lock lk{m_Mutex};

  const auto groups = path.groups();
  auto item         = m_Root;
  for (TESFile::GroupData group : groups) {
    if (group.hasParent()) {
      const auto& file            = path.files()[group.parent() >> 24];
      const std::uint8_t newIndex = static_cast<std::uint8_t>(
          std::distance(std::begin(m_Files), TESFile::find(m_Files, file)));

      if (newIndex == m_Files.size()) {
        m_Files.push_back(file);
      }

      group.setLocalIndex(newIndex);
    }

    if (group.hasDirectParent() &&
        (!item->record || item->record->formId() != group.parent())) {
      auto& nextItem = item->children[group.parent()];
      if (!nextItem) {
        nextItem         = std::make_shared<TreeItem>();
        nextItem->parent = item.get();
        nextItem->record = std::make_shared<Record>();
        nextItem->record->setIdentifier(group.parent(), m_Files);
      }
      nextItem->group = group;

      item = nextItem;
    } else {
      auto& nextItem = item->children[group];
      if (!nextItem) {
        nextItem         = std::make_shared<TreeItem>();
        nextItem->parent = item.get();
        nextItem->group  = group;
      }
      item = nextItem;
    }
  }

  TreeItem::Key key;
  if (path.hasFormId()) {
    const auto& file            = path.files()[path.formId() >> 24];
    const std::uint8_t newIndex = static_cast<std::uint8_t>(
        std::distance(std::begin(m_Files), TESFile::find(m_Files, file)));

    if (newIndex == m_Files.size()) {
      m_Files.push_back(file);
    }

    const std::uint32_t formId = (path.formId() & 0xFFFFFFU) | (newIndex << 24U);
    key                        = formId;
  } else if (path.hasEditorId()) {
    key = path.editorId();
  } else if (path.hasTypeId()) {
    key = path.typeId();
  } else {
    return item;
  }

  auto& recordItem = item->children[key];
  if (!recordItem) {
    recordItem         = std::make_shared<TreeItem>();
    recordItem->parent = item.get();
  }

  return recordItem;
}

}  // namespace TESData
