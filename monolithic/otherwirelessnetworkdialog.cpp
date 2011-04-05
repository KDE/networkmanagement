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
#include <QTreeWidget>

#include <activatablelist.h>
#include <uiutils.h>
#include <unconfiguredinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <wirelessnetwork.h>

#include <hiddenwirelessinterfaceconnection.h>
#include <wirelesssecurityidentifier.h>

static const int ItemActivatableRole = 34706;

OtherWirelessNetworkDialog::OtherWirelessNetworkDialog(ActivatableList * activatables, QWidget * parent)
    : KDialog(parent), m_activatables(activatables), m_networkItemCount(0)
{
    QWidget * mainWidget = new QWidget(this);
    setWindowTitle(i18nc("@window:title connect to other wireless network", "Select Wireless Network"));
    m_ui.setupUi(mainWidget);
    m_ui.searchLine->setTreeWidget(m_ui.twNetworks);
    setMainWidget(mainWidget);

    setButtonGuiItem(KDialog::Ok, KGuiItem(i18nc("@action:button connect to wireless network", "Connect")));
    enableButtonOk(false);

    QStringList itemStrings;
    itemStrings << i18nc("@item:inlist Create new ad-hoc wireless connection", "New Ad-Hoc Network...");
    m_newAdhocItem = new QTreeWidgetItem( m_ui.twNetworks, itemStrings);
    m_ui.twNetworks->setFirstItemColumnSpanned(m_newAdhocItem, true);
    m_newAdhocItem->setIcon(0, SmallIcon("document-new"));

    connect(m_ui.twNetworks, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemDoubleClicked(QTreeWidgetItem*)));
    connect(m_ui.twNetworks, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
}

OtherWirelessNetworkDialog::~OtherWirelessNetworkDialog() { }

void OtherWirelessNetworkDialog::handleAdd(Knm::Activatable * activatable)
{
    Knm::WirelessInterfaceConnection * wic = 0;
    Knm::WirelessNetwork * wn = 0;
    QTreeWidgetItem * item = 0;
    QStringList itemStrings;
    QString deviceText;
    QString strengthString;
    Knm::WirelessSecurity::Type best;
    switch (activatable->activatableType()) {
        case Knm::Activatable::HiddenWirelessInterfaceConnection:
            wic = static_cast<Knm::HiddenWirelessInterfaceConnection*>(activatable);
            best = Knm::WirelessSecurity::best(wic->interfaceCapabilities(), true, (wic->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc), wic->apCapabilities(), wic->wpaFlags(), wic->rsnFlags());
            if (wic->strength() >= 0) {
                strengthString = QString::fromLatin1("%1%").arg(wic->strength());
            }
            itemStrings << wic->connectionName() << strengthString << Knm::WirelessSecurity::label(best);
            item = new QTreeWidgetItem(itemStrings);
            item->setIcon(0, SmallIcon("document-properties"));
            item->setData(0, ItemActivatableRole, QVariant::fromValue(activatable));
            m_ui.twNetworks->insertTopLevelItem(m_ui.twNetworks->topLevelItemCount() - 2, item);
            break;
        case Knm::Activatable::UnconfiguredInterface:
            deviceText = UiUtils::interfaceNameLabel(activatable->deviceUni());

            itemStrings << i18nc("@item:inlist Create connection to other wireless network using named device", "Connect To Other Network With %1...", deviceText);
            item = new QTreeWidgetItem(itemStrings);
            item->setIcon(0, SmallIcon("document-new"));
            item->setData(0, ItemActivatableRole, QVariant::fromValue(activatable));
            m_ui.twNetworks->addTopLevelItem(item);
            m_ui.twNetworks->setFirstItemColumnSpanned(item, true);
            break;
        case Knm::Activatable::WirelessNetwork:
            wn = static_cast<Knm::WirelessNetwork*>(activatable);
            best = Knm::WirelessSecurity::best(wn->interfaceCapabilities(), true, (wn->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc), wn->apCapabilities(), wn->wpaFlags(), wn->rsnFlags());
            itemStrings << wn->ssid() << QString::fromLatin1("%1%").arg(wn->strength()) << Knm::WirelessSecurity::label(best);
            item = new QTreeWidgetItem(itemStrings);
            item->setIcon(0, SmallIcon("network-wireless"));
            item->setData(0, ItemActivatableRole, QVariant::fromValue(activatable));
            m_ui.twNetworks->insertTopLevelItem(m_networkItemCount++, item);
            break;
        case Knm::Activatable::WirelessInterfaceConnection:
            wic = static_cast<Knm::WirelessInterfaceConnection*>(activatable);
            best = Knm::WirelessSecurity::best(wic->interfaceCapabilities(), true, (wic->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc), wic->apCapabilities(), wic->wpaFlags(), wic->rsnFlags());
            if (wic->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc) {
                strengthString = i18nc("@item:inlist Signal strength replaced by description on ad-hoc", "create Ad-Hoc");
            }
            else
            {
                strengthString = (QString::fromLatin1("%1%").arg(wic->strength()));
            }
            connect(wic, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)), this, SLOT(setActivationState(Knm::InterfaceConnection::ActivationState)));
            itemStrings << wic->connectionName() << strengthString << Knm::WirelessSecurity::label(best);
            item = new QTreeWidgetItem(itemStrings);
            item->setIcon(0, SmallIcon("network-wireless"));
            item->setData(0, ItemActivatableRole, QVariant::fromValue(activatable));
            m_ui.twNetworks->insertTopLevelItem(m_networkItemCount++, item);
            break;
        default:
            break;
    }
    for (int i = 0; i < m_ui.twNetworks->columnCount(); i++) {
        m_ui.twNetworks->resizeColumnToContents(i);
    }
}

void OtherWirelessNetworkDialog::handleUpdate(Knm::Activatable *)
{
}
void OtherWirelessNetworkDialog::handleRemove(Knm::Activatable * activatable)
{
    for (int i = 0; i < m_ui.twNetworks->topLevelItemCount(); ++i) {
        QTreeWidgetItem * item = m_ui.twNetworks->topLevelItem(i);
        if (item->data(0, ItemActivatableRole).value<Knm::Activatable*>() == activatable) {
            if (i < m_networkItemCount) {
                --m_networkItemCount;
            }
            m_ui.twNetworks->takeTopLevelItem(i);
            delete item;
            break;
        }
    }
}

void OtherWirelessNetworkDialog::itemDoubleClicked(QTreeWidgetItem * item)
{
    activateInternal(item);
    m_ui.searchLine->clear();
    hide();
}

void OtherWirelessNetworkDialog::slotButtonClicked(int button)
{
    if (button == KDialog::Ok) {
        QList<QTreeWidgetItem *> selection = m_ui.twNetworks->selectedItems();
        if (!selection.isEmpty()) {
            QTreeWidgetItem * item = selection.first();
            Knm::Activatable * act = item->data(0, ItemActivatableRole).value<Knm::Activatable*>();
            if (act) {
                act->activate();
            }
        }
    }
    m_ui.searchLine->clear();
    KDialog::slotButtonClicked(button);
}

void OtherWirelessNetworkDialog::activateInternal(QTreeWidgetItem * item)
{
    if (item == m_newAdhocItem) {
        ;
    } else {
        Knm::Activatable * act = item->data(0, ItemActivatableRole).value<Knm::Activatable*>();
        if (act) {
            act->activate();
        }
    }
}

void OtherWirelessNetworkDialog::setActivationState(Knm::InterfaceConnection::ActivationState state)
{
    Knm::WirelessInterfaceConnection * wic = qobject_cast<Knm::WirelessInterfaceConnection*>(sender());
    for (int i = 0; i < m_ui.twNetworks->topLevelItemCount(); ++i) {
        QTreeWidgetItem * item = m_ui.twNetworks->topLevelItem(i);
        if (item->data(0, ItemActivatableRole).value<Knm::Activatable*>() == static_cast<Knm::Activatable*>(wic)) {
            QString strengthString;
            if (wic->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc)
            {
                strengthString = i18nc("@item:inlist Signal strength replaced by description on ad-hoc", "create Ad-Hoc");
            }
            else
            {
                strengthString = (QString::fromLatin1("%1%").arg(wic->strength()));
            }
            if (state == Knm::InterfaceConnection::Activated)
            {
                strengthString.append(i18nc("@item:inlist connection status added to signal strength if we are connected", " (connected)"));
            }
            item->setData(1,0,strengthString);
            break;
        }
    }
}

void OtherWirelessNetworkDialog::itemSelectionChanged()
{
    enableButtonOk(!m_ui.twNetworks->selectedItems().isEmpty());
}

// vim: sw=4 sts=4 et tw=100
