#ifndef TESDATA_ASSOCIATEDENTRY_H
#define TESDATA_ASSOCIATEDENTRY_H

#include <boost/container/flat_map.hpp>

#include <QString>

#include <functional>
#include <memory>
#include <set>
#include <string>

namespace cont = boost::container;

namespace TESData
{

using TESFileHandle = int;

struct AuxConflictItem
{
  std::set<TESFileHandle> alternatives;
};

class AuxGroupItem final
{
public:
  AuxGroupItem(const std::string& name, AuxGroupItem* parent = nullptr);

  std::shared_ptr<AuxGroupItem> insert(const std::string& name);
  std::shared_ptr<AuxConflictItem> createConflictItem(const std::string& name);
  void addConflictItem(const std::string& name, std::shared_ptr<AuxConflictItem> item);

  [[nodiscard]] int numChildren() const { return static_cast<int>(m_Children.size()); }
  [[nodiscard]] std::shared_ptr<AuxGroupItem> getByIndex(int index) const;
  [[nodiscard]] std::shared_ptr<AuxGroupItem> getByName(const std::string& name) const;

  [[nodiscard]] const auto& children() const { return m_Children; }
  [[nodiscard]] const auto& items() const { return m_Items; }

private:
  std::string m_Name;
  AuxGroupItem* m_Parent;
  cont::flat_map<std::string, std::shared_ptr<AuxGroupItem>> m_Children;
  cont::flat_map<std::string, std::shared_ptr<AuxConflictItem>> m_Items;
};

class AssociatedEntry final
{
public:
  explicit AssociatedEntry(const std::string& rootName = {});

  [[nodiscard]] std::shared_ptr<AuxGroupItem> root() { return m_Root; }
  [[nodiscard]] std::shared_ptr<const AuxGroupItem> root() const { return m_Root; }

  void forEachConflictItem(
      std::function<void(const std::shared_ptr<const AuxConflictItem>&)> func) const;

private:
  std::shared_ptr<AuxGroupItem> m_Root;
};

}  // namespace TESData

#endif  // TESDATA_ASSOCIATEDENTRY_H
