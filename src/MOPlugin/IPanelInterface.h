#ifndef IPANELINTERFACE_H
#define IPANELINTERFACE_H

#include <QList>
#include <QString>
#include <functional>

class IPanelInterface
{
public:
  // user selected files in the panel whose origins should be highlighted
  //
  virtual void setSelectedFiles(const QList<QString>& selectedFiles) = 0;

  // request mod info dialog for origin of file
  //
  virtual void displayOriginInformation(const QString& file) = 0;

  virtual bool onPanelActivated(const std::function<void()>& func) = 0;

  virtual bool
  onSelectedOriginsChanged(const std::function<void(const QList<QString>&)>& func) = 0;

  // HACK: this shouldn't be here, but the MOBase::IPluginList::setState function does
  // not cause the list to notify its onPluginStateChanged listeners, so this will work
  // around that
  virtual void setPluginState(const QString& name, bool enable) = 0;
};

#endif  // IPANELINTERFACE_H
