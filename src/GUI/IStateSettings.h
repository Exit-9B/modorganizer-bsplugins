#ifndef GUI_ISTATESETTINGS_H
#define GUI_ISTATESETTINGS_H

#include <concepts>

namespace GUI
{

template <class W>
class IStateSettings
{
public:
  virtual void saveState(const W* widget)    = 0;
  virtual void restoreState(W* widget) const = 0;
};

template <class W>
class [[nodiscard]] StateSaver final
{
public:
  StateSaver(IStateSettings<W>& s, W* w) : m_Settings{s}, m_Widget{w}
  {
    m_Settings.restoreState(m_Widget);
  }

  StateSaver(const StateSaver<W>&) = delete;
  StateSaver(StateSaver<W>&&)      = delete;

  ~StateSaver() { m_Settings.saveState(m_Widget); }

  StateSaver<W>& operator=(const StateSaver<W>&) = delete;
  StateSaver<W>& operator=(StateSaver<W>&&)      = delete;

private:
  IStateSettings<W>& m_Settings;
  W* m_Widget;
};

template <class W, class I = IStateSettings<W>>
StateSaver(I&, W*) -> StateSaver<W>;

}  // namespace GUI

#endif  // GUI_ISTATESETTINGS_H
