/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "wirelesspreferences.h"

#include <QVBoxLayout>
#include <QFile>

#include <KPluginFactory>
#include <KDebug>
#include <KLocale>
#include <KTabWidget>

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessaccesspoint.h>


#include "802_11_wirelesswidget.h"
#include "security/wirelesssecuritysettingwidget.h"
#include "ipv4widget.h"
#include "connectionwidget.h"

#include "connection.h"
#include "settings/802-11-wireless-security.h"
#include "settings/802-11-wireless.h"

#include <nm-setting-connection.h>
#include <nm-setting-wireless.h>

#include "wirelessnetworkinterfaceenvironment.h"

WirelessPreferences::WirelessPreferences(bool setDefaults, QWidget *parent, const QVariantList &args)
: ConnectionPreferences( KGlobal::mainComponent(), parent, args ), m_securityTabIndex(0)
{
    // at least 1
    Q_ASSERT(args.count());

    QString connectionId = args[0].toString();
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Wireless);

    QString ssid;
    QString deviceUni;
    QString apUni;

    if (args.count() == 3) {
        deviceUni = args[1].toString();
        apUni = args[2].toString();
        kDebug() << "DeviceUni" << deviceUni << "AP UNI" << apUni;
    } else {
        kDebug() << args;
    }

    Solid::Control::AccessPoint * ap = 0;
    Solid::Control::WirelessNetworkInterface * iface = 0;

    if (!deviceUni.isEmpty() && deviceUni != QLatin1String("/")) {
        iface = qobject_cast<Solid::Control::WirelessNetworkInterface*>(Solid::Control::NetworkManager::findNetworkInterface(deviceUni));
        if (iface) {
            if ( !apUni.isEmpty() && apUni != QLatin1String("/")) {
                ap = iface->findAccessPoint(apUni);
                if (ap) {
                    ssid = ap->ssid();
                }
            }
        }
    }

    QVBoxLayout * layout = new QVBoxLayout(this);
    m_contents = new ConnectionWidget(m_connection, (ssid.isEmpty() ? i18n("New Wireless Connection") : ssid), this);
    layout->addWidget(m_contents);
    m_wirelessWidget = new Wireless80211Widget(m_connection, ssid, this);
    m_securityWidget = new WirelessSecuritySettingWidget(m_connection, iface, ap, this);

    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);

    connect (m_contents->connectionSettingsWidget(), SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    addToTabWidget(m_wirelessWidget);
    m_securityTabIndex = addToTabWidget(m_securityWidget);
    addToTabWidget(ipv4Widget);

    if ( setDefaults )
    {
        // for defaults the security is most interesting
        m_contents->connectionSettingsWidget()->setCurrentIndex( 1 );
    }
}

WirelessPreferences::~WirelessPreferences()
{
}


bool WirelessPreferences::needsEdits() const
{
    return false;
}

void WirelessPreferences::tabChanged(int index)
{
    if (index == m_securityTabIndex) {
        Solid::Control::WirelessNetworkInterface * ifaceForSsid = 0;
        Solid::Control::AccessPoint * apForSsid = 0;
        // look up AP given by m_wirelessWidget, and set it on m_securityWidget
        QByteArray hwAddr = m_wirelessWidget->selectedInterfaceHardwareAddress();
        QString ssid = m_wirelessWidget->enteredSsid();
        if (!ssid.isEmpty()) {
            // find the S::C::WNI for this ssid
            // if hwAddr set, take that one
            // else, take the first one that can see this ssid
            foreach (Solid::Control::NetworkInterface * iface,
                    Solid::Control::NetworkManager::networkInterfaces()) {
                if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
                    Solid::Control::WirelessNetworkInterface * candidate = static_cast<Solid::Control::WirelessNetworkInterface*>(iface);
                    if (candidate->hardwareAddress() == hwAddr) {
                        ifaceForSsid = candidate;
                        break;
                    }
                    Solid::Control::WirelessNetworkInterfaceEnvironment env(candidate);

                    Solid::Control::WirelessNetwork * net = 0;
                    net = env.findNetwork(ssid);
                    if (net) {
                        QString apUni = net->referenceAccessPoint();
                        apForSsid = candidate->findAccessPoint(apUni);
                        ifaceForSsid = candidate;
                        break;
                    }
                }
            }
        }
        m_securityWidget->setIfaceAndAccessPoint(ifaceForSsid, apForSsid);
    }
}
// vim: sw=4 sts=4 et tw=100
