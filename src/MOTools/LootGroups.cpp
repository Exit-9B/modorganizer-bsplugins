#include "LootGroups.h"

#include <safewritefile.h>
#include <utility.h>

#include <QFile>
#include <QRegularExpression>

#pragma warning(push)
#pragma warning(disable : 4459)
#undef emit
#include <ryml.hpp>
#pragma warning(pop)

#include <map>

using namespace Qt::Literals::StringLiterals;

namespace MOTools
{

static bool isRegex(const ryml::csubstr& key)
{
  return key.first_of(":\\*?|\"") != ryml::npos;
}

static void importLootGroups(const QString& lootconfig,
                             std::map<QString, QString>& groups)
{
  if (lootconfig.isEmpty()) {
    return;
  }

  QFile f{lootconfig};

  if (!f.open(QIODevice::ReadOnly)) {
    return;
  }

  auto buffer     = f.readAll();
  ryml::Tree tree = ryml::parse_in_place(ryml::substr(buffer.data(), buffer.size()));
  tree.resolve();

  const std::size_t root_id = tree.root_id();
  if (!tree.is_map(root_id))
    return;

  const std::size_t plugins_id = tree.find_child(root_id, "plugins");
  if (plugins_id == ryml::npos || !tree.is_seq(plugins_id))
    return;

  for (std::size_t pos = 0, num = tree.num_children(plugins_id); pos < num; ++pos) {
    const std::size_t plugin_id = tree.child(plugins_id, pos);
    if (!tree.is_map(plugin_id))
      continue;

    const std::size_t group_id = tree.find_child(plugin_id, "group");
    if (group_id == ryml::npos)
      continue;

    const std::size_t name_id = tree.find_child(plugin_id, "name");
    if (name_id == ryml::npos)
      continue;

    const auto& name  = tree.val(name_id);
    const auto& group = tree.val(group_id);

    if (isRegex(name)) {
      const auto pattern = QString::fromLocal8Bit(name.data(), name.size());
      const auto re =
          QRegularExpression(pattern, QRegularExpression::CaseInsensitiveOption);
      for (auto it = groups.begin(); it != groups.end(); ++it) {
        const auto match = re.match(it->first);
        if (match.hasMatch()) {
          it->second = QString::fromLocal8Bit(group.data(), group.size());
        }
      }
    } else {
      const auto key = QString::fromLocal8Bit(name.data(), name.size());
      if (const auto it = groups.find(key); it != groups.end()) {
        it->second = QString::fromLocal8Bit(group.data(), group.size());
      }
    }
  }
}

static void writeGroups(const QString& plugingroups, std::map<QString, QString>& groups)
{
  MOBase::SafeWriteFile file{plugingroups};

  file->resize(0);
  file->write("# This file was automatically generated by Mod Organizer.\r\n"_ba);
  for (const auto& [plugin, group] : groups) {
    if (!group.isEmpty()) {
      file->write(u"%1|%2\r\n"_s.arg(plugin).arg(group).toUtf8());
    }
  }

  file.commit();
}

void importLootGroups(const TESData::PluginList* pluginList,
                      const QString& plugingroups, const QString& masterlist,
                      const QString& userlist)
{
  MOBase::TimeThis tt{"importLootGroups"};

  std::map<QString, QString> groups;
  for (int id = 0, count = pluginList->pluginCount(); id < count; ++id) {
    const auto plugin = pluginList->getPlugin(id);
    groups.emplace(plugin->name(), QString());
  }

  importLootGroups(masterlist, groups);
  importLootGroups(userlist, groups);
  writeGroups(plugingroups, groups);
}

}  // namespace MOTools