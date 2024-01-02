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
      m_OverlaySupported{overlaySupported}, m_CurrentGroup{NO_TYPE}
{}

bool FileReaderHandler::Group(const TESFile::GroupData& group)
{
  if (!group.hasFormType()) {
    return false;
  }

  if (m_Masters.empty() && group.formType() != "GMST"_ts &&
      group.formType() != "DOBJ"_ts) {
    return false;
  }

  m_CurrentGroup = group.formType();
  return true;
}

bool FileReaderHandler::Form(const TESFile::FormData& form)
{
  switch (m_CurrentGroup) {
  case NO_TYPE:
    if (form.type() == "TES4"_ts) {
      m_CurrentGroup = "TES4"_ts;

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

  case "GMST"_ts:
  case "DOBJ"_ts:
    return true;

  default:
    const int localModIndex   = form.localModIndex();
    const bool isMasterRecord = localModIndex < m_Masters.size();
    if (isMasterRecord) {
      m_PluginList->addForm(m_Plugin->name().toStdString(), m_CurrentGroup,
                            m_Masters[localModIndex], form.formId());
    }
    return isMasterRecord;
  }
}

bool FileReaderHandler::Chunk(TESFile::Type type)
{
  switch (m_CurrentGroup) {
  case "TES4"_ts:
    switch (type) {
    case "HEDR"_ts:
    case "MAST"_ts:
    case "CNAM"_ts:
    case "SNAM"_ts:
      return true;
    }
    return false;

  case "DOBJ"_ts:
    switch (type) {
    case "DNAM"_ts:
      return true;
    }
    return false;

  default:
    switch (type) {
    case "EDID"_ts:
      return true;
    }
    return false;
  }

  return false;
}

void FileReaderHandler::ChunkData(TESFile::Type type, std::istream& stream)
{
  switch (m_CurrentGroup) {
  case "TES4"_ts:
    switch (type) {
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
    break;

  case "DOBJ"_ts:
    switch (type) {
    case "DNAM"_ts:
      while (!stream.eof()) {
        const TESFile::Type name   = TESFile::readType<TESFile::Type>(stream);
        const std::uint32_t formId = TESFile::readType<std::uint32_t>(stream);
        if (name == TESFile::Type()) {
          continue;
        }
        if (name == "BBBB"_ts) {
          break;
        }
        m_PluginList->addDefaultObject(m_Plugin->name().toStdString(), name);
      }
      break;
    }

  case "GMST"_ts:
    switch (type) {
    case "EDID"_ts:
      std::string editorId;
      std::getline(stream, editorId, '\0');
      m_PluginList->addSetting(m_Plugin->name().toStdString(), editorId);
      break;
    }

  default:
    switch (type) {
    case "EDID"_ts:
      std::string editorId;
      std::getline(stream, editorId, '\0');
      // TODO
      break;
    }
  }
}

}  // namespace TESData
