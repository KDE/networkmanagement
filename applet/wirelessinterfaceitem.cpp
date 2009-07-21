/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "wirelessinterfaceitem.h"

#include <nm-setting-wireless.h>
#include <nm-setting-connection.h>

#include <QGraphicsGridLayout>

#include <KNotification>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>

#include "../libs/types.h"
#include "events.h"

WirelessInterfaceItem::WirelessInterfaceItem(Solid::Control::WirelessNetworkInterface * iface,  InterfaceItem::NameDisplayMode mode, QGraphicsItem* parent)
: InterfaceItem(iface, mode, parent), m_wirelessIface(iface), m_activeAccessPoint(0)
{
    // for updating our UI
    connect(iface, SIGNAL(activeAccessPointChanged(const QString&)),
            SLOT(activeAccessPointChanged(const QString&)));

    activeAccessPointChanged(m_wirelessIface->activeAccessPoint());
}

WirelessInterfaceItem::~WirelessInterfaceItem()
{
}

void WirelessInterfaceItem::activeAccessPointChanged(const QString &uni)
{
    kDebug() << "*** AP changed:" << uni << "***";
    // this is not called when the device is deactivated..
    if (m_activeAccessPoint) {
        m_activeAccessPoint->disconnect(this);
        m_activeAccessPoint = 0;
    }
    if (uni != "/") {
        m_activeAccessPoint = m_wirelessIface->findAccessPoint(uni);
        kDebug() << "new:" << m_activeAccessPoint;
        if (m_activeAccessPoint) {
            connect(m_activeAccessPoint, SIGNAL(signalStrengthChanged(int)), SLOT(activeSignalStrengthChanged(int)));
            connect(m_activeAccessPoint, SIGNAL(destroyed(QObject*)),
                    SLOT(accessPointDestroyed(QObject*)));
        }
    }
    setConnectionInfo();
}

QString WirelessInterfaceItem::ssid()
{
    if (m_activeAccessPoint) {
        return m_activeAccessPoint->ssid();
    }
    return QString();
}

void WirelessInterfaceItem::activeSignalStrengthChanged(int)
{
    setConnectionInfo();
}

void WirelessInterfaceItem::accessPointDestroyed(QObject* ap)
{
    kDebug() << "*** AP gone ***";
    if (ap == m_activeAccessPoint) {
        m_activeAccessPoint = 0;
    }
}

void WirelessInterfaceItem::connectButtonClicked()
{
#if 0
    kDebug();
    QList<RemoteConnection*> connections;
    QList<Solid::Control::AccessPoint*> accesspoints;
    switch ( m_iface->connectionState()) {
        case Solid::Control::NetworkInterface::Unavailable:
            // impossible, but nothing to do
            break;
        case Solid::Control::NetworkInterface::Disconnected:
        case Solid::Control::NetworkInterface::Failed:
             kDebug() << "Activating default connection.";
             connections = availableConnections();
             accesspoints = availableAccessPoints();
             connections = appropriateConnections(connections, accesspoints);
             if (!connections.isEmpty()) {
                 //pick the first one.  TODO:Decide what to do if more than on connection is applicable.
                 Solid::Control::NetworkManager::activateConnection(m_wirelessIface->uni(), connections[0]->service() + " " + connections[0]->path(), QVariantMap());
                 KNotification::event(Event::Connecting, i18nc("Notification text when activating a connection","Connecting %1", connections[0]->id()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
             }
            break;
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
        case Solid::Control::NetworkInterface::Activated: // deactivate active connections
            foreach (const ActiveConnectionPair &connection, m_activeConnections) {
                kDebug() << "Deactivating connection" << connection.second->path() << connection;
                Solid::Control::NetworkManager::deactivateConnection(connection.first);
            }
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::UnknownState:
            break;
    }
#endif
}

void WirelessInterfaceItem::setConnectionInfo()
{
    InterfaceItem::setConnectionInfo(); // Needed for m_currentIp

    //kDebug() << m_activeAccessPoint;
    //kDebug() << m_activeConnections;
    if (m_activeAccessPoint) { // TODO this is called on activeConnectionsChanged - which seems to arrive before the destroyed() signal from the AP.
        //TODO: this needs more streamlining, hiding and showing when APs come and go
        if (m_strengthMeter) {
            m_strengthMeter->setValue(m_activeAccessPoint->signalStrength());
            m_strengthMeter->show();
        }
        // TODO update icon contents
#if 0
        QVariantMapMap settings;
        if (!m_activeConnections.isEmpty()) {
            QString security;
            foreach (const ActiveConnectionPair &conn, m_activeConnections) {
                if (!conn.second) {
                    continue;
                }
                settings = conn.second->settings();
                if ( settings.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME))) {
                    QVariantMap connectionSetting = settings.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME));
                    if (connectionSetting.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT))) {
                        security = connectionSetting.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)).toString();
                    } else {
                        security = "wep";
                    }
                }
                if (!security.isEmpty()) {
                    break;
                }
            }

            if (security.isEmpty()) {
                m_connectionInfoIcon->setIcon("security-low");
                m_connectionInfoIcon->setToolTip(i18nc("wireless network is not encrypted", "Unencrypted network"));
            } else if (security == QLatin1String("wep")) {
                // security-weak
                m_connectionInfoIcon->setIcon("security-medium");
                m_connectionInfoIcon->setToolTip(i18nc("tooltip of the security icon in the connection list", "Weakly encrypted network (WEP)"));
            } else if (security == QLatin1String("wpa-psk")) {
                // security-medium
                m_connectionInfoIcon->setToolTip(i18nc("tooltip of the security icon in the connection list", "Encrypted network (WPA-PSK)"));
                m_connectionInfoIcon->setIcon("security-high");
            } else if (security == QLatin1String("wpa-eap")) {
                // security-strong
                m_connectionInfoIcon->setToolTip(i18nc("tooltip of the security icon in the connection list", "Encrypted network (WPA-EAP)"));
                m_connectionInfoIcon->setIcon("security-high");
            }

            // retrieve the name of the connection, or put the ssid into the label
            QString _name;
            if (!settings.value(NM_SETTING_CONNECTION_SETTING_NAME).isEmpty()) {
                _name = settings.value(NM_SETTING_CONNECTION_SETTING_NAME).value(NM_SETTING_CONNECTION_ID).toString();
            } else {
                _name = m_activeAccessPoint->ssid();
            }

            m_connectionNameLabel->setText(i18n("Connected to \"%1\"", _name));
            //m_connectionInfoLabel->setText(i18n("Address: %1", m_currentIp));
        } else {
            //kDebug() << "Active connections is empty while connected?";
#endif
        m_connectionInfoIcon->show();
    } else {
        // No active accesspoint
        //m_connectionInfoLabel->setText(QString());
        m_connectionInfoIcon->hide();
        m_strengthMeter->hide();
    }
}

#if 0
QList<RemoteConnection*> WirelessInterfaceItem::appropriateConnections(const QList<RemoteConnection*> &connections, const QList<Solid::Control::AccessPoint*> accesspoints) const
{
    QList<RemoteConnection*> retVal;
    foreach (RemoteConnection *conn, connections) {
        QVariantMapMap settings = conn->settings();
        //deterine if the accesspoint can apply to the connection
        foreach (Solid::Control::AccessPoint *ap, accesspoints) {
            if (settings[QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)][QLatin1String(NM_SETTING_WIRELESS_SSID)] != ap->ssid()) {
                //kDebug() << settings[QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)][QLatin1String(NM_SETTING_WIRELESS_SSID)] << " != " << ap->ssid();
                //kDebug() << "Skipping . . . ";
                continue;
            } else if (!settings[QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)][QLatin1String(NM_SETTING_WIRELESS_BSSID)].toString().isEmpty() &&
                        settings[QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)][QLatin1String(NM_SETTING_WIRELESS_BSSID)] != ap->hardwareAddress()) {
                //kDebug() << settings[QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)][QLatin1String(NM_SETTING_WIRELESS_BSSID)] << " != " << ap->hardwareAddress();
                //kDebug() << "Skipping . . . ";
                continue;
            } else {
                //kDebug() << "Connection " << conn->path() << " is applicable.";
                if (!retVal.contains(conn)) { //prevent multiple includes
                    retVal << conn;
                }
            }
        }
    }
    return retVal;
}
#endif

QList<Solid::Control::AccessPoint*> WirelessInterfaceItem::availableAccessPoints() const
{
    QList<Solid::Control::AccessPoint*> retVal;
    AccessPointList aps = m_wirelessIface->accessPoints(); //NOTE: AccessPointList is a QStringList
    foreach (const QString &ap, aps) {
        Solid::Control::AccessPoint *accesspoint = m_wirelessIface->findAccessPoint(ap);
        if(accesspoint) {
            retVal << accesspoint;
        }
    }
    return retVal;
}

void WirelessInterfaceItem::setEnabled(bool enable)
{
    kDebug() << enable;
    m_strengthMeter->setEnabled(enable);
    InterfaceItem::setEnabled(enable);
}

// vim: sw=4 sts=4 et tw=100
