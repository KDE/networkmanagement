/*
Copyright 2009 Andrey Batyiev <batyiev@gmail.com>

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
#include <KLocale>
#include <kdeversion.h>

#include "connectioninfoipv4tab.h"

#include <uiutils.h>
#include <arpa/inet.h>

ConnectionInfoIPv4Tab::ConnectionInfoIPv4Tab(Solid::Control::NetworkInterface *iface, QWidget *parent)
    : QWidget(parent), m_iface(iface)
{
    QFormLayout *layout = new QFormLayout(this);
    setLayout(layout);

    m_ipAddressesLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox ip addresses of network interface", "IP Addresses:"), m_ipAddressesLabel);

    m_netmasksLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox netmasks of network interface", "Netmasks:"), m_netmasksLabel);

    m_gatewaysLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox gateways used by network interface", "Gateways:"), m_gatewaysLabel);

    m_nameserversLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox nameservers used by network interface", "Nameservers:"), m_nameserversLabel);

    m_domainsLabel = new QLabel(this);
    layout->addRow(i18nc("@label:textbox domains used by network interface", "Search domains:"), m_domainsLabel);

    connect(iface, SIGNAL(ipDetailsChanged()), this, SLOT(updateIpDetails()));
    connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(updateIpDetails()));
    updateIpDetails();
}

void ConnectionInfoIPv4Tab::updateIpDetails()
{
    const Solid::Control::IPv4Config &config = m_iface->ipV4Config();

    QStringList ipAddresses,
                netmasks,
                gateways;
    foreach(const Solid::Control::IPv4Address &addr, config.addresses()) {
        ipAddresses << QHostAddress(addr.address()).toString();
#if KDE_IS_VERSION(4, 3, 67)
        netmasks << QHostAddress((0xffffffff) << (32 - addr.netMask())).toString();
#else
        netmasks << QHostAddress((0xffffffff) << (32 - ntohl(addr.netMask()))).toString();
#endif
        gateways << QHostAddress(addr.gateway()).toString();
    }
    m_ipAddressesLabel->setText(ipAddresses.join("\n"));
    m_netmasksLabel->setText(netmasks.join("\n"));
    m_gatewaysLabel->setText(gateways.join("\n"));

    QStringList nameservers;
    foreach(const quint32 &addr, config.nameservers())
        nameservers << QHostAddress(ntohl(addr)).toString();

    m_nameserversLabel->setText(nameservers.join("\n"));

    m_domainsLabel->setText(config.domains().join("\n"));
}
