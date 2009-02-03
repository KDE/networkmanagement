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

#include "wirelessinterfaceitem.h"

#include <nm-setting-wireless.h>

#include <QGridLayout>
#include <QCheckBox>

#include <KNotification>
#include <KIconLoader>
#include <KDebug>
#include <KLocale>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>

#include "../libs/types.h"
#include "events.h"
#include "remoteconnection.h"
#include "wirelessenvironment.h"
#include "wirelessnetwork.h"

WirelessInterfaceItem::WirelessInterfaceItem(Solid::Control::WirelessNetworkInterface * iface, NetworkManagerSettings * userSettings, NetworkManagerSettings * systemSettings, InterfaceItem::NameDisplayMode mode, QWidget* parent)
: InterfaceItem(iface, userSettings, systemSettings, mode, parent), m_wirelessIface(iface), m_activeAccessPoint(0), m_environment(new WirelessEnvironment(iface, this))
{
    // for updating our UI
    connect(iface, SIGNAL(activeAccessPointChanged(const QString&)),
            SLOT(activeAccessPointChanged(const QString&)));

    activeAccessPointChanged(m_wirelessIface->activeAccessPoint());

    m_rfCheckBox = new QCheckBox(this);
    m_rfCheckBox->setChecked(Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
    m_rfCheckBox->setText(i18n("Enable"));
    m_rfCheckBox->setToolTip(i18nc("CheckBox to enable or disable wireless interface (rfkill)", "Enable Wireless"));
    m_layout->addWidget(m_rfCheckBox, 0, 2, 1, 2, Qt::AlignRight);
    connect(m_rfCheckBox, SIGNAL(toggled(bool)),
            SLOT(wirelessEnabledToggled(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));
}

WirelessInterfaceItem::~WirelessInterfaceItem()
{
}

WirelessEnvironment * WirelessInterfaceItem::wirelessEnvironment() const
{
    return m_environment;
}

void WirelessInterfaceItem::activeAccessPointChanged(const QString &uni)
{
    // this is not called when the device is deactivated..
    if (m_activeAccessPoint)
        m_activeAccessPoint->disconnect(this);
    m_activeAccessPoint = m_wirelessIface->findAccessPoint(uni);
    if (m_activeAccessPoint) {
        connect(m_activeAccessPoint, SIGNAL(signalStrengthChanged(int)), SLOT(activeSignalStrengthChanged(int)));
        connect(m_activeAccessPoint, SIGNAL(destroyed(QObject*)),
                SLOT(accessPointDestroyed(QObject*)));
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
    if (ap == m_activeAccessPoint) {
        m_activeAccessPoint = 0;
    }
}

void WirelessInterfaceItem::connectButtonClicked()
{
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
            foreach ( ActiveConnectionPair connection, m_activeConnections) {
                Solid::Control::NetworkManager::deactivateConnection(connection.first);
            }
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::UnknownState:
            break;
    }
}

void WirelessInterfaceItem::setConnectionInfo()
{
    InterfaceItem::setConnectionInfo();
    if (m_activeAccessPoint) {
        m_connectionInfoLabel->setText(QString::fromLatin1("<b>Signal Strength:</b> %1 %").arg(m_activeAccessPoint->signalStrength())); //TODO:
        if (m_strengthMeter) {
            m_strengthMeter->setValue(m_activeAccessPoint->signalStrength());
            m_strengthMeter->show();
        }
        // TODO update icon contents
        if (!m_activeConnections.isEmpty()) {
            QString security;
            foreach (ActiveConnectionPair conn, m_activeConnections) {
                if (!conn.second) {
                    continue;
                }
                QVariantMapMap settings = conn.second->settings();
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
                m_connectionInfoIcon->setPixmap(SmallIcon("object-unlocked"));
                m_connectionInfoIcon->setToolTip(i18n("Not secured"));
            } else if (security == QLatin1String("wep")) {
                // security-weak
                m_connectionInfoIcon->setPixmap(SmallIcon("object-locked"));
                m_connectionInfoIcon->setToolTip(i18n("WEP Encryption"));
             } else if (security == QLatin1String("wpa-psk")) {
                // security-medium
                m_connectionInfoIcon->setToolTip(i18n("WPA-PSK Encryption"));
                m_connectionInfoIcon->setPixmap(SmallIcon("object-locked"));
            } else if (security == QLatin1String("wpa-eap")) {
                // security-strong
                m_connectionInfoIcon->setToolTip(i18n("WPA-EAP Encryption"));
                m_connectionInfoIcon->setPixmap(SmallIcon("object-locked"));
            }
        } else {
            m_connectionInfoLabel->setText(i18n("No active connection"));
        }
        m_connectionInfoIcon->show();
    } else {
        m_connectionInfoLabel->setText(QString());
        //m_connectionInfoIcon->hide();
        m_strengthMeter->hide();
    }
}

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
    m_rfCheckBox->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
    m_strengthMeter->setEnabled(enable);
    InterfaceItem::setEnabled(enable);
}

void WirelessInterfaceItem::wirelessEnabledToggled(bool checked)
{
    kDebug() << "Applet wireless enable switch toggled" << checked;
    Solid::Control::NetworkManager::setWirelessEnabled(checked);
}

void WirelessInterfaceItem::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    m_rfCheckBox->setChecked(enabled);
}

void WirelessInterfaceItem::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_rfCheckBox->setChecked(enabled && Solid::Control::NetworkManager::isWirelessEnabled());
    m_rfCheckBox->setEnabled(!enabled);
}

bool WirelessInterfaceItem::isUsing(const AbstractWirelessNetwork * net) const
{
    if (m_activeAccessPoint) {
        return m_activeAccessPoint->ssid() == net->ssid();
    }
    return false;
}

// vim: sw=4 sts=4 et tw=100
