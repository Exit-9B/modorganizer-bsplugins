#ifndef TESDATA_FILEENTRY_H
#define TESDATA_FILEENTRY_H

#include "Record.h"
#include "RecordPath.h"
#include "TESFile/Type.h"

#include <boost/container/flat_map.hpp>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
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
    using Identifier =
        std::variant<TESFile::GroupData, std::uint32_t, std::string, TESFile::Type>;

    const TreeItem* parent;
    std::variant<std::monostate, std::shared_ptr<Record>, TESFile::GroupData> data;
    cont::flat_map<Identifier, std::shared_ptr<TreeItem>> children;
  };

  FileEntry(TESFileHandle handle, const std::string& name);

  [[nodiscard]] TESFileHandle handle() const;
  [[nodiscard]] const std::string& name() const;

  void
  forEachRecord(std::function<void(const std::shared_ptr<const Record>&)> func) const;

  std::shared_ptr<Record> createRecord(const RecordPath& path);

  void addRecord(const RecordPath& path, std::shared_ptr<Record> record);

private:
  std::shared_ptr<TreeItem> createHierarchy(const RecordPath& path);

  TESFileHandle m_Handle;
  std::string m_Name;
  std::shared_ptr<TreeItem> m_Root;
  std::vector<std::string> m_Files;
};

}  // namespace TESData

#endif  // TESDATA_FILEENTRY_H
