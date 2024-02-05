#ifndef TESDATA_RECORD_H
#define TESDATA_RECORD_H

#include "TESFile/Type.h"

#include <QString>

#include <set>
#include <span>

namespace TESData
{

using TESFileHandle = int;

class Record final
{
public:
  using Identifier =
      std::variant<std::monostate, std::uint32_t, std::string, TESFile::Type>;

  [[nodiscard]] TESFile::Type formType() const { return m_FormType; }

  [[nodiscard]] const std::string& file() const { return m_File; }

  [[nodiscard]] const Identifier& identifier() const { return m_Identifier; }

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

  [[nodiscard]] const std::set<TESFileHandle>& alternatives() const
  {
    return m_Alternatives;
  }

  [[nodiscard]] bool ignored() const { return m_Ignored; }
  void setIgnored(bool value) { m_Ignored = value; }

  void setIdentifier(const Identifier& identifier, std::span<const std::string> files)
  {
    m_Identifier = identifier;

    if (std::holds_alternative<std::uint32_t>(m_Identifier)) {
      std::uint32_t& formId         = std::get<std::uint32_t>(m_Identifier);
      const std::uint8_t localIndex = formId >> 24U;
      formId &= ~0xFF000000U;
      m_File = files[localIndex];
    }
  }

  void addAlternative(TESFileHandle origin) { m_Alternatives.insert(origin); }

private:
  TESFile::Type m_FormType;
  std::string m_File;
  Identifier m_Identifier;
  std::set<TESFileHandle> m_Alternatives;
  bool m_Ignored = false;
};

}  // namespace TESData

#endif  // TESDATA_RECORD_H
