#include "Record.h"

namespace TESData
{

const std::set<TESFileHandle>& Record::alternatives() const
{
  return m_Alternatives;
}

void Record::addAlternative(TESFileHandle origin)
{
  m_Alternatives.insert(origin);
}

}  // namespace TESData
