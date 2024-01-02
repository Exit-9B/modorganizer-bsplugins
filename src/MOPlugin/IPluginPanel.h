#ifndef IPLUGINPANEL_H
#define IPLUGINPANEL_H

#include "IPanelInterface.h"

#include <iplugin.h>

class IPluginPanel : public QObject, public MOBase::IPlugin
{
  Q_INTERFACES(MOBase::IPlugin)

public:
  enum class Order
  {
    Before,
    AtStart,
    InPlaceOf,
    After,
    AtEnd,
  };

  struct Position
  {
    Order order_;
    QString reference_;

    static Position before(const QString& panelName);
    static Position after(const QString& panelName);
    static Position inPlaceOf(const QString& panelName);
    static Position atStart();
    static Position atEnd();
  };

  bool init(MOBase::IOrganizer* organizer) final;

  virtual bool initPlugin(MOBase::IOrganizer* organizer) = 0;

  virtual QWidget* createWidget(IPanelInterface* callbacks, QWidget* parent) = 0;

  virtual QString label() const = 0;

  virtual Position position() const = 0;
};

Q_DECLARE_INTERFACE(IPluginPanel, "com.tannin.ModOrganizer.Plugin/2.0")

#endif  // IPLUGINPANEL_H
