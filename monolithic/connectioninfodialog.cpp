/*
Copyright 2009 Andrey Batyiev <batyiev@gmail.com>

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
#include <KLocale>

#include "connectioninfodialog.h"
#include "connectioninfoipv4tab.h"
#include "connectioninfowiredtab.h"
#include "connectioninfowirelesstab.h"

#include <solid/control/networkmanager.h>

#include <uiutils.h>


ConnectionInfoDialog::ConnectionInfoDialog(Knm::InterfaceConnection *ic, QWidget *parent)
    : KDialog(parent), m_ic(ic), m_guiMade(false)
{
    QWidget *mainWidget = new QWidget(this);
    m_ui.setupUi(mainWidget);
    setMainWidget(mainWidget);

    m_ui.connectionName->setText(ic->connectionName());

    // handle activation/deactivation of InterfaceConnection
    connect(ic, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
            this, SLOT(interfaceConnectionStateChanged(Knm::InterfaceConnection::ActivationState)));
    interfaceConnectionStateChanged(ic->activationState());
    
    // handle removal of InterfaceConnection
    connect(ic, SIGNAL(destroyed(QObject*)),
            this, SLOT(close()));

    setAttribute(Qt::WA_DeleteOnClose);
    setButtons(KDialog::Close);
}

ConnectionInfoDialog::~ConnectionInfoDialog()
{
}

void ConnectionInfoDialog::interfaceConnectionStateChanged(Knm::InterfaceConnection::ActivationState state)
{
    if (state == Knm::InterfaceConnection::Unknown) {
        if (m_guiMade)
            clearGUI();
        m_ui.connectionState->setText(QString());
    } else {
        if (!m_guiMade)
            buildGUI();
    }
}

void ConnectionInfoDialog::buildGUI()
{
    QString deviceUni = m_ic->deviceUni();
    m_iface = Solid::Control::NetworkManager::findNetworkInterface(deviceUni);

    if (m_iface == 0) {
        return;
    }
    
    ConnectionInfoIPv4Tab *ipTab = new ConnectionInfoIPv4Tab(m_iface, m_ui.infoTabWidget);
    m_ui.infoTabWidget->addTab(ipTab, i18nc("@title:tab information about ip address, nameserveres, etc", "IPv4"));
    
    QString deviceType = UiUtils::interfaceTypeLabel(m_iface->type());
    switch (m_iface->type()) {
    case Solid::Control::NetworkInterface::Ethernet: {
            ConnectionInfoWiredTab *wiredTab = new ConnectionInfoWiredTab(qobject_cast<Solid::Control::WiredNetworkInterface*>(m_iface), m_ui.infoTabWidget);
            m_ui.infoTabWidget->addTab(wiredTab, deviceType);
            m_ui.connectionIcon->setPixmap(KIconLoader::global()->loadIcon("network-wired", KIconLoader::Panel));
            break;
        }
    case Solid::Control::NetworkInterface::Wifi: {
            ConnectionInfoWirelessTab *wirelessTab = new ConnectionInfoWirelessTab(qobject_cast<Solid::Control::WirelessNetworkInterface*>(m_iface), m_ui.infoTabWidget);
            m_ui.infoTabWidget->addTab(wirelessTab, deviceType);
            m_ui.connectionIcon->setPixmap(KIconLoader::global()->loadIcon("network-wireless", KIconLoader::Panel));
            break;
        }
    default: break;
    }

    connect(m_iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(updateConnectionState(int,int,int)));

    Solid::Control::NetworkInterfaceNm09::ConnectionState state = static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(m_iface->connectionState());
    m_ui.connectionState->setText(UiUtils::connectionStateToString(state));
    m_guiMade = true;
}

void ConnectionInfoDialog::clearGUI()
{
    disconnect(m_iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(updateConnectionState(int,int,int)));
    
    int tabCount = m_ui.infoTabWidget->count();
    while (tabCount--) {
        QWidget *w = m_ui.infoTabWidget->widget(0);
        m_ui.infoTabWidget->removeTab(0);
        delete w;
    }
    m_ui.connectionState->setText(QString());
    m_guiMade = false;
}

void ConnectionInfoDialog::updateConnectionState(int new_state, int, int)
{
    m_ui.connectionState->setText(UiUtils::connectionStateToString((Solid::Control::NetworkInterfaceNm09::ConnectionState)new_state));
}
