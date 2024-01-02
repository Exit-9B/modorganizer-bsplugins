#ifndef TESDATA_RECORD_H
#define TESDATA_RECORD_H

#include <QString>

#include <vector>

namespace TESData
{

using TESFileHandle = int;

class Record final
{
public:
  [[nodiscard]] const std::vector<TESFileHandle>& alternatives() const;

  void addAlternative(TESFileHandle origin);

private:
  QString m_EditorID;
  std::vector<TESFileHandle> m_Alternatives;
};

}  // namespace TESData

#endif  // TESDATA_RECORD_H
