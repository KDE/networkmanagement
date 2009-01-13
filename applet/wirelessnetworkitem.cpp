/*
Copyright 2008 Sebastian Kügler <sebas@kde.org>
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

#include "wirelessnetworkitem.h"
#include <nm-setting-wireless.h>

#include <QLabel>
#include <QGridLayout>
#include <QGraphicsGridLayout>
#include <QProgressBar>
#include <QToolButton>

#include <KGlobalSettings>
#include <KIconLoader>

#include <plasma/theme.h>

#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "wirelessnetwork.h"

WirelessNetworkItem::WirelessNetworkItem(AbstractWirelessNetwork * network, QWidget * parent)
: AbstractConnectableItem(parent), m_wirelessNetwork(network), m_security(0), m_securityIcon(0), m_securityIconName(0)
{
    m_strengthMeter = new QProgressBar(this);
    m_strength = 0;
    m_ssid = network->ssid();
    setStrength(m_ssid, network->strength());
    connect(m_wirelessNetwork, SIGNAL(strengthChanged(const QString&, int)), SLOT(setStrength(const QString, int)));
    Solid::Control::AccessPoint *ap = network->referenceAccessPoint();

    if ( ap->capabilities().testFlag( Solid::Control::AccessPoint::Privacy ) )
        m_security = QLatin1String("wep"); // the minimum

    // TODO: this was done by a clueless (coolo)
    if ( ap->wpaFlags().testFlag( Solid::Control::AccessPoint::PairWep40 ) ||
         ap->wpaFlags().testFlag( Solid::Control::AccessPoint::PairWep104 ) )
        m_security = QLatin1String("wep");

    if ( ap->wpaFlags().testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
         ap->wpaFlags().testFlag( Solid::Control::AccessPoint::PairTkip ) )
        m_security = QLatin1String("wpa-psk");

    if ( ap->wpaFlags().testFlag( Solid::Control::AccessPoint::KeyMgmt8021x ) ||
         ap->wpaFlags().testFlag( Solid::Control::AccessPoint::GroupCcmp ) )
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
    QString ssid = m_ssid;
    int rowHeight = 24;
    int spacing = 4;
    m_layout = new QGridLayout(this);
    // First, third and fourth colunm are fixed width for the icons
//X    m_layout->setColumnPreferredWidth(0, 160);
//X    m_layout->setColumnFixedWidth(1, 60);
//X    m_layout->setColumnFixedWidth(2, rowHeight);
//X    m_layout->setColumnSpacing(1, spacing);

    // icon on the left
    m_connectButton = new QToolButton(this);
    m_connectButton->setIcon(MainBarIcon("network-connect"));
    m_connectButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_connectButton->setText(m_ssid);
    m_connectButton->setMinimumWidth(160);
    m_connectButton->setMinimumHeight(rowHeight);
    m_connectButton->setMaximumHeight(rowHeight);
    m_layout->addWidget(m_connectButton, 0, 0, 1, 1);

    m_strengthMeter->setMinimum(0);
    m_strengthMeter->setMaximum(100);
    m_strengthMeter->setValue(m_strength);
    m_strengthMeter->setMaximumHeight(rowHeight/2);
    m_strengthMeter->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_layout->addWidget(m_strengthMeter, 0, 1, 1, 1, Qt::AlignCenter);

    m_securityIcon = new QLabel(this);
    m_securityIcon->setPixmap(MainBarIcon(m_securityIconName));
    m_securityIcon->setMinimumHeight(22);
    m_securityIcon->setMaximumHeight(22);
    m_layout->addWidget(m_securityIcon, 0, 2, 1, 1, Qt::AlignLeft );

    connect( m_connectButton, SIGNAL(clicked()), this, SLOT(emitClicked()));
}

WirelessNetworkItem::~WirelessNetworkItem()
{
}

AbstractWirelessNetwork * WirelessNetworkItem::net() const
{
    return m_wirelessNetwork;
}

void WirelessNetworkItem::setStrength(QString ssid, int strength)
{
    Q_UNUSED(ssid);
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
