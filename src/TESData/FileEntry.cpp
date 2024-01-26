#include "FileEntry.h"

#include <log.h>

namespace TESData
{

FileEntry::FileEntry(TESFileHandle handle, const std::string& name)
    : m_Handle{handle}, m_Name{name}, m_Root{std::make_shared<TreeItem>()}
{}

TESFileHandle FileEntry::handle() const
{
  return m_Handle;
}

const std::string& FileEntry::name() const
{
  return m_Name;
}

void FileEntry::forEachRecord(
    std::function<void(const std::shared_ptr<const Record>&)> func) const
{
  std::vector<std::shared_ptr<TreeItem>> stack;
  stack.push_back(m_Root);
  while (!stack.empty()) {
    const auto item = std::move(stack.back());
    stack.pop_back();

    if (std::holds_alternative<std::shared_ptr<Record>>(item->data)) {
      if (const auto record = std::get<std::shared_ptr<Record>>(item->data)) {
        func(record);
      }
    }

    for (const auto& [identifier, child] : item->children) {
      stack.push_back(child);
    }
  }
}

std::shared_ptr<FileEntry::TreeItem> FileEntry::createHierarchy(const RecordPath& path)
{
  const auto groups = path.groups();
  auto item         = m_Root;
  for (TESFile::GroupData group : groups) {
    if (group.hasParent()) {
      const auto& file = path.files()[group.parent() >> 24];
      const std::uint8_t newIndex = static_cast<std::uint8_t>(
          std::distance(std::begin(m_Files), std::ranges::find(m_Files, file)));

      if (newIndex == m_Files.size()) {
        m_Files.push_back(file);
      }

      group.setLocalIndex(newIndex);
      auto& nextItem = item->children[group.parent()];
      if (!nextItem) {
        MOBase::log::debug("Parent form '{}|{:06X}' was not initialized", file,
                           group.parent() & 0xFFFFFF);

        nextItem         = std::make_shared<TreeItem>();
        nextItem->parent = item.get();
      }

      item = nextItem;
    }

    auto& nextItem = item->children[group];
    if (!nextItem) {
      nextItem         = std::make_shared<TreeItem>();
      nextItem->parent = item.get();
      nextItem->data   = group;
    }
    item = nextItem;
  }

  TreeItem::Identifier identifier;
  if (path.hasFormId()) {
    const auto& file            = path.files()[path.formId() >> 24];
    const std::uint8_t newIndex = static_cast<std::uint8_t>(
        std::distance(std::begin(m_Files), std::ranges::find(m_Files, file)));

    if (newIndex == m_Files.size()) {
      m_Files.push_back(file);
    }

    const std::uint32_t formId = (path.formId() & 0xFFFFFFU) | (newIndex << 24U);
    identifier                 = formId;
  } else if (path.hasEditorId()) {
    identifier = path.editorId();
  } else if (path.hasTypeId()) {
    identifier = path.typeId();
  }
  auto& recordItem = item->children[identifier];
  if (!recordItem) {
    recordItem         = std::make_shared<TreeItem>();
    recordItem->parent = item.get();
  }

  return recordItem;
}

std::shared_ptr<Record> FileEntry::createRecord(const RecordPath& path)
{
  const auto item = createHierarchy(path);

  if (!std::holds_alternative<std::shared_ptr<Record>>(item->data)) {
    const auto record = std::make_shared<Record>();
    item->data        = record;
    record->addAlternative(m_Handle);
  }

  return std::get<std::shared_ptr<Record>>(item->data);
}

void FileEntry::addRecord(const RecordPath& path, std::shared_ptr<Record> record)
{
  record->addAlternative(m_Handle);
  const auto item = createHierarchy(path);
  item->data      = record;
}

}  // namespace TESData
