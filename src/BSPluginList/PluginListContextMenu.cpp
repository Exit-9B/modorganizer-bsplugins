#include "PluginListContextMenu.h"
#include "PluginListModel.h"
#include "PluginListView.h"

#include <imodlist.h>
#include <ipluginlist.h>
#include <utility.h>

#include <QInputDialog>
#include <QMessageBox>

#include <algorithm>
#include <limits>

namespace BSPluginList
{

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

PluginListContextMenu::PluginListContextMenu(const QModelIndex& index,
                                             PluginListModel* model,
                                             PluginListView* view,
                                             MOBase::IModList* modList,
                                             MOBase::IPluginList* pluginList)
    : QMenu(view), m_Index{index}, m_Model{model}, m_View{view}
{
  if (view->selectionModel()->hasSelection()) {
    m_Selected = view->indexViewToModel(view->selectionModel()->selectedRows(), model);
  } else if (index.isValid()) {
    m_Selected = {index};
  }

  if (!m_Selected.isEmpty()) {
    addAction(tr("Enable selected"), [this]() {
      m_Model->setEnabled(m_Selected, true);
    });
    addAction(tr("Disable selected"), [this]() {
      m_Model->setEnabled(m_Selected, false);
    });

    addSeparator();
  }

  addAction(tr("Enable all"), [this]() {
    if (QMessageBox::question(m_View->topLevelWidget(), tr("Confirm"),
                              tr("Really enable all plugins?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      m_Model->setEnabledAll(true);
    }
  });
  addAction(tr("Disable all"), [this]() {
    if (QMessageBox::question(m_View->topLevelWidget(), tr("Confirm"),
                              tr("Really disable all plugins?"),
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
      m_Model->setEnabledAll(false);
    }
  });

  if (!m_Selected.isEmpty()) {
    addSeparator();
    QMenu* const sendToMenu = addMenu(tr("Send to... "));
    sendToMenu->addAction(tr("Top"), [this]() {
      m_Model->sendToPriority(m_Selected, 0);
    });
    sendToMenu->addAction(tr("Bottom"), [this]() {
      m_Model->sendToPriority(m_Selected, std::numeric_limits<int>::max());
    });
    sendToMenu->addAction(tr("Priority..."), [this]() {
      bool ok;
      const int newPriority =
          QInputDialog::getInt(m_View->topLevelWidget(), tr("Set Priority"),
                               tr("Set the priority of the selected plugins"), 0, 0,
                               std::numeric_limits<int>::max(), 1, &ok);
      if (!ok)
        return;

      m_Model->sendToPriority(m_Selected, newPriority);
    });
  }

  if (m_Index.isValid()) {
    addSeparator();

    if (std::ranges::any_of(m_Selected, [=](auto&& idx) {
          QString fileName = idx.data().toString();
          return modList->getMod(pluginList->origin(fileName)) != nullptr;
        })) {
      addAction(tr("Open Origin in Explorer"), [=, this]() {
        openOriginExplorer(m_Selected, modList, pluginList);
      });

      if (m_Selected.size() == 1) {
        const auto nameIdx  = m_Index.siblingAtColumn(PluginListModel::COL_NAME);
        const auto fileName = nameIdx.data().toString();
        const auto modInfo  = modList->getMod(pluginList->origin(fileName));
        if (modInfo && !modInfo->isForeign()) {
          const auto infoAction = addAction(tr("Open Origin Info..."), [this, nameIdx] {
            emit openModInformation(nameIdx);
          });
          setDefaultAction(infoAction);
        }
      }
    }
  }
}

}  // namespace BSPluginList
