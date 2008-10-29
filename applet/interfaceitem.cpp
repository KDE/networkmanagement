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

#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QLabel>

#include <KDebug>
#include <KGlobalSettings>
#include <KNotification>

#include <Plasma/Icon>
#include <Plasma/Label>

#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>

#include "connectioninspector.h"
#include "events.h"
#include "nm-active-connectioninterface.h"
#include "networkmanagersettings.h"
#include "remoteconnection.h"
#include "wirelessnetwork.h"

InterfaceItem::InterfaceItem(Solid::Control::NetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, NameDisplayMode mode, QGraphicsItem * parent) : QGraphicsWidget(parent), m_iface(iface), m_userSettings(userSettings), m_systemSettings(systemSettings), m_nameMode(mode), m_connectionInspector(0)
{
#if 0 // this layouts pretty badly, esp where labels and icons share a horizontal layout
    // main layout
    m_layout = new QGraphicsLinearLayout(Qt::Horizontal, this);
    //   icon on left
    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("network-wireless");
    //   vertical stack of info in middle
    m_infoLayout = new QGraphicsLinearLayout(Qt::Vertical, 0);
    //     interface layout
    m_ifaceNameLabel = new Plasma::Label(this);
    m_ifaceNameLabel->setText(QString::fromLatin1("<b>%1</b>").arg(m_iface->interfaceName()));
    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText(i18n("Active connection name"));
    //     connection info
    m_connectionInfoLayout = new QGraphicsLinearLayout(Qt::Horizontal, 0);
    //       IP address
    m_connectionInfoLabel = new Plasma::Label(this);
    m_connectionInfoLabel->setText("192.168.0.8");
    //       signal strength
    m_connectionInfoStrengthLabel = new Plasma::Label(this);
    m_connectionInfoStrengthLabel->setText("101%");
    //       security
    m_connectionInfoIcon = new Plasma::IconWidget(this);
    m_connectionInfoIcon->setIcon("system-lock-screen");
    m_connectionInfoLayout->addItem(m_connectionInfoStrengthLabel);
    m_connectionInfoLayout->addItem(m_connectionInfoIcon);
    m_infoLayout->addItem(m_ifaceNameLabel);
    m_infoLayout->addItem(m_connectionNameLabel);
    m_infoLayout->addItem(m_connectionInfoLabel);
    m_infoLayout->addItem(m_connectionInfoLayout);
    // connect button at right
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setIcon("media-playback-start");
    m_layout->addItem(m_icon);
    m_layout->addItem(m_infoLayout);
    m_layout->addItem(m_connectButton);
    setLayout(m_layout);
#else
    m_layout = new QGraphicsGridLayout(this);
    m_layout->setVerticalSpacing(0);
    m_layout->setHorizontalSpacing(8);
    m_layout->setColumnFixedWidth(0, 48);
    m_layout->setColumnPreferredWidth(1, 100);

    m_icon = new Plasma::IconWidget(this);
    m_icon->setMinimumHeight(48);
    m_icon->setMaximumHeight(48);
    m_layout->addItem(m_icon, 0, 0, 3, 1);


    switch (m_iface->type() ) {
        case Solid::Control::NetworkInterface::Ieee8023:
            m_icon->setIcon("network-wired");
            break;
        case Solid::Control::NetworkInterface::Ieee80211:
            m_icon->setIcon("network-wireless");
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
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 3);

    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText("<b>Connection:</b>");
    m_connectionNameLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionNameLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_connectionNameLabel, 1, 1, 1, 3);


    //       IP address
    m_connectionInfoLabel = new Plasma::Label(this);
    m_connectionInfoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionInfoLabel->nativeWidget()->setWordWrap(false);
    m_connectionInfoLabel->setText("<b>IP Address:</b> dum.my.ip.addr");

    //       security
    m_connectionInfoIcon = new Plasma::IconWidget(this);
    //m_connectionInfoIcon->setIcon("system-lock-screen");
    m_connectionInfoIcon->setMinimumHeight(22);
    m_connectionInfoIcon->setMaximumHeight(22);
    m_layout->addItem(m_connectionInfoLabel, 2, 1, 1, 1);
    //m_layout->addItem(m_connectionInfoStrengthLabel, 2, 2, 1, 1);
    m_layout->addItem(m_connectionInfoIcon, 1, 3, 1, 1);
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setMinimumHeight(24);
    m_connectButton->setMaximumHeight(24);
    //m_connectButton->setIcon("media-playback-start");
    m_layout->addItem(m_connectButton, 0, 4, 3, 1);
#endif
    connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(activeConnectionsChanged()),
            SLOT(activeConnectionsChanged()));
    connect(m_iface, SIGNAL(connectionStateChanged(int)),
            SLOT(connectionStateChanged(int)));
    connect(m_connectButton, SIGNAL(clicked()),
            SLOT(connectButtonClicked()));

    setNameDisplayMode(mode);
    // the applet may be starting when NetworkManager is already connected,
    // so initialise the list of active connections
    if (m_activeConnections.isEmpty()) {
        activeConnectionsChanged();
    }
    // set the state of our UI correctly
    connectionStateChanged(m_iface->connectionState());
}

InterfaceItem::~InterfaceItem()
{
}


void InterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if ( m_nameMode == InterfaceName ) {
        m_ifaceNameLabel->setText(QString::fromLatin1("<b>Device: %1</b>").arg(m_iface->interfaceName()));
    } else {
        m_ifaceNameLabel->setText(QString::fromLatin1("<b>Hardware name goes here</b>"));
    }
}

InterfaceItem::NameDisplayMode InterfaceItem::nameDisplayMode() const
{
    return m_nameMode;
}

void InterfaceItem::setConnectionInfo()
{
    if (m_iface->connectionState() == Solid::Control::NetworkInterface::Activated) {
        Solid::Control::IPv4Config ip4Config = m_iface->ipV4Config();
        QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
        if (addresses.isEmpty()) {
            m_connectionInfoLabel->setText("ip display error");
        } else {
            // FIXME: doesn't seem to work?
            QHostAddress addr(addresses.first().address());
            m_connectionInfoLabel->setText("IP:" + addr.toString());
            kDebug() << "IP:" << addr.toString();
        }
    }
}

void InterfaceItem::activeConnectionsChanged()
{
    kDebug();
    QList<QPair<QDBusObjectPath, RemoteConnection*> > newConnectionList;
    QStringList activeConnections = Solid::Control::NetworkManager::activeConnections();
    QString serviceName;
    QDBusObjectPath connectionObjectPath;
    // find the active connection on this device
    foreach (QString conn, activeConnections) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface candidate(NM_DBUS_SERVICE,
                conn, QDBusConnection::systemBus(), 0);
        foreach (QDBusObjectPath path, candidate.devices()) {
            if (path.path() == m_iface->uni()) {
                // this device is using the connection
                serviceName = candidate.serviceName();
                connectionObjectPath = candidate.connection();

                NetworkManagerSettings * service = 0;
                if (serviceName == NM_DBUS_SERVICE_USER_SETTINGS) {
                    service = m_userSettings;
                }
                if (serviceName == NM_DBUS_SERVICE_SYSTEM_SETTINGS) {
                    service = m_systemSettings;
                }
                if (service && service->isValid()) { // it's possible that the service is no longer running
                                                     // but provided a connection in the past
                    RemoteConnection * connection = service->findConnection(connectionObjectPath.path());
                    newConnectionList.append(QPair<QDBusObjectPath, RemoteConnection*>(connectionObjectPath, connection));
                }
            }
        }
    }
    m_activeConnections = newConnectionList;
    // update our UI
    setConnectionInfo();
}

void InterfaceItem::connectionStateChanged(int state)
{
    // get the active connections
    // check if any of them affect our interface
    // setActiveConnection on ourself

    // if this connection is not there is an active connection on this interface
    switch (state) {
        case Solid::Control::NetworkInterface::Unavailable:
            setUnavailable();
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            KNotification::event(Event::Disconnected, i18nc("Notification text when a network interface was disconnected","Network interface %1 disconnected", m_iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
            setInactive();
            break;
        case Solid::Control::NetworkInterface::Failed:
            // set the disconnected icon
            KNotification::event(Event::ConnectFailed, i18nc("Notification text when a network interface connection attempt failed","Connection on Network interface %1 failed", m_iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
            setInactive();
            break;
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
            setActiveConnection(state);
            break;
        case Solid::Control::NetworkInterface::Activated: // lookup the active connection, get its state
            KNotification::event(Event::Connected, i18nc("Notification text when a network interface connection succeeded","Network interface %1 connected", m_iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
            setActiveConnection(state);
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::UnknownState:
            break;
    }
}

void InterfaceItem::connectButtonClicked()
{
    kDebug();
    switch ( m_iface->connectionState()) {
        case Solid::Control::NetworkInterface::Unavailable:
            // impossible, but nothing to do
            break;
        case Solid::Control::NetworkInterface::Disconnected:
        case Solid::Control::NetworkInterface::Failed:
            kDebug() << "TODO: implement activating the default connection";
            break;
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
        case Solid::Control::NetworkInterface::Activated: // deactivate active connections
            foreach ( ActiveConnectionPair connection, m_activeConnections) {
                Solid::Control::NetworkManager::deactivateConnection(connection.first.path());
            }
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::UnknownState:
            break;
    }
}

void InterfaceItem::setUnavailable()
{
    m_icon->setEnabled(false);
    m_connectionNameLabel->setText(i18nc("Label for network interfaces that cannot be activated", "Unavailable"));
    m_connectionInfoLabel->setText("unavailable");
    m_connectButton->setEnabled(false);
}

void InterfaceItem::setInactive()
{
    m_icon->setEnabled(false);
    m_connectionNameLabel->setText("empty name");
    m_connectionInfoLabel->setText("empty info");
    m_connectButton->setIcon("media-playback-stop");
}


void InterfaceItem::setActiveConnection(int state)
{
    m_icon->setEnabled(true);
    m_connectButton->setEnabled(true);
    m_connectButton->setIcon("media-playback-stop");
    QStringList connectionIds;
    foreach (ActiveConnectionPair connection, m_activeConnections) {
        if (connection.second->isValid()) {
            //connection name
            connectionIds.append(connection.second->id());
        } else {
            // fallback label
            connectionIds.append(i18nc("Text for connections not owned by a service", "Orphaned connection"));
        }
    }
    QString stateString;
    if (!connectionIds.isEmpty()) {
        switch (state) {
            case Solid::Control::NetworkInterface::Preparing:
                stateString = i18nc("description of preparing to connect state followed by list of active connections in ()", "Preparing to connect (%1)"), connectionIds.join(QChar(','));
                break;
            case Solid::Control::NetworkInterface::Configuring:
                stateString = i18nc("description of configuring hardware state followed by list of active connections in ()", "Configuring interface (%1)"), connectionIds.join(QChar(','));
                break;
            case Solid::Control::NetworkInterface::NeedAuth:
                stateString = i18nc("description of waiting for authentication state followed by list of active connections in ()", "Waiting for authorization (%1)"), connectionIds.join(QChar(','));
                break;
            case Solid::Control::NetworkInterface::IPConfig:
                stateString = i18nc("description of setting IP address state followed by list of active connections in ()", "Setting network address (%1)"), connectionIds.join(QChar(','));
                break;
            case Solid::Control::NetworkInterface::Activated:
            default:
                stateString = QString::fromLatin1("<b>Network:</b> %1").arg(connectionIds.join(QChar(',')));
                break;
        }
    }
    m_connectionNameLabel->setText(stateString);
    setConnectionInfo();
}

void InterfaceItem::setConnectionInspector(ConnectionInspector * insp)
{
    delete m_connectionInspector;
    m_connectionInspector = insp;
}

ConnectionInspector * InterfaceItem::connectionInspector() const
{
    return m_connectionInspector;
}

// vim: sw=4 sts=4 et tw=100
