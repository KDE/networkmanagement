/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#include "interfaceitem.h"

#include <NetworkManager.h>

#include <QDBusObjectPath>
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>

#include <KDebug>

#include <Plasma/Icon>
#include <Plasma/Label>

#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>
#include "nm-active-connectioninterface.h"
#include "networkmanagersettings.h"
#include "remoteconnection.h"

InterfaceItem::InterfaceItem(Solid::Control::NetworkInterface * iface, NameDisplayMode mode, QGraphicsItem * parent) : QGraphicsWidget(parent), m_iface(iface), m_nameMode(mode)
{
#if 0 // this layouts pretty badly, esp where labels and icons share a horizontal layout
    // main layout
    m_layout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    //   icon on left
    m_icon = new Plasma::Icon(this);
    m_icon->setIcon("network-wireless");
    //   vertical stack of info in middle
    m_infoLayout = new QGraphicsLinearLayout(Qt::Vertical, 0);
    //     interface layout
    m_ifaceNameLabel = new Plasma::Label(this);
    m_ifaceNameLabel->setText(QString::fromLatin1("<b>%1</b>").arg(m_iface->interfaceName()));
    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText("Active connection name");
    //     connection info
    m_connectionInfoLayout = new QGraphicsLinearLayout(Qt::Horizontal, 0);
    //       IP address
    m_connectionInfoIpAddrLabel = new Plasma::Label(this);
    m_connectionInfoIpAddrLabel->setText("192.168.0.8");
    //       signal strength
    m_connectionInfoStrengthLabel = new Plasma::Label(this);
    m_connectionInfoStrengthLabel->setText("101%");
    //       security
    m_connectionInfoSecurityIcon = new Plasma::Icon(this);
    m_connectionInfoSecurityIcon->setIcon("system-lock-screen");
    m_connectionInfoLayout->addItem(m_connectionInfoStrengthLabel);
    m_connectionInfoLayout->addItem(m_connectionInfoSecurityIcon);
    m_infoLayout->addItem(m_ifaceNameLabel);
    m_infoLayout->addItem(m_connectionNameLabel);
    m_infoLayout->addItem(m_connectionInfoIpAddrLabel);
    m_infoLayout->addItem(m_connectionInfoLayout);
    // connect button at right
    m_connectButton = new Plasma::Icon(this);
    m_connectButton->setIcon("media-playback-start");
    m_layout->addItem(m_icon);
    m_layout->addItem(m_infoLayout);
    m_layout->addItem(m_connectButton);
    setLayout(m_layout);
#else
    m_layout = new QGraphicsGridLayout(this);
    m_icon = new Plasma::Icon(this);

    switch (m_iface->type() ) {
        case Solid::Control::NetworkInterface::Ieee8023:
            m_icon->setIcon("network-wired");
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            m_icon->setIcon("network-wired");
            break;
        case Solid::Control::NetworkInterface::Serial:
            m_icon->setIcon("modem");
            break;
        case Solid::Control::NetworkInterface::Gsm:
        case Solid::Control::NetworkInterface::Cdma:
            m_icon->setIcon("phone");
            break;
        default:
            m_icon->setIcon("network-wired");
            break;
    }

    //     interface layout
    m_ifaceNameLabel = new Plasma::Label(this);
    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    //       IP address
    m_connectionInfoIpAddrLabel = new Plasma::Label(this);
    m_connectionInfoIpAddrLabel->setText("dum.my.ip.addr");
    //       signal strength
    //m_connectionInfoStrengthLabel = new Plasma::Label(this);
    //m_connectionInfoStrengthLabel->setText("101%");
    //       security
    m_connectionInfoSecurityIcon = new Plasma::Icon(this);
    m_connectionInfoSecurityIcon->setIcon("system-lock-screen"); m_connectionInfoSecurityIcon->setMaximumHeight(32); // connect button at right
    m_connectButton = new Plasma::Icon(this);
    //m_connectButton->setIcon("media-playback-start");
    m_layout->addItem(m_icon, 0, 0, 3, 1);
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 3);
    m_layout->addItem(m_connectionNameLabel, 1, 1, 1, 3);
    m_layout->addItem(m_connectionInfoIpAddrLabel, 2, 1, 1, 1);
    //m_layout->addItem(m_connectionInfoStrengthLabel, 2, 2, 1, 1);
    m_layout->addItem(m_connectionInfoSecurityIcon, 2, 3, 1, 1);
    m_layout->addItem(m_connectButton, 0, 4, 3, 1);
#endif
    connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(activeConnectionsChanged()),
            SLOT(activeConnectionsChanged()));
    connect(m_iface, SIGNAL(connectionStateChanged(int)),
            SLOT(connectionStateChanged(int)));

    setNameDisplayMode(mode);
    connectionStateChanged(m_iface->connectionState());
}

InterfaceItem::~InterfaceItem()
{
}


void InterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if ( m_nameMode == InterfaceName ) {
        m_ifaceNameLabel->setText(QString::fromLatin1("<b>%1</b>").arg(m_iface->interfaceName()));
    } else {
        m_ifaceNameLabel->setText(QString::fromLatin1("<b>Hardware name goes here</b>"));
    }
}

InterfaceItem::NameDisplayMode InterfaceItem::nameDisplayMode() const
{
    return m_nameMode;
}

void InterfaceItem::activeConnectionsChanged()
{
    kDebug();
}

void InterfaceItem::connectionStateChanged(int state)
{
    kDebug() << state;
    // get the active connections
    // check if any of them affect our interface
    // setActiveConnection on ourself

    // if this connection is not there is an active connection on this interface
    switch (state) {
        case Solid::Control::NetworkInterface::Unavailable:
            setUnavailable();
            break;
        case Solid::Control::NetworkInterface::Disconnected:
        case Solid::Control::NetworkInterface::Failed:
            // set the disconnected icon
            setInactive();
            break;
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
        case Solid::Control::NetworkInterface::Activated: // lookup the active connection, get its state
            setActiveConnection(state);
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::UnknownState:
            kDebug() << "This can't be happening to me! Unmanaged interfaces are not show in the widget.";
            break;
    }
}

void InterfaceItem::setUnavailable()
{
    m_icon->setEnabled(false);
    m_connectionNameLabel->setText(i18nc("Label for network interfaces that cannot be activated", "Unavailable"));
    m_connectionInfoIpAddrLabel->setText("");
    m_connectButton->setEnabled(false);
}

void InterfaceItem::setInactive()
{
    m_icon->setEnabled(false);
    m_connectionNameLabel->setText("");
    m_connectionInfoIpAddrLabel->setText("");
    m_connectButton->setIcon("media-playback-stop");
}


void InterfaceItem::setActiveConnection(int state)
{
    QStringList activeConnections = Solid::Control::NetworkManager::activeConnections();
    QString ourService;
    QDBusObjectPath ourConnectionPath;
    bool defaultRoute;
    // find the active connection on this device
    bool found = false;
    m_icon->setEnabled(true);
    m_connectButton->setEnabled(true);
    m_connectButton->setIcon("media-playback-stop");
    foreach (QString conn, activeConnections) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface candidate(NM_DBUS_SERVICE,
                conn, QDBusConnection::systemBus(), 0);
        foreach (QDBusObjectPath path, candidate.devices()) {
            if (path.path() == m_iface->uni()) {
                ourService = candidate.serviceName();
                ourConnectionPath = candidate.connection();
                defaultRoute = candidate.getDefault();
                found = true;
                break;
            }
        }
        if (found) {
            break;
        }
    }
    // now we know what the first active connection on this device is, update our UI
    // sanity check
    if (!ourService.isEmpty()) {
        NetworkManagerSettings service(ourService, 0 );
        RemoteConnection * ourConnection = service.findConnection(ourConnectionPath.path());
        //connection name
        m_connectionNameLabel->setText(ourConnection->id());
        // ip address (Hello, Jos!)
        if (state == Solid::Control::NetworkInterface::Activated) {
            Solid::Control::IPv4Config ip4Config = m_iface->ipV4Config();
            QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
            if (addresses.isEmpty()) {
                m_connectionInfoIpAddrLabel->setText("ip display error");
            } else {
                QHostAddress addr(addresses.first().address());
                m_connectionInfoIpAddrLabel->setText(addr.toString());
            }
        }
    }
}

// vim: sw=4 sts=4 et tw=100
