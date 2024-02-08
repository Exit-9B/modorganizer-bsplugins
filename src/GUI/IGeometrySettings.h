#ifndef GUI_IGEOMETRYSETTINGS_H
#define GUI_IGEOMETRYSETTINGS_H

#include <concepts>

namespace GUI
{

template <class W>
class IGeometrySettings
{
public:
  virtual void saveGeometry(const W* widget)    = 0;
  virtual void restoreGeometry(W* widget) const = 0;
};

template <class W>
class [[nodiscard]] GeometrySaver final
{
public:
  GeometrySaver(IGeometrySettings<W>& s, W* w) : m_Settings{s}, m_Widget{w}
  {
    m_Settings.restoreGeometry(m_Widget);
  }

  GeometrySaver(const GeometrySaver<W>&) = delete;
  GeometrySaver(GeometrySaver<W>&&)      = delete;

  ~GeometrySaver() { m_Settings.saveGeometry(m_Widget); }

  GeometrySaver<W>& operator=(const GeometrySaver<W>&) = delete;
  GeometrySaver<W>& operator=(GeometrySaver<W>&&)      = delete;

private:
  IGeometrySettings<W>& m_Settings;
  W* m_Widget;
};

template <class W, std::derived_from<W> Derived>
GeometrySaver(IGeometrySettings<W>&, Derived*) -> GeometrySaver<W>;

}  // namespace GUI

#endif  // GUI_IGEOMETRYSETTINGS_H
