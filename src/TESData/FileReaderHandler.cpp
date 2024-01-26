#include "FileReaderHandler.h"
#include "PluginList.h"

#include <log.h>

#include <stdexcept>
#include <string_view>

namespace TESData
{

FileReaderHandler::FileReaderHandler(PluginList* pluginList, FileInfo* plugin,
                                     bool lightSupported, bool overlaySupported)
    : m_PluginList{pluginList}, m_Plugin{plugin}, m_LightSupported{lightSupported},
      m_OverlaySupported{overlaySupported}
{}

bool FileReaderHandler::Group(TESFile::GroupData group)
{
  m_CurrentPath.push(group, m_Masters, m_Plugin->name().toStdString());

  if (!group.hasFormType()) {
    return false;
  }

  if (m_Masters.empty() && group.formType() != "GMST"_ts &&
      group.formType() != "DOBJ"_ts) {
    return false;
  }

  return true;
}

void FileReaderHandler::EndGroup()
{
  m_CurrentPath.pop();
}

bool FileReaderHandler::Form(TESFile::FormData form)
{
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
    m_CurrentPath.setFormId(form.formId(), m_Masters, m_Plugin->name().toStdString());

    const int localModIndex   = form.localModIndex();
    const bool isMasterRecord = localModIndex < m_Masters.size();
    if (isMasterRecord) {
      m_PluginList->addRecordConflict(m_Plugin->name().toStdString(), form.type(),
                                      m_CurrentPath);
    }
    return isMasterRecord;
  }
}

bool FileReaderHandler::Chunk(TESFile::Type type)
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

void FileReaderHandler::Data(std::istream& stream)
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

void FileReaderHandler::MainRecordData(std::istream& stream)
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
    std::string master;
    std::getline(stream, master, '\0');
    if (!master.empty()) {
      m_Plugin->addMaster(QString::fromStdString(master.c_str()));
      m_Masters.push_back(master);
    }
  } break;

  case "CNAM"_ts: {
    std::string author;
    std::getline(stream, author, '\0');
    if (!author.empty()) {
      m_Plugin->setAuthor(QString::fromLatin1(author.data()));
    }
  } break;

  case "SNAM"_ts: {
    std::string desc;
    std::getline(stream, desc, '\0');
    if (!desc.empty()) {
      m_Plugin->setDescription(QString::fromLatin1(desc.data()));
    }
  } break;
  }
}

void FileReaderHandler::DefaultObjectData(std::istream& stream)
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
      m_PluginList->addRecordConflict(m_Plugin->name().toStdString(), "DOBJ"_ts,
                                      m_CurrentPath);
    }
    break;
  }
}

void FileReaderHandler::GameSettingData(std::istream& stream)
{
  switch (m_CurrentChunk) {
  case "EDID"_ts:
    std::string editorId;
    std::getline(stream, editorId, '\0');
    m_CurrentPath.setEditorId(editorId);
    m_PluginList->addRecordConflict(m_Plugin->name().toStdString(), "GMST"_ts,
                                    m_CurrentPath);
    break;
  }
}

void FileReaderHandler::StandardData(std::istream& stream)
{
  switch (m_CurrentChunk) {
  case "EDID"_ts:
    std::string editorId;
    std::getline(stream, editorId, '\0');
    // TODO
    break;
  }
}

}  // namespace TESData
