/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
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
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>

#include <Solid/Device>

#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>


//#include "connectioninspector.h"
#include "events.h"
//#include "nm-active-connectioninterface.h"
#include "networkmanager.h"


InterfaceItem::InterfaceItem(Solid::Control::NetworkInterface * iface, NameDisplayMode mode, QGraphicsItem * parent) : QGraphicsWidget(parent),
    m_iface(iface),
    m_connectionNameLabel(0),
    m_connectionInfoLabel(0),
    m_strengthMeter(0),
    m_nameMode(mode),
    m_enabled(false),
    m_unavailableText(i18nc("Label for network interfaces that cannot be activated", "Unavailable"))
{
    setAcceptHoverEvents(true);

    Solid::Device* dev = new Solid::Device(m_iface->uni());
    m_interfaceName = dev->product();

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setVerticalSpacing(0);
    m_layout->setColumnSpacing(0, 8);
    m_layout->setColumnSpacing(1, 4);
    m_layout->setColumnSpacing(2, 6);
    m_layout->setPreferredWidth(240);
    m_layout->setColumnFixedWidth(0, 48);
    m_layout->setColumnMinimumWidth(1, 104);
    m_layout->setColumnFixedWidth(2, 60); // FIXME: spacing?
    m_layout->setColumnFixedWidth(3, 22); // FIXME: spacing?

    m_icon = new Plasma::IconWidget(this);
    m_icon->setMinimumHeight(48);
    m_icon->setMaximumHeight(48);
    m_icon->setAcceptHoverEvents(true);
    connect(m_icon, SIGNAL(clicked()), this, SLOT(itemClicked()));
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
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 2);

    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setMaximumHeight(22);
    m_connectButton->setMaximumWidth(22);
    m_connectButton->setIcon("dialog-ok");
    m_connectButton->setToolTip(i18n("Connect wireless"));
    m_connectButton->hide(); // Shown when hovered

    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(connectButtonClicked()));

    m_layout->addItem(m_connectButton, 0, 3, 1, 1, Qt::AlignRight);

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
    m_layout->addItem(m_connectionInfoLabel, 2, 1, 1, 2, Qt::AlignCenter);

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
        m_strengthMeter->setPreferredSize(QSizeF(48, meterHeight));
        m_strengthMeter->setMaximumWidth(48);
        m_strengthMeter->setMaximumHeight(meterHeight);
        m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_strengthMeter->hide();
        m_layout->addItem(m_strengthMeter, 2, 0, 1, 1, Qt::AlignCenter);
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

//X     connect(Solid::Control::NetworkManager::notifier(),
//X             SIGNAL(activeConnectionsChanged()),
//X             this, SLOT(activeConnectionsChanged()));
//
    connect(m_iface, SIGNAL(connectionStateChanged(int)),
            this, SLOT(connectionStateChanged(int)));
    QObject::connect(m_iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
    connect(m_iface, SIGNAL(linkUpChanged(bool)), this, SLOT(setActive(bool)));

    //RemoteInterfaceConnection * ric = static_cast<RemoteInterfaceConnection*>(activatable);
    Solid::Control::WiredNetworkInterface* wirediface = static_cast<Solid::Control::WiredNetworkInterface*>(m_iface);
    if (wirediface) {
        connect(m_iface, SIGNAL(carrierChanged(bool)), this, SLOT(setActive(bool)));
        kDebug() << "CONNECTED Carrier !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    }
    setNameDisplayMode(mode);
    // the applet may be starting when NetworkManager is already connected,
    // so initialise the list of active connections
    // activeConnectionsChanged();
    // set the state of our UI correctly
    //
    connectionStateChanged(m_iface->connectionState());
    setLayout(m_layout);
    m_layout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void InterfaceItem::itemClicked()
{
    emit clicked(m_iface->uni());
}

void InterfaceItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event )
    if (m_icon->isEnabled()) {
        //m_connectButton->show();
    }
}

void InterfaceItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED( event )
    m_connectButton->hide();
}

QString InterfaceItem::label()
{
    return m_ifaceNameLabel->text();
}

void InterfaceItem::setActive(bool active)
{
    kDebug() << "+ + + + + + Active?" << active;
    connectionStateChanged(m_iface->connectionState());
}

void InterfaceItem::setEnabled(bool enable)
{
    //kDebug() << "ENABLE?" << enable;
    m_enabled = enable;
    m_icon->setEnabled(enable);
    m_connectionInfoLabel->setEnabled(enable);
    m_connectionNameLabel->setEnabled(enable);
    m_ifaceNameLabel->setEnabled(enable);
    m_connectButton->setEnabled(enable);
    m_connectionInfoIcon->setEnabled(enable);
    if (m_strengthMeter) {
        if (enable) {
            m_strengthMeter->show();
        } else {
            m_strengthMeter->hide();
        }
    }
}

InterfaceItem::~InterfaceItem()
{
}

void InterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if ( m_nameMode == InterfaceName ) {
        Solid::Device* dev = new Solid::Device(m_iface->uni());
        kDebug() << "Product:" << dev->product();
        m_ifaceNameLabel->setText(i18n("<b>%1</b>", dev->product()));
    } else {
        m_ifaceNameLabel->setText(i18nc("network interface name unknown", "<b>Unknown Network Interface</b>"));
    }
}

InterfaceItem::NameDisplayMode InterfaceItem::nameDisplayMode() const
{
    return m_nameMode;
}

QString InterfaceItem::connectionName()
{
    // Default active connection's name is empty, room for improvement?
    return QString();
}

void InterfaceItem::setConnectionInfo()
{
    connectionStateChanged(m_iface->connectionState());
    return;
    if (m_connectionInfoLabel && m_connectionNameLabel) {
        if (m_iface->connectionState() == Solid::Control::NetworkInterface::Activated) {
            if (connectionName().isEmpty()) {
                m_connectionNameLabel->setText(i18nc("wireless interface is connected", "Connected"));
            } else {
                m_connectionNameLabel->setText(i18nc("wireless interface is connected", "Connected to %1", connectionName()));
            }
            m_connectionInfoLabel->setText(i18nc("ip address of the network interface", "Address: %1", currentIpAddress()));
            //kDebug() << "addresses non-empty" << m_currentIp;
            if (m_strengthMeter) {
                m_strengthMeter->show();
            }
        } else {
            if (m_strengthMeter) {
                m_strengthMeter->hide();
            }
        }
    }
}

QString InterfaceItem::currentIpAddress()
{
    if (m_iface->connectionState() != Solid::Control::NetworkInterface::Activated) {
        return i18n("No IP address.");
    }
    Solid::Control::IPv4Config ip4Config = m_iface->ipV4Config();
    QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
    if (addresses.isEmpty()) {
        return i18n("IP display error.");
    }
    QHostAddress addr(addresses.first().address());
    return addr.toString();
}

void InterfaceItem::activeConnectionsChanged()
{
#if 0
    QList<ActiveConnectionPair > newConnectionList;
    QStringList activeConnections = Solid::Control::NetworkManager::activeConnections();
    QString serviceName;
    QDBusObjectPath connectionObjectPath;
    //kDebug() << "... updating active connection list for " << m_iface->uni() << m_iface->interfaceName();
    // find the active connection on this device
    foreach (const QString &conn, activeConnections) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface candidate(NM_DBUS_SERVICE,
                                                                        conn, QDBusConnection::systemBus(), 0);
        if (candidate.isValid()) { // in case the Solid backend is broken and returns bad paths.
            foreach (const QDBusObjectPath &path, candidate.devices()) {
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
                        if (connection) {
                            newConnectionList.append(ActiveConnectionPair(conn, connection));
                        }
                    }
                }
            }
        }
    }
    m_activeConnections = newConnectionList;
    if (!m_activeConnections.isEmpty()) {
        kDebug() << m_iface->interfaceName() << "Active connections:";
        foreach (const ActiveConnectionPair &connection, m_activeConnections) {
            kDebug() << connection.first << connection.second->path();
        }
    } else {
        kDebug() << m_iface->interfaceName() << "Interface has no active connections";
    }
    // update our UI
    m_layout->updateGeometry();
    //kDebug() << "Active connections changed ... setting connection info";
    setConnectionInfo();
#endif
}

void InterfaceItem::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED(old_state);
    Q_UNUSED(reason);
    connectionStateChanged(new_state);
}

// slot
void InterfaceItem::connectionStateChanged(int state)
{
    // get the active connections
    // check if any of them affect our interface
    // setActiveConnection on ourself

    // button to connect, disconnect
    m_disconnect = false;
    // Name and info labels
    QString lname = QString();
    QString linfo = QString();

    switch (state) {
        case Solid::Control::NetworkInterface::Unavailable:
            lname = i18n("Unavailable");
            linfo = QString();
            setEnabled(false);
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            lname = i18n("Disconnected");
            linfo = QString();
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::Failed:
            // set the disconnected icon
            lname = i18nc("Notification text when a network interface connection attempt failed","Connection on %1 failed", m_interfaceName);
            linfo = i18n("Connection failed");
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::Preparing:
            lname = i18n("Connecting...");
            linfo = i18n("Preparing network connection");
            m_disconnect = true;
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::Configuring:
            lname = i18n("Connecting...");
            linfo = i18n("Configuring network connection");
            m_disconnect = true;
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::NeedAuth:
            lname = i18n("Connecting...");
            linfo = i18n("Requesting authentication");
            m_disconnect = true;
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::IPConfig:
            lname = i18n("Connecting...");
            linfo = i18n("Setting network address");
            setEnabled(true);
            m_disconnect = true;
            break;
        case Solid::Control::NetworkInterface::Activated:
            if (connectionName().isEmpty()) {
                lname = i18nc("wireless interface is connected", "Connected");
            } else {
                lname = i18nc("wireless interface is connected", "Connected to %1", connectionName());
            }
            linfo = i18nc("ip address of the network interface", "Address: %1", currentIpAddress());
            m_disconnect = true;
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
            lname = i18n("Unmanaged");
            linfo = QString();
            setEnabled(false);
            break;
        case Solid::Control::NetworkInterface::UnknownState:
            lname = i18n("Unknown");
            linfo = QString();
            setEnabled(false);
            break;
    }

    // Update connect button
    if (!m_disconnect) {
        m_connectButton->setIcon("dialog-ok");
        m_connectButton->setToolTip(i18n("Connect"));
    } else {
        m_connectButton->setIcon("dialog-cancel");
        m_connectButton->setToolTip(i18n("Disconnect"));
    }
    m_connectionNameLabel->setText(lname);
    m_connectionInfoLabel->setText(linfo);

    kDebug() << ">>>>>>> State changed" << lname << linfo;

    emit stateChanged();
}

QPixmap InterfaceItem::statePixmap(const QString &icon) {
    // Which pixmap should we display with the notification?
    return KIcon(icon).pixmap(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
}

// vim: sw=4 sts=4 et tw=100
