#ifndef TESDATA_RECORDPATH_H
#define TESDATA_RECORDPATH_H

#include "TESFile/Stream.h"

#include <boost/container/small_vector.hpp>

#include <cstdint>
#include <span>
#include <string>
#include <variant>
#include <vector>

namespace TESData
{

class RecordPath final
{
public:
  [[nodiscard]] bool hasFormId() const
  {
    return std::holds_alternative<std::uint32_t>(m_Identifier);
  }

  [[nodiscard]] bool hasEditorId() const
  {
    return std::holds_alternative<std::string>(m_Identifier);
  }

  [[nodiscard]] bool hasTypeId() const
  {
    return std::holds_alternative<TESFile::Type>(m_Identifier);
  }

  [[nodiscard]] std::uint32_t formId() const
  {
    return std::get<std::uint32_t>(m_Identifier);
  }

  [[nodiscard]] const std::string& editorId() const
  {
    return std::get<std::string>(m_Identifier);
  }

  [[nodiscard]] TESFile::Type typeId() const
  {
    return std::get<TESFile::Type>(m_Identifier);
  }

  [[nodiscard]] std::span<const std::string> files() const { return m_Files; }

  [[nodiscard]] std::span<const TESFile::GroupData> groups() const { return m_Groups; }

  [[nodiscard]] std::string string() const;

  void setFormId(std::uint32_t formId, const std::vector<std::string>& masters,
                 const std::string& file);

  void unsetFormId();

  void setEditorId(const std::string& editorId);

  void setTypeId(TESFile::Type type);

  void push(TESFile::GroupData group, const std::vector<std::string>& masters,
            const std::string& file);

  void pop();

private:
  void cleanLastFile();

  boost::container::small_vector<std::string, 2> m_Files;
  boost::container::small_vector<TESFile::GroupData, 4> m_Groups;
  std::variant<std::monostate, std::uint32_t, std::string, TESFile::Type> m_Identifier;
};

}  // namespace TESData

#endif  // TESDATA_RECORDPATH_H
