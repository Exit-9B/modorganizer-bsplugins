#include "BranchConflictParser.h"

#include <algorithm>
#include <iterator>

namespace TESData
{

BranchConflictParser::BranchConflictParser(PluginList* pluginList,
                                           const std::string& pluginName,
                                           const RecordPath& path)
    : m_PluginList{pluginList}, m_PluginName{pluginName}, m_Path{path}
{}

bool BranchConflictParser::Group(TESFile::GroupData group)
{
  if (m_CurrentPath.groups().size() < m_Path.groups().size()) {

    if (group.hasParent()) {
      const std::uint8_t localIndex = group.parent() >> 24U;
      const std::string& owner =
          localIndex < m_Masters.size() ? m_Masters[localIndex] : m_PluginName;
      const auto files            = m_Path.files();
      const std::uint8_t newIndex = static_cast<std::uint8_t>(
          std::distance(std::begin(files), std::ranges::find(files, owner)));

      group.setLocalIndex(newIndex);
    }

    if (group == m_Path.groups()[m_CurrentPath.groups().size()]) {
      m_CurrentPath.push(group, m_Path.files(), m_PluginName);
      return true;
    }

    return false;
  } else {
    m_CurrentPath.push(group, m_Masters, m_PluginName);
    return true;
  }
}

void BranchConflictParser::EndGroup()
{
  m_CurrentPath.pop();
}

bool BranchConflictParser::Form(TESFile::FormData form)
{
  m_CurrentType = form.type();

  if (m_CurrentPath.groups().empty()) {
    return form.type() == "TES4"_ts;
  }

  if (m_CurrentPath.groups().size() < m_Path.groups().size()) {
    return false;
  }

  m_CurrentPath.setFormId(form.formId(), m_Masters, m_PluginName);

  const std::uint8_t localModIndex = form.localModIndex();
  const bool isMasterRecord        = localModIndex < m_Masters.size();
  return isMasterRecord;
}

void BranchConflictParser::EndForm()
{
  if (m_CurrentType != "TES4"_ts && m_CurrentType != "TES3"_ts) {

    m_PluginList->addRecordConflict(m_PluginName, m_CurrentPath, m_CurrentType,
                                    m_CurrentName);
  }

  m_CurrentPath.unsetFormId();
  m_CurrentType  = {};
  m_CurrentChunk = {};
  m_CurrentName.clear();
}

bool BranchConflictParser::Chunk(TESFile::Type type)
{
  m_CurrentChunk = type;
  if (m_CurrentPath.groups().empty()) {
    return type == "MAST"_ts;
  } else {
    return type == "EDID"_ts;
  }
}

void BranchConflictParser::Data(std::istream& stream)
{
  switch (m_CurrentChunk) {
  case "MAST"_ts: {
    const std::string master = TESFile::readZstring(stream);
    if (!master.empty()) {
      m_Masters.push_back(master);
    }
  } break;

  case "EDID"_ts: {
    const std::string editorId = TESFile::readZstring(stream);
    m_CurrentName              = std::move(editorId);
  } break;
  }
}

}  // namespace TESData
