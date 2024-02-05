#include "PluginListContextMenu.h"
#include "GUI/ListDialog.h"
#include "MOPlugin/Settings.h"
#include "PluginListModel.h"
#include "PluginListView.h"

#include <utility.h>

#include <QInputDialog>
#include <QMessageBox>

#include <algorithm>
#include <limits>
#include <utility>

namespace BSPluginList
{

PluginListContextMenu::PluginListContextMenu(const QModelIndex& index,
                                             PluginListModel* model,
                                             PluginListView* view,
                                             MOBase::IModList* modList,
                                             MOBase::IPluginList* pluginList)
    : QMenu(view), m_Index{index}, m_Model{model}, m_View{view}
{
  m_ViewSelected = view->selectionModel()->selectedRows();
  if (!m_ViewSelected.isEmpty()) {
    m_ModelSelected = view->indexViewToModel(m_ViewSelected, model);
  } else if (index.isValid()) {
    m_ModelSelected = {index};
  }

  m_FilesSelected =
      !m_ViewSelected.isEmpty() && std::ranges::all_of(m_ViewSelected, [](auto&& idx) {
        return !idx.model()->hasChildren(idx);
      });

  m_GroupsSelected =
      !m_ViewSelected.isEmpty() && std::ranges::all_of(m_ViewSelected, [](auto&& idx) {
        return idx.model()->hasChildren(idx);
      });

  m_FilesTogglable =
      m_FilesSelected && std::ranges::all_of(m_ViewSelected, [](auto&& idx) {
        const auto plugin =
            idx.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
        return plugin && plugin->canBeToggled();
      });

  m_FilesESM =
      m_FilesSelected && std::ranges::all_of(m_ViewSelected, [](auto&& idx) {
        const auto plugin =
            idx.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
        return plugin && !plugin->forceLoaded() && plugin->isMasterFile();
      });

  m_FilesESP =
      m_FilesSelected && std::ranges::all_of(m_ViewSelected, [](auto&& idx) {
        const auto plugin =
            idx.data(PluginListModel::InfoRole).value<const TESData::FileInfo*>();
        return plugin && !plugin->forceLoaded() && !plugin->isMasterFile();
      });

  m_FilesMovable = m_FilesESM | m_FilesESP;

  addAllItemsMenu();
  addSelectedFilesActions();
  addSelectedGroupActions();
  addSelectionActions();
  addOriginActions(modList, pluginList);
}

void PluginListContextMenu::addAllItemsMenu()
{
  QMenu* const allItemsMenu = addMenu(tr("All Items"));

  allItemsMenu->addAction(tr("Collapse all"), [this]() {
    m_View->collapseAll();
    m_View->scrollToTop();
  });
  allItemsMenu->addAction(tr("Expand all"), [this]() {
    m_View->expandAll();
    m_View->scrollToTop();
  });

  allItemsMenu->addSeparator();

  allItemsMenu->addAction(tr("Enable all"), [this]() {
    if (QMessageBox::question(m_View->topLevelWidget(), tr("Confirm"),
                              tr("Really enable all plugins?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      m_Model->setEnabledAll(true);
    }
  });
  allItemsMenu->addAction(tr("Disable all"), [this]() {
    if (QMessageBox::question(m_View->topLevelWidget(), tr("Confirm"),
                              tr("Really disable all plugins?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      m_Model->setEnabledAll(false);
    }
  });
}

void PluginListContextMenu::addSelectedFilesActions()
{
  if (!m_FilesTogglable)
    return;

  addSeparator();

  addAction(tr("Enable selected"), [this]() {
    m_Model->setEnabled(m_ModelSelected, true);
  });
  addAction(tr("Disable selected"), [this]() {
    m_Model->setEnabled(m_ModelSelected, false);
  });
}

void PluginListContextMenu::addSelectedGroupActions()
{
  if (!m_GroupsSelected || m_ViewSelected.length() != 1)
    return;

  addSeparator();

  const auto selectedIndex = m_ViewSelected.first();
  const bool expanded      = m_View->isExpanded(selectedIndex);
  addAction(tr("Collapse others"), [=, this]() {
    m_View->collapseAll();
    m_View->setExpanded(selectedIndex, expanded);
    m_View->scrollTo(selectedIndex);
  });
}

void PluginListContextMenu::addSelectionActions()
{
  if (m_ModelSelected.isEmpty())
    return;

  addSeparator();

  addSendToMenu();

  if (m_FilesSelected) {
    addAction(tr("Create Group..."), [this]() {
      bool ok;
      const QString group =
          QInputDialog::getText(m_View->topLevelWidget(), tr("Create Group..."),
                                tr("Please enter a name:"), QLineEdit::Normal, "", &ok);

      if (!ok || group.isEmpty())
        return;

      QList<QPersistentModelIndex> persistent;
      persistent.reserve(m_ViewSelected.length());
      std::ranges::transform(m_ViewSelected, std::back_inserter(persistent),
                             [](auto&& idx) {
                               return QPersistentModelIndex(idx);
                             });

      const int priority = m_ModelSelected.first()
                               .siblingAtColumn(PluginListModel::COL_PRIORITY)
                               .data()
                               .toInt();
      m_Model->sendToPriority(m_ModelSelected, priority);
      m_Model->setGroup(m_ModelSelected, group);
      for (const auto& index : persistent) {
        m_View->setExpanded(index.parent(), true);
      }
    });
  } else if (m_GroupsSelected) {
    if (m_ViewSelected.length() == 1) {
      addAction(tr("Rename Group..."), [this]() {
        const auto& selected = m_ViewSelected.first();
        QModelIndexList indices;
        for (int i = 0, count = selected.model()->rowCount(selected); i < count; ++i) {
          const auto child = selected.model()->index(i, 0, selected);
          auto&& index     = m_View->indexViewToModel(child, m_Model);
          indices.append(std::move(index));
        }

        bool ok;
        const QString group = QInputDialog::getText(
            m_View->topLevelWidget(), tr("Rename Group..."), tr("Please enter a name:"),
            QLineEdit::Normal, "", &ok);

        if (!ok || group.isEmpty())
          return;

        const auto persistentIndex =
            QPersistentModelIndex(selected.model()->index(0, 0, selected));
        const bool expanded = m_View->isExpanded(selected);

        m_Model->setGroup(indices, group);

        const auto groupIndex = persistentIndex.parent();
        const auto groupRight =
            groupIndex.siblingAtColumn(selected.model()->columnCount() - 1);
        m_View->setExpanded(groupIndex, expanded);
        m_View->selectionModel()->select(QItemSelection(groupIndex, groupRight),
                                         QItemSelectionModel::ClearAndSelect);
        m_View->selectionModel()->setCurrentIndex(groupIndex,
                                                  QItemSelectionModel::Current);
      });
    }

    addAction(tr("Remove Group..."), [this]() {
      QModelIndexList indices;
      for (const auto& selected : m_ViewSelected) {
        for (int i = 0, count = selected.model()->rowCount(selected); i < count; ++i) {
          const auto child = selected.model()->index(i, 0, selected);
          auto&& index     = m_View->indexViewToModel(child, m_Model);
          indices.append(std::move(index));
        }
      }

      if (QMessageBox::question(m_View->topLevelWidget(), tr("Confirm"),
                                tr("Are you sure you want to remove \"%1\"?")
                                    .arg(m_ViewSelected.first().data().toString()),
                                QMessageBox::Yes | QMessageBox::No) ==
          QMessageBox::Yes) {
        m_Model->setGroup(indices, QString());
      }
    });
  }
}

void PluginListContextMenu::addSendToMenu()
{
  if (!m_FilesMovable) {
    return;
  }

  QMenu* const sendToMenu = addMenu(tr("Send to... "));
  sendToMenu->addAction(tr("Top"), [this]() {
    const auto selectedIndex   = m_ViewSelected.first();
    const auto persistentIndex = QPersistentModelIndex(selectedIndex);
    m_Model->sendToPriority(m_ModelSelected, 0);
    m_View->scrollTo(persistentIndex);
  });
  sendToMenu->addAction(tr("Bottom"), [this]() {
    const auto selectedIndex   = m_ViewSelected.first();
    const auto persistentIndex = QPersistentModelIndex(selectedIndex);
    m_Model->sendToPriority(m_ModelSelected, std::numeric_limits<int>::max());
    m_View->scrollTo(persistentIndex);
  });
  sendToMenu->addAction(tr("Priority..."), [this]() {
    const auto selectedIndex = m_ViewSelected.first();

    bool ok;
    const int newPriority =
        QInputDialog::getInt(m_View->topLevelWidget(), tr("Set Priority"),
                             tr("Set the priority of the selected plugins"), 0, 0,
                             std::numeric_limits<int>::max(), 1, &ok);
    if (!ok)
      return;

    const auto persistentIndex = QPersistentModelIndex(selectedIndex);
    m_Model->sendToPriority(m_ModelSelected, newPriority);
    m_View->scrollTo(persistentIndex);
  });
  sendToMenu->addAction(tr("Group..."), [this]() {
    sendSelectedToGroup();
  });
}

static void openOriginExplorer(const QModelIndexList& indices,
                               MOBase::IModList* modList,
                               MOBase::IPluginList* pluginList)
{
  for (const auto& idx : indices) {
    QString fileName   = idx.data().toString();
    const auto modInfo = modList->getMod(pluginList->origin(fileName));
    if (modInfo == nullptr) {
      continue;
    }
    MOBase::shell::Explore(modInfo->absolutePath());
  }
}

void PluginListContextMenu::addOriginActions(MOBase::IModList* modList,
                                             MOBase::IPluginList* pluginList)
{
  if (!m_Index.isValid())
    return;

  addSeparator();

  const auto selectedIdx =
      m_ModelSelected.length() == 1 ? m_ModelSelected.first() : m_Index;
  const auto nameIdx = selectedIdx.siblingAtColumn(PluginListModel::COL_NAME);

  if (std::ranges::any_of(m_ModelSelected, [=](auto&& idx) {
        QString fileName   = idx.data().toString();
        const auto modInfo = modList->getMod(pluginList->origin(fileName));
        return modInfo != nullptr;
      })) {
    addAction(tr("Open Origin in Explorer"), [=, this]() {
      openOriginExplorer(m_ModelSelected, modList, pluginList);
    });

    const auto fileName = nameIdx.data().toString();
    const auto modInfo  = modList->getMod(pluginList->origin(fileName));
    if (modInfo && !modInfo->isForeign()) {
      addAction(tr("Open Origin Info..."), [this, nameIdx] {
        emit openModInformation(nameIdx);
      });
    }
  }

  const auto pluginInfoAction = addAction("Open Plugin Info...", [this, nameIdx] {
    emit openPluginInformation(nameIdx);
  });
  setDefaultAction(pluginInfoAction);
}

void PluginListContextMenu::sendSelectedToGroup()
{
  GUI::ListDialog dialog{*Settings::instance(), m_View->topLevelWidget()};
  dialog.setWindowTitle(tr("Select a group..."));
  if (m_FilesESM) {
    dialog.setChoices(m_Model->masterGroups());
  } else if (m_FilesESP) {
    dialog.setChoices(m_Model->regularGroups());
  }

  if (dialog.exec() != QDialog::Accepted) {
    return;
  }

  const QString result = dialog.getChoice();
  if (result.isEmpty()) {
    return;
  }

  m_Model->sendToGroup(m_ModelSelected, result, m_FilesESM);
}

}  // namespace BSPluginList
