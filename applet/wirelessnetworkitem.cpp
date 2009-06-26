/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>
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

#include "wirelessnetworkitem.h"
#include <nm-setting-wireless.h>

#include <QLabel>
#include <QGraphicsGridLayout>

#include <KGlobalSettings>
#include <KIconLoader>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>
#include <plasma/theme.h>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "remotewirelessnetworkitem.h"

WirelessNetworkItem::WirelessNetworkItem(RemoteWirelessNetworkItem * remote, QGraphicsItem * parent)
: ActivatableItem(remote, parent), m_security(0), m_securityIcon(0), m_securityIconName(0)
{
    m_strengthMeter = new Plasma::Meter(this);
    m_strength = 0;
    m_ssid = remote->ssid();
    setStrength(remote->strength());
    connect(remote, SIGNAL(changed()), SLOT(update()));

    // TODO
    #if 0
    if ( ap->capabilities().testFlag( Solid::Control::AccessPoint::Privacy ) )
        m_security = QLatin1String("wep"); // the minimum
    #endif

    Solid::Control::AccessPoint::WpaFlags wpaFlags = remote->wpaFlags();
    Solid::Control::AccessPoint::WpaFlags rsnFlags = remote->rsnFlags();

    // TODO: this was done by a clueless (coolo)
    if ( wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep40 ) ||
         wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep104 ) )
        m_security = QLatin1String("wep");

    if ( wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
         wpaFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) )
        m_security = QLatin1String("wpa-psk");

    if ( rsnFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
         rsnFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) ||
         rsnFlags.testFlag( Solid::Control::AccessPoint::PairCcmp ) )
        m_security = QLatin1String("wpa-psk");

    if ( wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmt8021x ) ||
         wpaFlags.testFlag( Solid::Control::AccessPoint::GroupCcmp ) )
        m_security = QLatin1String("wpa-eap");
}

void WirelessNetworkItem::setupItem()
{
    readSettings();
    //kDebug();// << "Connection Settings:" << m_connection->settings();
    //kDebug();// << "Security:" << m_connection->type() << m_security;
    // painting of a non-active wifi network
    /*
    +----+-------------+-----+---+
    |icon essid        |meter|sec|
    +----+-------------+-----+---+
    */
    int rowHeight = 24;
    int spacing = 4;

    m_layout = new QGraphicsGridLayout(this);
    // First, third and fourth colunm are fixed width for the icons
    m_layout->setColumnPreferredWidth(0, 160);
    m_layout->setColumnFixedWidth(1, 60);
    m_layout->setColumnFixedWidth(2, rowHeight);
    m_layout->setColumnSpacing(1, spacing);

    // icon on the left
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setDrawBackground(true);
    m_connectButton->setIcon("network-wireless");
    m_connectButton->setText(m_ssid);
    m_connectButton->setMinimumWidth(160);
    m_connectButton->setOrientation(Qt::Horizontal);
#if KDE_IS_VERSION(4,2,60)
    m_connectButton->setTextBackgroundColor(QColor());
#endif
    //m_connectButton->setToolTip(i18nc("icon to connect to wireless network", "Connect to wireless network %1", ssid));
    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);
    m_layout->addItem(m_connectButton, 0, 0, 1, 1 );

    m_strengthMeter = new Plasma::Meter(this);
    m_strengthMeter->setMinimum(0);
    m_strengthMeter->setMaximum(100);
    m_strengthMeter->setValue(m_strength);
    m_strengthMeter->setMeterType(Plasma::Meter::BarMeterHorizontal);
    m_strengthMeter->setPreferredSize(QSizeF(60, rowHeight/2));
    m_strengthMeter->setMaximumHeight(rowHeight/2);
    m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->addItem(m_strengthMeter, 0, 1, 1, 1, Qt::AlignCenter);

    m_securityIcon = new Plasma::IconWidget(this);
    m_securityIcon->setIcon(m_securityIconName);
    m_securityIcon->setMinimumHeight(22);
    m_securityIcon->setMaximumHeight(22);
    m_securityIcon->setToolTip(m_securityIconToolTip);
    m_layout->addItem(m_securityIcon, 0, 2, 1, 1, Qt::AlignLeft);

    connect( m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));
}

WirelessNetworkItem::~WirelessNetworkItem()
{
}

void WirelessNetworkItem::setStrength(int strength)
{
    //kDebug() << ssid << "signal strength changed to " << strength;
    if (strength == m_strength) {
        return;
    }
    m_strength = strength;
    m_strengthMeter->setValue(m_strength);
}


void WirelessNetworkItem::readSettings()
{
    if (m_security.isEmpty()) {
        m_securityIconName = "security-low";
        m_securityIconToolTip = i18nc("wireless network is not encrypted", "Unencrypted network");
    } else if (m_security == QLatin1String("wep")) {
        // security-weak
        m_securityIconName = "security-medium";
        m_securityIconToolTip = i18nc("tooltip of the security icon in the connection list", "Weakly encrypted network (WEP)");
    } else if (m_security == QLatin1String("wpa-psk")) {
        // security-medium
        m_securityIconName = "security-high";
        m_securityIconToolTip = i18nc("tooltip of the security icon in the connection list", "Encrypted network (WPA-PSK)");
    } else if (m_security == QLatin1String("wpa-eap")) {
        // security-strong
        m_securityIconName = "security-high";
        m_securityIconToolTip = i18nc("tooltip of the security icon in the connection list", "Encrypted network (WPA-PSK)");
    } else {
        m_securityIconName = "security-low"; // FIXME: Shouldn't we always have a security setting?
        m_securityIconToolTip = i18nc("tooltip of the security icon in the connection list", "Encrypted network (WPA-EAP)");
    }
}

RemoteWirelessNetworkItem * WirelessNetworkItem::wirelessNetworkItem() const
{
    return static_cast<RemoteWirelessNetworkItem*>(m_activatable);
}

void WirelessNetworkItem::update()
{
    setStrength(wirelessNetworkItem()->strength());
}

// vim: sw=4 sts=4 et tw=100
