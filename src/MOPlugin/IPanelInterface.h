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

  virtual bool onPanelActivated(const std::function<void()>& func) = 0;

  virtual bool
  onSelectedOriginsChanged(const std::function<void(const QList<QString>&)>& func) = 0;
};

#endif  // IPANELINTERFACE_H
