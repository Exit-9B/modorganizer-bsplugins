#include "AssociatedEntry.h"

#include <vector>

namespace TESData
{

AuxItem::AuxItem(const std::string& name, AuxItem* parent)
    : m_Name{name}, m_Parent{parent}
{}

std::shared_ptr<AuxItem> AuxItem::getByIndex(int index) const
{
  if (index < 0 || index >= m_Children.size()) {
    return nullptr;
  }

  return m_Children.nth(index)->second;
}

std::shared_ptr<AuxItem> AuxItem::getByName(const std::string& name) const
{
  const auto it = m_Children.find(name);
  if (it == m_Children.end()) {
    return nullptr;
  }

  return it->second;
}

std::shared_ptr<AuxItem> AuxItem::insert(const std::string& name)
{
  const auto [it, inserted] =
      m_Children.try_emplace(name, std::make_shared<AuxItem>(name, this));
  return it->second;
}

std::shared_ptr<AuxMember> AuxItem::createMember(const std::string& path)
{
  auto& item = m_Member;
  if (item != nullptr) {
    return item;
  }

  item = std::make_shared<AuxMember>();
  item->path = path;
  return item;
}

void AuxItem::setMember(std::shared_ptr<AuxMember> item)
{
  m_Member = item;
}

AssociatedEntry::AssociatedEntry(const std::string& rootName)
    : m_Root{std::make_shared<AuxItem>(rootName)}
{}

void AssociatedEntry::forEachMember(
    std::function<void(const std::shared_ptr<const AuxMember>&)> func) const
{
  std::vector<std::shared_ptr<AuxItem>> stack{m_Root};

  while (!stack.empty()) {
    const auto item = std::move(stack.back());
    stack.pop_back();

    if (const auto member = item->member()) {
      func(member);
    }

    for (const auto& [name, child] : item->children()) {
      stack.push_back(child);
    }
  }
}

}  // namespace TESData
