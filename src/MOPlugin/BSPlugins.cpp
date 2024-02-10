#include "BSPlugins.h"

#include "BSPluginList/PluginsWidget.h"
#include "Settings.h"

using namespace Qt::Literals::StringLiterals;

bool BSPlugins::initPlugin(MOBase::IOrganizer* organizer)
{
  m_Organizer = organizer;
  Settings::init(organizer);
  return true;
}

QString BSPlugins::name() const
{
  return NAME;
}

std::vector<std::shared_ptr<const MOBase::IPluginRequirement>>
BSPlugins::requirements() const
{
  return {Requirements::gameDependency(
      {u"Oblivion"_s, u"Fallout 3"_s, u"New Vegas"_s, u"Skyrim"_s, u"Enderal"_s,
       u"Fallout 4"_s, u"Skyrim Special Edition"_s, u"Enderal Special Edition"_s,
       u"Skyrim VR"_s, u"Fallout 4 VR"_s, u"Starfield"_s})};
}

QString BSPlugins::author() const
{
  return u"Parapets"_s;
}

QString BSPlugins::description() const
{
  return tr("Manages plugin load order for BGS game engines");
}

MOBase::VersionInfo BSPlugins::version() const
{
  return MOBase::VersionInfo(0, 1, 1, 0, MOBase::VersionInfo::RELEASE_BETA);
}

QList<MOBase::PluginSetting> BSPlugins::settings() const
{
  return {
      {u"enable_sort_button"_s, u"Enable the Sort button in the Plugins panel"_s, true},
      {u"external_change_warning"_s,
       u"Warn if load order changes while running an executable"_s, true},
      {u"loot_show_dirty"_s,
       u"LOOT: Show information about plugins that can be cleaned"_s, true},
      {u"loot_show_messages"_s,
       u"LOOT: Show general information and warning messages"_s, true},
      {u"loot_show_problems"_s,
       u"LOOT: Show information about incompatibilities and missing masters"_s, true},
  };
}

bool BSPlugins::enabledByDefault() const
{
  return true;
}

QWidget* BSPlugins::createWidget(IPanelInterface* panelInterface, QWidget* parent)
{
  const auto widget =
      new BSPluginList::PluginsWidget(m_Organizer, panelInterface, parent);
  return widget;
}

QString BSPlugins::label() const
{
  return tr("Plugins");
}

IPluginPanel::Position BSPlugins::position() const
{
  return Position::inPlaceOf(u"espTab"_s);
}
