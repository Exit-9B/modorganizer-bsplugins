#include "FileConflictParser.h"
#include "PluginList.h"

#include <log.h>

#include <stdexcept>
#include <string_view>

namespace TESData
{

FileConflictParser::FileConflictParser(PluginList* pluginList, FileInfo* plugin,
                                       bool lightSupported, bool overlaySupported)
    : m_PluginList{pluginList}, m_Plugin{plugin}, m_LightSupported{lightSupported},
      m_OverlaySupported{overlaySupported}
{
  m_PluginName = m_Plugin->name().toStdString();
}

bool FileConflictParser::Group(TESFile::GroupData group)
{
  if (group.hasDirectParent()) {
    m_CurrentPath.push(group, m_Masters, m_PluginName);
    m_PluginList->addGroupPlaceholder(m_PluginName, m_CurrentPath);
    m_CurrentPath.pop();
    return false;
  }

  if (m_Masters.empty() && group.hasFormType() && group.formType() != "GMST"_ts &&
      group.formType() != "DOBJ"_ts) {
    return false;
  }

  if (group.hasFormType() && group.formType() == "NAVI"_ts) {
    return false;
  }

  m_CurrentPath.push(group, m_Masters, m_PluginName);
  return true;
}

void FileConflictParser::EndGroup()
{
  m_CurrentPath.pop();
}

bool FileConflictParser::Form(TESFile::FormData form)
{
  m_CurrentType = form.type();

  if (m_CurrentPath.groups().empty()) {
    if (form.type() == "TES4"_ts) {
      m_Plugin->setMasterFlagged(form.flags() & TESFile::RecordFlags::Master);
      m_Plugin->setOverlayFlagged(m_OverlaySupported &&
                                  (form.flags() & TESFile::RecordFlags::Overlay));
      m_Plugin->setLightFlagged(
          m_OverlaySupported ? (form.flags() & TESFile::RecordFlags::LightNew)
          : m_LightSupported ? (form.flags() & TESFile::RecordFlags::LightOld)
                             : false);
      return true;
    } else {
      throw std::runtime_error("Unsupported header record");
    }
  }

  switch (m_CurrentPath.groups().front().formType()) {
  case "DOBJ"_ts:
  case "GMST"_ts:
    return true;
  default:
    m_CurrentPath.setFormId(form.formId(), m_Masters, m_PluginName);

    const int localModIndex   = form.localModIndex();
    const bool isMasterRecord = localModIndex < m_Masters.size();
    return isMasterRecord;
  }
}

void FileConflictParser::EndForm()
{
  if (m_CurrentType != "TES4"_ts && m_CurrentType != "TES3"_ts &&
      m_CurrentType != "GMST"_ts && m_CurrentType != "DOBJ"_ts) {

    m_PluginList->addRecordConflict(m_PluginName, m_CurrentPath, m_CurrentType,
                                    m_CurrentName);
  }

  m_CurrentPath.unsetFormId();
  m_CurrentType  = {};
  m_CurrentChunk = {};
  m_CurrentName.clear();
}

bool FileConflictParser::Chunk(TESFile::Type type)
{
  m_CurrentChunk = type;
  if (m_CurrentPath.groups().empty()) {
    switch (type) {
    case "HEDR"_ts:
    case "MAST"_ts:
    case "CNAM"_ts:
    case "SNAM"_ts:
      return true;
    }
    return false;
  } else if (m_CurrentPath.groups().front().formType() == "DOBJ"_ts) {
    switch (type) {
    case "DNAM"_ts:
      return true;
    }
    return false;
  } else {
    switch (type) {
    case "EDID"_ts:
      return true;
    }
    return false;
  }
}

void FileConflictParser::Data(std::istream& stream)
{
  if (m_CurrentPath.groups().empty()) {
    return MainRecordData(stream);
  }

  switch (m_CurrentPath.groups().front().formType()) {
  case "DOBJ"_ts:
    return DefaultObjectData(stream);
  case "GMST"_ts:
    return GameSettingData(stream);
  default:
    return StandardData(stream);
  }
}

void FileConflictParser::MainRecordData(std::istream& stream)
{
  switch (m_CurrentChunk) {
  case "HEDR"_ts: {
    struct Header
    {
      float version;
      int32_t numRecords;
      uint32_t nextObjectId;
    };

    const auto header = TESFile::readType<Header>(stream);
    if (stream.fail()) {
      MOBase::log::error("failed to read HEDR data");
      return;
    }

    m_Plugin->setHasNoRecords(header.numRecords == 0);
  } break;

  case "MAST"_ts: {
    const std::string master = TESFile::readZstring(stream);
    if (!master.empty()) {
      m_Plugin->addMaster(QString::fromStdString(master.c_str()));
      m_Masters.push_back(master);
    }
  } break;

  case "CNAM"_ts: {
    const std::string author = TESFile::readZstring(stream);
    if (!author.empty()) {
      m_Plugin->setAuthor(QString::fromLatin1(author.data()));
    }
  } break;

  case "SNAM"_ts: {
    const std::string desc = TESFile::readZstring(stream);
    if (!desc.empty()) {
      m_Plugin->setDescription(QString::fromLatin1(desc.data()));
    }
  } break;
  }
}

void FileConflictParser::DefaultObjectData(std::istream& stream)
{
  switch (m_CurrentChunk) {
  case "DNAM"_ts:
    while (!stream.eof()) {
      const TESFile::Type name = TESFile::readType<TESFile::Type>(stream);
      [[maybe_unused]] const std::uint32_t formId =
          TESFile::readType<std::uint32_t>(stream);
      if (name == TESFile::Type()) {
        continue;
      }
      if (name == "BBBB"_ts) {
        break;
      }
      m_CurrentPath.setTypeId(name);
      m_PluginList->addRecordConflict(m_PluginName, m_CurrentPath, "DOBJ"_ts, "");
    }
    break;
  }
}

void FileConflictParser::GameSettingData(std::istream& stream)
{
  switch (m_CurrentChunk) {
  case "EDID"_ts: {
    const std::string editorId = TESFile::readZstring(stream);
    m_CurrentPath.setEditorId(editorId);
    m_PluginList->addRecordConflict(m_PluginName, m_CurrentPath, "GMST"_ts, "");
  } break;
  }
}

void FileConflictParser::StandardData(std::istream& stream)
{
  switch (m_CurrentChunk) {
  case "EDID"_ts: {
    const std::string editorId = TESFile::readZstring(stream);
    m_CurrentName              = std::move(editorId);
  } break;
  }
}

}  // namespace TESData
