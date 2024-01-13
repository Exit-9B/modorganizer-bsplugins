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
#include <utility>

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
  const auto selectedRows = view->selectionModel()->selectedRows();
  if (!selectedRows.isEmpty()) {
    m_Selected = view->indexViewToModel(selectedRows, model);
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

    const bool filesSelected = std::ranges::all_of(selectedRows, [=](auto&& idx) {
      return !idx.model()->hasChildren(idx);
    });

    const bool groupsSelected = std::ranges::all_of(selectedRows, [=](auto&& idx) {
      return idx.model()->hasChildren(idx);
    });

    if (filesSelected) {
      addAction(tr("Create Group..."), [this]() {
        bool ok;
        const QString group = QInputDialog::getText(
            m_View->topLevelWidget(), tr("Create Group..."), tr("Please enter a name:"),
            QLineEdit::Normal, "", &ok);

        if (!ok || group.isEmpty())
          return;

        m_Model->setGroup(m_Selected, group);
      });
    } else if (groupsSelected) {
      if (selectedRows.length() == 1) {
        addAction(tr("Rename Group..."), [this]() {
          const auto selectedRows = m_View->selectionModel()->selectedRows();
          const auto& selected    = selectedRows.first();
          QModelIndexList indices;
          for (int i = 0, count = selected.model()->rowCount(selected); i < count;
               ++i) {
            const auto child = selected.model()->index(i, 0, selected);
            auto&& index     = m_View->indexViewToModel(child, m_Model);
            indices.append(std::move(index));
          }

          bool ok;
          const QString group = QInputDialog::getText(
              m_View->topLevelWidget(), tr("Rename Group..."),
              tr("Please enter a name:"), QLineEdit::Normal, "", &ok);

          if (!ok || group.isEmpty())
            return;

          m_Model->setGroup(indices, group);
        });
      }

      addAction(tr("Remove Group..."), [this]() {
        const auto selectedRows = m_View->selectionModel()->selectedRows();
        QModelIndexList indices;
        for (const auto& selected : selectedRows) {
          for (int i = 0, count = selected.model()->rowCount(selected); i < count;
               ++i) {
            const auto child = selected.model()->index(i, 0, selected);
            auto&& index     = m_View->indexViewToModel(child, m_Model);
            indices.append(std::move(index));
          }
        }

        if (QMessageBox::question(m_View->topLevelWidget(), tr("Confirm"),
                                  tr("Are you sure you want to remove \"%1\"?")
                                      .arg(selectedRows.first().data().toString()),
                                  QMessageBox::Yes | QMessageBox::No) ==
            QMessageBox::Yes) {
          m_Model->setGroup(indices, QString());
        }
      });
    }
  }

  if (m_Index.isValid()) {
    addSeparator();

    if (std::ranges::any_of(m_Selected, [=](auto&& idx) {
          QString fileName   = idx.data().toString();
          const auto modInfo = modList->getMod(pluginList->origin(fileName));
          return modInfo != nullptr;
        })) {
      addAction(tr("Open Origin in Explorer"), [=, this]() {
        openOriginExplorer(m_Selected, modList, pluginList);
      });

      const auto selectedIdx = m_Selected.length() == 1 ? m_Selected.first() : m_Index;
      const auto nameIdx     = selectedIdx.siblingAtColumn(PluginListModel::COL_NAME);
      const auto fileName    = nameIdx.data().toString();
      const auto modInfo     = modList->getMod(pluginList->origin(fileName));
      if (modInfo && !modInfo->isForeign()) {
        const auto infoAction = addAction(tr("Open Origin Info..."), [this, nameIdx] {
          emit openModInformation(nameIdx);
        });
        setDefaultAction(infoAction);
      }
    }
  }
}

}  // namespace BSPluginList
