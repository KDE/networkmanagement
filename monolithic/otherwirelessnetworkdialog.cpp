/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "otherwirelessnetworkdialog.h"

#include <QWidget>

#include <activatablelist.h>
#include <unconfiguredinterface.h>
#include <Solid/Device>
#include <wirelessnetwork.h>

#include <hiddenwirelessinterfaceconnection.h>

static const int ItemActivatableRole = 34706;

Q_DECLARE_METATYPE(Knm::Activatable*)

OtherWirelessNetworkDialog::OtherWirelessNetworkDialog(ActivatableList * activatables, QWidget * parent)
    : KDialog(parent), m_activatables(activatables), m_networkItemCount(0)
{
    QWidget * mainWidget = new QWidget(this);
    setWindowTitle(i18nc("@window:title connect to other wireless network", "Select Wireless Network"));
    m_ui.setupUi(mainWidget);
    m_ui.searchLine->setListWidget(m_ui.lwNetworks);
    setMainWidget(mainWidget);

    setButtonGuiItem(KDialog::Ok, KGuiItem(i18nc("@action:button connect to wireless network", "Connect")));
    enableButtonOk(false);

    m_newAdhocItem = new QListWidgetItem(SmallIcon("document-new"), i18nc("@item:inlist Create new ad-hoc wireless connection", "New Ad-Hoc Network..."), m_ui.lwNetworks);

    connect(m_ui.lwNetworks, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(itemDoubleClicked(QListWidgetItem*)));
    connect(m_ui.lwNetworks, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
}

OtherWirelessNetworkDialog::~OtherWirelessNetworkDialog() { }

void OtherWirelessNetworkDialog::handleAdd(Knm::Activatable * activatable)
{
    Knm::WirelessInterfaceConnection * wic = 0;
    Knm::WirelessNetwork * wn = 0;
    QListWidgetItem * item = 0;
    Solid::Device* dev = 0;

    switch (activatable->activatableType()) {
        case Knm::Activatable::HiddenWirelessInterfaceConnection:
            wic = static_cast<Knm::HiddenWirelessInterfaceConnection*>(activatable);
            item = new QListWidgetItem(wic->connectionName());
            item->setData(ItemActivatableRole, QVariant::fromValue(activatable));
            m_ui.lwNetworks->insertItem(m_ui.lwNetworks->count() - 2, item);
            break;
        case Knm::Activatable::UnconfiguredInterface:
            dev = new Solid::Device(activatable->deviceUni());
            item = new QListWidgetItem(SmallIcon("document-new"), i18nc("@item:inlist Create connection to other wireless network using named device", "Connect To Other With %1...", dev->product()), m_ui.lwNetworks);
            item->setData(ItemActivatableRole, QVariant::fromValue(activatable));
            m_ui.lwNetworks->addItem(item);
            break;
        case Knm::Activatable::WirelessNetwork:
            wn = static_cast<Knm::WirelessNetwork*>(activatable);
            item = new QListWidgetItem(SmallIcon("network-wireless"), wn->ssid());
            item->setData(ItemActivatableRole, QVariant::fromValue(activatable));
            m_ui.lwNetworks->insertItem(m_networkItemCount++, item);
            break;
        default:
            break;
    }
}

void OtherWirelessNetworkDialog::handleUpdate(Knm::Activatable *)
{
}
void OtherWirelessNetworkDialog::handleRemove(Knm::Activatable * activatable)
{
    for (int i = 0; i < m_ui.lwNetworks->count(); ++i) {
        QListWidgetItem * item = m_ui.lwNetworks->item(i);
        if (item->data(ItemActivatableRole).value<Knm::Activatable*>() == activatable) {
            if (i < m_networkItemCount) {
                --m_networkItemCount;
            }
            m_ui.lwNetworks->takeItem(i);
            delete item;
            break;
        }
    }
}

void OtherWirelessNetworkDialog::itemDoubleClicked(QListWidgetItem * item)
{
    activateInternal(item);
    m_ui.searchLine->clear();
    hide();
}

void OtherWirelessNetworkDialog::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        QList<QListWidgetItem *> selection = m_ui.lwNetworks->selectedItems();
        if (!selection.isEmpty()) {
            QListWidgetItem * item = selection.first();
            Knm::Activatable * act = item->data(ItemActivatableRole).value<Knm::Activatable*>();
            if (act) {
                act->activate();
            }
        }
    }
    m_ui.searchLine->clear();
    KDialog::slotButtonClicked(button);
}

void OtherWirelessNetworkDialog::activateInternal(QListWidgetItem * item)
{
    if (item == m_newAdhocItem) {
        ;
    } else {
        Knm::Activatable * act = item->data(ItemActivatableRole).value<Knm::Activatable*>();
        if (act) {
            act->activate();
        }
    }
}

void OtherWirelessNetworkDialog::itemSelectionChanged()
{
    enableButtonOk(!m_ui.lwNetworks->selectedItems().isEmpty());
}

// vim: sw=4 sts=4 et tw=100
