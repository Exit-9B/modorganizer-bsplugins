#ifndef TESDATA_FILEENTRY_H
#define TESDATA_FILEENTRY_H

#include "Record.h"
#include "RecordPath.h"
#include "TESFile/Type.h"

#include <boost/container/flat_map.hpp>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace cont = boost::container;

namespace TESData
{

using TESFileHandle = int;

class FileEntry final
{
public:
  struct TreeItem
  {
    using Key =
        std::variant<TESFile::GroupData, std::uint32_t, std::string, TESFile::Type>;

    const TreeItem* parent;
    std::string name;
    TESFile::Type formType;
    std::optional<TESFile::GroupData> group;
    std::shared_ptr<Record> record;
    cont::flat_map<Key, std::shared_ptr<TreeItem>> children;
  };

  FileEntry(TESFileHandle handle, const std::string& name);

  [[nodiscard]] TESFileHandle handle() const { return m_Handle; }
  [[nodiscard]] const std::string& name() const { return m_Name; }
  [[nodiscard]] TreeItem* dataRoot() const { return m_Root.get(); }
  [[nodiscard]] const std::vector<std::string>& files() const { return m_Files; }

  void
  forEachRecord(std::function<void(const std::shared_ptr<const Record>&)> func) const;

  std::shared_ptr<Record> createRecord(const RecordPath& path, const std::string& name,
                                       TESFile::Type formType);
  void addRecord(const RecordPath& path, const std::string& name,
                 TESFile::Type formType, std::shared_ptr<Record> record);
  void addGroup(const RecordPath&);

  [[nodiscard]] std::shared_ptr<Record> findRecord(const RecordPath& path) const;

private:
  std::shared_ptr<TreeItem> createHierarchy(const RecordPath& path);

  TESFileHandle m_Handle;
  std::string m_Name;
  std::shared_ptr<TreeItem> m_Root;
  std::vector<std::string> m_Files;
};

}  // namespace TESData

#endif  // TESDATA_FILEENTRY_H
