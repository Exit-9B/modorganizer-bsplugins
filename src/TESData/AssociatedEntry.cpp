#include "AssociatedEntry.h"

#include <vector>

namespace TESData
{

AuxGroupItem::AuxGroupItem(const std::string& name, AuxGroupItem* parent)
    : m_Name{name}, m_Parent{parent}
{}

std::shared_ptr<AuxGroupItem> AuxGroupItem::insert(const std::string& name)
{
  const auto [it, inserted] =
      m_Children.try_emplace(name, std::make_shared<AuxGroupItem>(name, this));
  return it->second;
}

std::shared_ptr<AuxConflictItem>
AuxGroupItem::createConflictItem(const std::string& name)
{
  auto& item = m_Items[name];
  if (item != nullptr) {
    return item;
  }

  return item = std::make_shared<AuxConflictItem>();
}

void AuxGroupItem::addConflictItem(const std::string& name,
                                   std::shared_ptr<AuxConflictItem> item)
{
  m_Items[name] = item;
}

std::shared_ptr<AuxGroupItem> AuxGroupItem::getByIndex(int index) const
{
  if (index < 0 || index >= m_Children.size()) {
    return nullptr;
  }

  return m_Children.nth(index)->second;
}

std::shared_ptr<AuxGroupItem> AuxGroupItem::getByName(const std::string& name) const
{
  const auto it = m_Children.find(name);
  if (it == m_Children.end()) {
    return nullptr;
  }

  return it->second;
}

AssociatedEntry::AssociatedEntry(const std::string& rootName)
    : m_Root{std::make_shared<AuxGroupItem>(rootName)}
{}

void AssociatedEntry::forEachConflictItem(
    std::function<void(const std::shared_ptr<const AuxConflictItem>&)> func) const
{
  std::vector<std::shared_ptr<AuxGroupItem>> stack{m_Root};

  while (!stack.empty()) {
    const auto group = std::move(stack.back());
    stack.pop_back();

    for (const auto& [name, item] : group->items()) {
      func(item);
    }

    for (const auto& [name, child] : group->children()) {
      stack.push_back(child);
    }
  }
}

}  // namespace TESData
