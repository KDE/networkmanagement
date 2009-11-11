/*
Copyright 2009 Paul Marchouk <paul.marchouk@gmail.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "menutooltipsettingswidget.h"

#include <QListWidget>
#include <QListWidgetItem>

#include <KIcon>

#include <tooltips.h>

/* arbitrary number identifying the data role for tool tip keys */
static int ToolTipKeyRole = 49172;

class ToolTipListWidgetItem : public QListWidgetItem
{
public:
    ToolTipListWidgetItem(const QString & toolTipKey, QListWidget * parent)
        :QListWidgetItem(parent)
    {
        setText(Knm::ToolTips::labelForKey(toolTipKey));
        setToolTip(Knm::ToolTips::toolTipForKey(toolTipKey));
        setData(ToolTipKeyRole, toolTipKey);
    }
};

MenuToolTipSettingsWidget::MenuToolTipSettingsWidget(QWidget * parent)
: QWidget(parent)
{
    m_ui.setupUi(this);
}

MenuToolTipSettingsWidget::MenuToolTipSettingsWidget(const QStringList & allKeys,
                              const QStringList & selectedKeys, QWidget * parent)
: QWidget(parent)
{
    m_ui.setupUi(this);

    foreach (const QString &toolTipKey, allKeys) {
        if (!selectedKeys.contains(toolTipKey)) {
            addItemToKeysWidget(m_ui.allOptionsListWidget, toolTipKey);
        }
    }

    m_ui.allOptionsListWidget->sortItems(Qt::AscendingOrder);

    foreach(const QString &key, selectedKeys) {
        addItemToKeysWidget(m_ui.selectedOptionsListWidget, key);
    }

    updateButtonsState();
    m_ui.includePushButton->setIcon(KIcon("arrow-right"));
    m_ui.excludePushButton->setIcon(KIcon("arrow-left"));
    m_ui.upPushButton->setIcon(KIcon("arrow-up"));
    m_ui.downPushButton->setIcon(KIcon("arrow-down"));

    connect(m_ui.includePushButton, SIGNAL(clicked()), this, SLOT(includeButtonClicked()));
    connect(m_ui.excludePushButton, SIGNAL(clicked()), this, SLOT(excludeButtonClicked()));
    connect(m_ui.upPushButton, SIGNAL(clicked()), this, SLOT(upButtonClicked()));
    connect(m_ui.downPushButton, SIGNAL(clicked()), this, SLOT(downButtonClicked()));

    connect(m_ui.selectedOptionsListWidget, SIGNAL(currentRowChanged(int)),
            this, SLOT(selectedOptionsCurrentRowChanged(int)));
}

MenuToolTipSettingsWidget::~MenuToolTipSettingsWidget()
{
}

QStringList MenuToolTipSettingsWidget::toolTipKeys() const
{
    QStringList keys;
    for (int i = 0; i < m_ui.selectedOptionsListWidget->count(); ++i) {
        keys << m_ui.selectedOptionsListWidget->item(i)->data(ToolTipKeyRole).toString();
    }
    return keys;
}

void MenuToolTipSettingsWidget::includeButtonClicked()
{
    int currentRow = m_ui.allOptionsListWidget->currentRow();

    QListWidgetItem *currentItem = m_ui.allOptionsListWidget->takeItem(currentRow);;

    if (currentItem) {
        // insert after the selected item in the selectedList
        int pos = m_ui.selectedOptionsListWidget->currentRow() + 1;

        insertItemToKeysWidget(m_ui.selectedOptionsListWidget, currentItem, pos);

        updateButtonsState(IncludeButton | ExcludeButton | DownButton);
    }

    emit changed();
}

void MenuToolTipSettingsWidget::excludeButtonClicked()
{
    int currentRow = m_ui.selectedOptionsListWidget->currentRow();

    QListWidgetItem *currentItem = m_ui.selectedOptionsListWidget->takeItem(currentRow);

    if (currentItem) {

        // insert after the selected item in the allKeysList
        int pos = m_ui.allOptionsListWidget->currentRow() + 1;
        insertItemToKeysWidget(m_ui.allOptionsListWidget, currentItem, pos);

        updateButtonsState();
    }

    emit changed();
}

void MenuToolTipSettingsWidget::upButtonClicked()
{
    int from = m_ui.selectedOptionsListWidget->currentRow();
    int to = from - 1;

    moveSelectedItem(from, to);
    emit changed();
}

void MenuToolTipSettingsWidget::downButtonClicked()
{
    int from = m_ui.selectedOptionsListWidget->currentRow();
    int to = from + 1;

    moveSelectedItem(from, to);
}

void MenuToolTipSettingsWidget::selectedOptionsCurrentRowChanged(int)
{
    updateButtonsState(UpButton | DownButton);
}

void MenuToolTipSettingsWidget::updateButtonsState(Buttons buttons)
{
    bool isEnabled;

    if (buttons.testFlag(ExcludeButton)) {
        isEnabled = m_ui.selectedOptionsListWidget->count() > 0;
        m_ui.excludePushButton->setEnabled(isEnabled);
    }

    if (buttons.testFlag(IncludeButton)) {
        isEnabled = m_ui.allOptionsListWidget->count() > 0;
        m_ui.includePushButton->setEnabled(isEnabled);
    }

    if (buttons.testFlag(UpButton)) {
        isEnabled = (m_ui.selectedOptionsListWidget->currentRow() > 0);
        m_ui.upPushButton->setEnabled(isEnabled);
    }

    if (buttons.testFlag(DownButton)) {
        isEnabled = m_ui.selectedOptionsListWidget->currentRow() != (m_ui.selectedOptionsListWidget->count() - 1);
        m_ui.downPushButton->setEnabled(isEnabled);
    }
}

void MenuToolTipSettingsWidget::insertItemToKeysWidget(QListWidget * widget, QListWidgetItem *item, int row)
{
    bool isItFirstItem = (widget->count() == 0);

    widget->insertItem(row, item);

    if (isItFirstItem) {
        // emulating "Always show selection"
        widget->setCurrentRow(0);
    }
}

void MenuToolTipSettingsWidget::addItemToKeysWidget(QListWidget * widget, const QString &toolTipKey)
{
    bool isItFirstItem = (widget->count() == 0);

    new ToolTipListWidgetItem(toolTipKey, widget);

    if (isItFirstItem) {
        // emulating "Always show selection"
        widget->setCurrentRow(0);
    }
}

void MenuToolTipSettingsWidget::moveSelectedItem(int from, int to)
{
    QListWidgetItem *currentItem = m_ui.selectedOptionsListWidget->takeItem(from);
    if (currentItem) {
        insertItemToKeysWidget(m_ui.selectedOptionsListWidget, currentItem, to);

        m_ui.selectedOptionsListWidget->setCurrentRow(to);

        updateButtonsState(UpButton | DownButton);

        emit changed();
    }
}

// vim: sw=4 sts=4 et tw=100
