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

struct AuxMember
{
  std::string path;
  std::set<TESFileHandle> alternatives;
};

class AuxItem final
{
public:
  explicit AuxItem(const std::string& name, const AuxItem* parent = nullptr);

  [[nodiscard]] const std::string& name() const { return m_Name; }
  [[nodiscard]] const AuxItem* parent() const { return m_Parent; }

  [[nodiscard]] const auto& children() const { return m_Children; }
  [[nodiscard]] int numChildren() const { return static_cast<int>(m_Children.size()); }
  [[nodiscard]] std::shared_ptr<AuxItem> getByIndex(int index) const;
  [[nodiscard]] std::shared_ptr<AuxItem> getByName(const std::string& name) const;
  std::shared_ptr<AuxItem> insert(const std::string& name);

  [[nodiscard]] const auto& member() const { return m_Member; }
  std::shared_ptr<AuxMember> createMember(const std::string& path);
  void setMember(std::shared_ptr<AuxMember> item);

private:
  std::string m_Name;
  const AuxItem* m_Parent;
  cont::flat_map<std::string, std::shared_ptr<AuxItem>> m_Children;
  std::shared_ptr<AuxMember> m_Member;
};

class AssociatedEntry final
{
public:
  explicit AssociatedEntry(const std::string& rootName = {});

  [[nodiscard]] std::shared_ptr<AuxItem> root() { return m_Root; }
  [[nodiscard]] std::shared_ptr<const AuxItem> root() const { return m_Root; }

  void forEachMember(
      std::function<void(const std::shared_ptr<const AuxMember>&)> func) const;

private:
  std::shared_ptr<AuxItem> m_Root;
};

}  // namespace TESData

#endif  // TESDATA_ASSOCIATEDENTRY_H
