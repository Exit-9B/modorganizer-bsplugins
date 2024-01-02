#include "IPluginPanel.h"
#include "MOPanelInterface.h"

#include <QTabWidget>

using namespace Qt::Literals::StringLiterals;

IPluginPanel::Position IPluginPanel::Position::before(const QString& panelName)
{
  return {Order::Before, panelName};
}

IPluginPanel::Position IPluginPanel::Position::after(const QString& panelName)
{
  return {Order::After, panelName};
}

IPluginPanel::Position IPluginPanel::Position::inPlaceOf(const QString& panelName)
{
  return {Order::InPlaceOf, panelName};
}

IPluginPanel::Position IPluginPanel::Position::atStart()
{
  return {Order::AtStart, {}};
}

IPluginPanel::Position IPluginPanel::Position::atEnd()
{
  return {Order::AtEnd, {}};
}

bool IPluginPanel::init(MOBase::IOrganizer* organizer)
{
  if (organizer == nullptr) {
    return false;
  }

  organizer->onUserInterfaceInitialized([this, organizer](QMainWindow* mainWindow) {
    const auto tabWidget = mainWindow->findChild<QTabWidget*>(u"tabWidget"_s);

    if (tabWidget == nullptr) {
      return;
    }

    const auto intfc    = new MOPanelInterface(organizer, mainWindow);
    const auto widget   = this->createWidget(intfc, tabWidget);
    const auto label    = this->label();
    const auto position = this->position();

    switch (position.order_) {
    case Order::Before: {
      const auto refTab = !position.reference_.isNull()
                              ? tabWidget->findChild<QWidget*>(position.reference_)
                              : nullptr;
      if (refTab != nullptr) {
        if (const int index = tabWidget->indexOf(refTab); index != -1) {
          const int currentIndex = tabWidget->currentIndex();
          tabWidget->insertTab(index, widget, label);
          if (index <= currentIndex) {
            tabWidget->setCurrentIndex(currentIndex);
          }
        }
        break;
      }
    }
      [[fallthrough]];
    case Order::AtStart: {
      const int currentIndex = tabWidget->currentIndex();
      tabWidget->insertTab(0, widget, label);
      tabWidget->setCurrentIndex(currentIndex);
    } break;
    case Order::InPlaceOf:
    case Order::After: {
      const auto refTab = !position.reference_.isNull()
                              ? tabWidget->findChild<QWidget*>(position.reference_)
                              : nullptr;

      if (refTab != nullptr) {
        if (const int index = tabWidget->indexOf(refTab); index != -1) {
          tabWidget->insertTab(index + 1, widget, label);
          if (position.order_ == Order::InPlaceOf) {
            tabWidget->removeTab(index);
          }
        }
        break;
      }
    }
      [[fallthrough]];
    case Order::AtEnd: {
      tabWidget->addTab(widget, label);
    } break;
    }

    intfc->assignWidget(tabWidget, widget);
  });

  return this->initPlugin(organizer);
}
