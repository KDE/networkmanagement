/*
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

#include "wirelessconnectionitem.h"
#include <nm-setting-wireless.h>

#include <QLabel>
#include <QGraphicsGridLayout>

#include <KGlobalSettings>
//#include "../libs/types.h"

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>
#include <plasma/theme.h>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "remoteconnection.h"
#include "wirelessnetwork.h"

WirelessConnectionItem::WirelessConnectionItem(RemoteConnection * conn, QGraphicsItem * parent)
: ConnectionItem(conn, parent), m_connection(conn), m_wirelessNetwork(0), m_strengthMeter(0), m_securityIcon(0), m_strength(0)
{
}

void WirelessConnectionItem::setupItem()
{
    readSettings();
    //kDebug() << "Connection Settings:" << m_connection->settings();
    //kDebug() << "Security:" << m_connection->type() << m_security;
    // painting of a non-active wifi network
    /*
    +----+---------+---+-----+--+
    |icon| essid   |sec|meter|on|
    +----+---------+---+-----+--+
    */
    // icon on the left
    int rowHeight = 24;
    int spacing = 4;
    m_layout = new QGraphicsGridLayout(this);
    // First, third and fourthcolunm are fixed width for the icons
    m_layout->setColumnFixedWidth(0, rowHeight);
    m_layout->setColumnPreferredWidth(1, 100);
    m_layout->setColumnFixedWidth(2, rowHeight);
    m_layout->setColumnFixedWidth(3, 60);
    m_layout->setColumnFixedWidth(4, rowHeight);
    // tighten
    m_layout->setColumnSpacing(0, spacing);
    m_layout->setColumnSpacing(1, spacing);
    m_layout->setColumnSpacing(2, spacing);
    m_layout->setColumnSpacing(3, spacing);

    // TODO: security symbol

    m_icon = new Plasma::IconWidget(this);
    m_icon->setIcon("network-wireless");
    m_icon->setMinimumHeight(rowHeight);
    m_icon->setMaximumHeight(rowHeight);
    m_layout->addItem(m_icon, 0, 0, 1, 1 );

    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText(m_connection->id());
    m_connectionNameLabel->nativeWidget()->setWordWrap(false);
    m_connectionNameLabel->setMaximumWidth(200);
    m_layout->addItem(m_connectionNameLabel, 0, 1, 1, 1);

    //kDebug() << "security icon:" << m_securityIconName;
    m_securityIcon = new Plasma::IconWidget(this);
    m_securityIcon->setIcon(m_securityIconName);
    m_securityIcon->setMinimumHeight(22);
    m_securityIcon->setMaximumHeight(22);
    m_layout->addItem(m_securityIcon, 0, 2, 1, 1 );

    m_strengthMeter = new Plasma::Meter(this);
    m_strengthMeter->setMinimum(0);
    m_strengthMeter->setMaximum(100);
    m_strengthMeter->setValue(m_strength);

    m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_strengthMeter->setPreferredSize(QSizeF(60, rowHeight/2));
    m_strengthMeter->setMaximumHeight(rowHeight/2);
    m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->addItem(m_strengthMeter, 0, 3, 1, 1, Qt::AlignCenter);

    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setIcon("media-playback-start");
    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);
    m_layout->addItem(m_connectButton, 0, 4, 1, 1, Qt::AlignLeft);

    connect( m_connectButton, SIGNAL(clicked()), SLOT(emitClicked()));
}

WirelessConnectionItem::~WirelessConnectionItem()
{
}

void WirelessConnectionItem::setNetwork(AbstractWirelessNetwork * network)
{
    Q_ASSERT(network);
    m_wirelessNetwork = network;
    setStrength(network->ssid(), network->strength());
    connect(m_wirelessNetwork, SIGNAL(strengthChanged(const QString&, int)), SLOT(setStrength(const QString&, int)));
}

void WirelessConnectionItem::setStrength(QString ssid, int strength)
{
    Q_UNUSED(ssid);
    kDebug() << ssid << "signal strength changed to " << strength;
    if (strength == m_strength) {
        return;
    }
    m_strength = strength;
    if (m_strengthMeter) {
        m_strengthMeter->setValue(m_strength);
    }
}


QString WirelessConnectionItem::ssid()
{
    return m_ssid;
}

void WirelessConnectionItem::readSettings() {
    // from wirelessinterfaceitem, TODO: share this code in WirelessNetwork?
    QVariantMapMap settings = m_connection->settings();
    if ( settings.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME))) {
        QVariantMap connectionSetting = settings.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME));
        if (connectionSetting.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT))) {
            m_security = connectionSetting.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_KEY_MGMT)).toString();
        } else {
            m_security = "wep";
        }
    }
    if (m_security.isEmpty()) {
        m_securityIconName = "object-unlocked";
    } else if (m_security == QLatin1String("wep")) {
        // security-weak
        m_securityIconName = "object-locked";
    } else if (m_security == QLatin1String("wpa-psk")) {
        // security-medium
        m_securityIconName = "object-locked";
    } else if (m_security == QLatin1String("wpa-eap")) {
        // security-strong
        m_securityIconName = "object-locked";
    } else {
        m_securityIconName = "object-locked-finished"; // FIXME: Shouldn't we always have a security setting?
    }
}

// vim: sw=4 sts=4 et tw=100
