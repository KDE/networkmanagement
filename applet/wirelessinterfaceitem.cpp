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

#include <KNotification>
#include <KIconLoader>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

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
    m_activeAccessPoint->disconnect(this);
    m_activeAccessPoint = m_wirelessIface->findAccessPoint(uni);
    if (m_activeAccessPoint) {
        connect(m_activeAccessPoint, SIGNAL(signalStrengthChanged(int)), SLOT(activeSignalStrengthChanged(int)));
        connect(m_activeAccessPoint, SIGNAL(destroyed(QObject*)),
                SLOT(accessPointDestroyed(QObject*)));
    }
    setConnectionInfo();
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

void WirelessInterfaceItem::setConnectionInfo()
{
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
            } else if (security == QLatin1String("wep")) {
                // security-weak
                m_connectionInfoIcon->setPixmap(SmallIcon("object-locked"));
            } else if (security == QLatin1String("wpa-psk")) {
                // security-medium
                m_connectionInfoIcon->setPixmap(SmallIcon("object-locked"));
            } else if (security == QLatin1String("wpa-eap")) {
                // security-strong
                m_connectionInfoIcon->setPixmap(SmallIcon("object-locked"));
            }
        }
        m_connectionInfoIcon->show();
    } else {
        m_connectionInfoLabel->setText(QString());
        m_connectionInfoIcon->hide();
        m_strengthMeter->hide();
    }
}
// vim: sw=4 sts=4 et tw=100
