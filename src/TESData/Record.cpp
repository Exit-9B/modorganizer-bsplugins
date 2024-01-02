#include "Record.h"

namespace TESData
{

const std::vector<TESFileHandle>& Record::alternatives() const
{
  return m_Alternatives;
}

void Record::addAlternative(TESFileHandle origin)
{
  m_Alternatives.push_back(origin);
}

}  // namespace TESData
