#include "RecordPath.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <sstream>

namespace TESData
{

std::string RecordPath::string() const
{
  std::ostringstream ss;
  ss << ":/";

  for (const TESFile::GroupData group : m_Groups) {
    if (group.hasFormType()) {
      ss << group.formType().view() << "/";
    } else if (group.hasParent()) {
      switch (group.type()) {
      case TESFile::GroupType::CellPersistentChildren:
        ss << "Persistent/";
      case TESFile::GroupType::CellTemporaryChildren:
        ss << "Temporary/";
      case TESFile::GroupType::CellVisibleDistantChildren:
        ss << "Visible Distant/";
      default: {
        const auto parentId           = group.parent();
        const std::uint8_t localIndex = parentId >> 24U;
        const std::string& owner      = m_Files.at(localIndex);
        ss << owner << "|";
        ss << std::hex << std::setfill('0') << std::setw(6) << (parentId & 0xFFFFFF);
        ss << "/";
      }
      }
    } else if (group.hasBlock()) {
      ss << group.block() << "/";
    } else if (group.hasGridCell()) {
      auto [x, y] = group.gridCell();
      ss << x << ", " << y << "/";
    }
  }

  if (hasFormId()) {
    const auto id                 = formId();
    const std::uint8_t localIndex = id >> 24U;
    const std::string& owner      = m_Files.at(localIndex);
    ss << owner << "|";
    ss << std::hex << std::setfill('0') << std::setw(6) << (id & 0xFFFFFF);
  } else if (hasEditorId()) {
    ss << editorId();
  } else if (hasTypeId()) {
    ss << typeId();
  }

  return ss.str();
}

void RecordPath::setFormId(std::uint32_t formId, std::span<const std::string> masters,
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

void RecordPath::setIdentifier(const Identifier& identifier,
                               std::span<const std::string> masters)
{
  if (std::holds_alternative<std::uint32_t>(identifier)) {
    setFormId(std::get<std::uint32_t>(identifier), masters, "");
  } else {
    unsetFormId();
    m_Identifier = identifier;
  }
}

void RecordPath::push(TESFile::GroupData group, std::span<const std::string> masters,
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
