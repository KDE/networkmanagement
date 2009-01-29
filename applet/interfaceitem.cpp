/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>

#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>

#include "connectioninspector.h"
#include "events.h"
#include "nm-active-connectioninterface.h"
#include "networkmanager.h"
#include "networkmanagersettings.h"
#include "remoteconnection.h"
#include "wirelessnetwork.h"

InterfaceItem::InterfaceItem(Solid::Control::NetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, NameDisplayMode mode, QGraphicsItem * parent) : QGraphicsWidget(parent), m_iface(iface), m_userSettings(userSettings), m_systemSettings(systemSettings), m_connectionInfoLabel(0), m_strengthMeter(0), m_nameMode(mode), m_enabled(false), m_connectionInspector(0), m_unavailableText(i18nc("Label for network interfaces that cannot be activated", "Unavailable"))
{
    m_layout = new QGraphicsGridLayout(this);
    m_layout->setVerticalSpacing(0);
    m_layout->setColumnSpacing(0, 8);
    m_layout->setColumnSpacing(1, 4);
    m_layout->setColumnSpacing(2, 4);
    m_layout->setPreferredWidth(300);
    m_layout->setColumnFixedWidth(0, 48);
    m_layout->setColumnMinimumWidth(1, 144);
    m_layout->setColumnFixedWidth(2, 60); // FIXME: spacing?
    m_layout->setColumnFixedWidth(3, 22); // FIXME: spacing?

    m_icon = new Plasma::IconWidget(this);
    m_icon->setMinimumHeight(48);
    m_icon->setMaximumHeight(48);
    m_icon->setAcceptHoverEvents(false);
    m_layout->addItem(m_icon, 0, 0, 2, 1);


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
    m_ifaceNameLabel->nativeWidget()->setWordWrap(false);
    //m_ifaceNameLabel->setMinimumWidth(176);
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 1);

    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText(i18n("[not updated yet]")); // TODO: check connection status
    m_connectionNameLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionNameLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_connectionNameLabel, 1, 1, 1, 2);

    //       IP address
    m_connectionInfoLabel = new Plasma::Label(this);
    m_connectionInfoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionInfoLabel->nativeWidget()->setWordWrap(false);
    m_connectionInfoLabel->setText(i18n("<b>IP Address:</b> dum.my.ip.addr"));
    m_layout->addItem(m_connectionInfoLabel, 2, 1, 1, 1, Qt::AlignCenter);

    if (m_iface->type() == Solid::Control::NetworkInterface::Ieee80211 ||
            m_iface->type() == Solid::Control::NetworkInterface::Cdma ||
            m_iface->type() == Solid::Control::NetworkInterface::Gsm ) {
        // Signal strength meter
        int meterHeight = 12;
        m_strengthMeter = new Plasma::Meter(this);
        m_strengthMeter->setMinimum(0);
        m_strengthMeter->setMaximum(100);
        m_strengthMeter->setValue(0);
        m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
        m_strengthMeter->setPreferredSize(QSizeF(60, meterHeight));
        m_strengthMeter->setMaximumWidth(60);
        m_strengthMeter->setMaximumHeight(meterHeight);
        m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_layout->addItem(m_strengthMeter, 2, 2, 1, 1, Qt::AlignCenter);
        //m_connectionInfoLabel->hide();

   // m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //m_layout->addItem(m_strengthMeter, 0, 1, 1, 1, Qt::AlignCenter);

    }

    //       security
    m_connectionInfoIcon = new Plasma::IconWidget(this);
    //m_connectionInfoIcon->setIcon("object-unlocked"); // FIXME: set correct icon on start
    m_connectionInfoIcon->setMinimumHeight(22);
    m_connectionInfoIcon->setMinimumWidth(22);
    m_connectionInfoIcon->setMaximumHeight(22);
    //m_connectionInfoIcon->setAcceptHoverEvents(false);
    //m_layout->addItem(m_connectionInfoStrengthLabel, 2, 2, 1, 1);
    m_layout->addItem(m_connectionInfoIcon, 2, 3, 1, 1, Qt::AlignRight);
    m_connectionInfoIcon->hide(); // hide by default, we'll enable it later

    connect(Solid::Control::NetworkManager::notifier(),
            SIGNAL(activeConnectionsChanged()),
            this, SLOT(activeConnectionsChanged()));
    // Since we don't keep the connection identifiers around, and the RemoteConnections
    // we hold are invalid when this signal is emitted, we rebuild the active connection
    // list from scratch
    connect(m_userSettings,
            SIGNAL(connectionRemoved(NetworkManagerSettings *, const QString&)),
            this, SLOT(activeConnectionsChanged()));
    connect(m_systemSettings,
            SIGNAL(connectionRemoved(NetworkManagerSettings *, const QString&)),
            this, SLOT(activeConnectionsChanged()));
    connect(m_iface, SIGNAL(connectionStateChanged(int)),
            this, SLOT(connectionStateChanged(int)));
    setNameDisplayMode(mode);
    // the applet may be starting when NetworkManager is already connected,
    // so initialise the list of active connections
    activeConnectionsChanged();
    // set the state of our UI correctly
    connectionStateChanged(m_iface->connectionState(), true);
    setLayout(m_layout);
    m_layout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(m_userSettings, SIGNAL(disappeared(NetworkManagerSettings*)), SLOT(serviceDisappeared(NetworkManagerSettings*)));
    connect(m_systemSettings, SIGNAL(disappeared(NetworkManagerSettings*)), SLOT(serviceDisappeared(NetworkManagerSettings*)));
}

void InterfaceItem::setEnabled(bool enable)
{
    kDebug() << enable;
    m_enabled = enable;
    m_connectionInfoLabel->setEnabled(enable);
    m_connectionNameLabel->setEnabled(enable);
    m_ifaceNameLabel->setEnabled(enable);
    m_connectionInfoIcon->setEnabled(enable);
    if (m_strengthMeter) {
        m_strengthMeter->setEnabled(enable);
    }
}

InterfaceItem::~InterfaceItem()
{
    delete m_connectionInspector;
}


void InterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if ( m_nameMode == InterfaceName ) {
        m_ifaceNameLabel->setText(i18n("<b>Interface %1</b>", m_iface->interfaceName()));
    } else {
        m_ifaceNameLabel->setText(i18nc("network interface name unknown", "<b>Unknown Network Interface</b>"));
    }
}

InterfaceItem::NameDisplayMode InterfaceItem::nameDisplayMode() const
{
    return m_nameMode;
}

QString InterfaceItem::ssid()
{
    return QString("Blaat");
}

void InterfaceItem::setConnectionInfo()
{
    if (m_iface->connectionState() == Solid::Control::NetworkInterface::Activated) {
        Solid::Control::IPv4Config ip4Config = m_iface->ipV4Config();
        QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
        if (addresses.isEmpty()) {
            m_connectionInfoLabel->setText(i18n("ip display error"));
        } else {
            QHostAddress addr(addresses.first().address());
            QString ip = addr.toString();
            m_connectionNameLabel->setText(i18nc("wireless interface is connected", "Connected"));
            m_connectionInfoLabel->setText(i18nc("ip address of the network interface", "IP: %1", ip));
        }
    }
}
void InterfaceItem::activeConnectionsChanged()
{
    kDebug() << "-------------------- updating active connection list for " << m_iface->uni();
    QList<ActiveConnectionPair > newConnectionList;
    QStringList activeConnections = Solid::Control::NetworkManager::activeConnections();
    kDebug() << activeConnections;
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
                    //kDebug() << "looking up connection" << connectionObjectPath.path() << "on" << service->objectName();
                    RemoteConnection * connection = service->findConnection(connectionObjectPath.path());
                    if (connection) {
                        //kDebug() << "found it";
                        newConnectionList.append(ActiveConnectionPair(conn, connection));
                    } else {
                        //kDebug() << "not found";
                    }
                }
            }
        }
    }
    m_activeConnections = newConnectionList;
    if (!m_activeConnections.isEmpty()) {
        kDebug() << "Active connections:";
        foreach ( ActiveConnectionPair connection, m_activeConnections) {
            kDebug() << connection.first << connection.second->path();
        }
    } else {
        kDebug() << "Interface has no active connections";
    }
    // update our UI
    m_layout->invalidate();
    setConnectionInfo();
}

// slot
void InterfaceItem::connectionStateChanged(int state )
{
    connectionStateChanged( state, false );
}

void InterfaceItem::connectionStateChanged(int state, bool silently)
{
    // get the active connections
    // check if any of them affect our interface
    // setActiveConnection on ourself

    switch (state) {
        case Solid::Control::NetworkInterface::Unavailable:
            setUnavailable();
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            if ( !silently )
                KNotification::event(Event::Disconnected, i18nc("Notification text when a network interface was disconnected","Network interface %1 disconnected", m_iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
            setInactive();
            break;
        case Solid::Control::NetworkInterface::Failed:
            // set the disconnected icon
            if ( !silently )
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
            if ( !silently )
                KNotification::event(Event::Connected, i18nc("Notification text when a network interface connection succeeded","Network interface %1 connected", m_iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
            setActiveConnection(state);
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::UnknownState:
            break;
    }
    kDebug() << "EMIT";
    emit stateChanged();
}

void InterfaceItem::setUnavailable()
{
    m_icon->setEnabled(false);
    m_ifaceNameLabel->setText(i18n("<b>Interface %1</b>", m_iface->interfaceName()));
    m_connectionNameLabel->setText(m_unavailableText);
    m_connectionInfoLabel->setText("");
    m_connectionInfoIcon->hide();
    if (m_strengthMeter) {
        m_strengthMeter->hide();
    }
}

void InterfaceItem::setInactive()
{
    m_icon->setEnabled(false);
    m_connectionNameLabel->setText(i18nc("networking device is not connected", "Disconnected"));
    m_connectionInfoLabel->setText("");
    m_connectionInfoIcon->hide();
    if (m_strengthMeter) {
        m_strengthMeter->hide();
    }
}

void InterfaceItem::setActiveConnection(int state)
{
    m_icon->setEnabled(true);
    QStringList connectionIds;
    //kDebug();
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
        QString connId = connectionIds.join(QChar(','));
        stateString = NetworkManagerApplet::connectionStateToString((Solid::Control::NetworkInterface::ConnectionState)state);
        m_ifaceNameLabel->setText("<b>" + connId + "</b>");
    }
    m_connectionNameLabel->setText(stateString);
    setConnectionInfo();
    m_connectionInfoIcon->show();
    if (m_strengthMeter) {
        m_strengthMeter->show();
    }
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

void InterfaceItem::serviceDisappeared(NetworkManagerSettings* service)
{
    QMutableListIterator<ActiveConnectionPair> i(m_activeConnections);
    while (i.hasNext()) {
        i.next();
        if (i.value().first == service->service()) {
            i.remove();
        }
    }
    setConnectionInfo();
}

QList<RemoteConnection*> InterfaceItem::availableConnections() const
{
    QList<RemoteConnection*> rconnections;
    foreach (QString conn, m_userSettings->connections()) {
       RemoteConnection *rconnection = m_userSettings->findConnection(conn);
       if (rconnection && rconnection->type() == m_iface->type()) {
           rconnections << rconnection;
       }
    }
    return rconnections;
}

// vim: sw=4 sts=4 et tw=100
