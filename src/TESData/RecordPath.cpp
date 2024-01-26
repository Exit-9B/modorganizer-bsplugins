#include "RecordPath.h"

#include <algorithm>
#include <iterator>

namespace TESData
{

void RecordPath::setFormId(std::uint32_t formId,
                           const std::vector<std::string>& masters,
                           const std::string& file)
{
  const std::uint8_t localIndex = formId >> 24U;
  const std::string& owner = localIndex < masters.size() ? masters[localIndex] : file;
  const std::uint8_t newIndex = static_cast<std::uint8_t>(
      std::distance(std::begin(m_Files), std::ranges::find(m_Files, owner)));

  if (newIndex == m_Files.size()) {
    m_Files.push_back(owner);
  }

  m_Identifier = (formId & 0xFFFFFF) | (newIndex << 24U);
}

void RecordPath::unsetFormId()
{
  if (!hasFormId())
    return;

  const std::uint8_t localIndex = formId() >> 24U;
  m_Identifier                  = {};

  if (localIndex == m_Files.size() - 1) {
    cleanLastFile();
  }
}

void RecordPath::setEditorId(const std::string& editorId)
{
  unsetFormId();
  m_Identifier = editorId;
}

void RecordPath::setTypeId(TESFile::Type type)
{
  unsetFormId();
  m_Identifier = type;
}

void RecordPath::push(TESFile::GroupData group, const std::vector<std::string>& masters,
                      const std::string& file)
{
  if (group.hasParent()) {
    const std::uint8_t localIndex = group.parent() >> 24U;
    const std::string& owner = localIndex < masters.size() ? masters[localIndex] : file;
    const std::uint8_t newIndex = static_cast<std::uint8_t>(
        std::distance(std::begin(m_Files), std::ranges::find(m_Files, owner)));

    if (newIndex == m_Files.size()) {
      m_Files.push_back(owner);
    }

    group.setLocalIndex(newIndex);
  }

  m_Groups.push_back(group);
}

void RecordPath::pop()
{
  unsetFormId();

  const auto& top               = m_Groups.back();
  const std::uint8_t localIndex = top.hasParent() ? top.parent() >> 24U : 0xFF;
  m_Groups.pop_back();

  if (localIndex == m_Files.size() - 1) {
    cleanLastFile();
  }
}

void RecordPath::cleanLastFile()
{
  if (!std::ranges::any_of(m_Groups, [&](auto&& group) {
        return group.hasParent() && group.parent() >> 24U == m_Files.size() - 1;
      })) {
    m_Files.pop_back();
  }
}

}  // namespace TESData
