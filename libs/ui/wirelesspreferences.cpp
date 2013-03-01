/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010-2013 Lamarque V. Souza <lamarque@kde.org>

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

#include <QFile>

#include <KPluginFactory>
#include <KDebug>
#include <KLocale>
#include <KTabWidget>

#include <solid/control/networkmanager.h>

#include "802_11_wirelesswidget.h"
#include "security/wirelesssecuritysettingwidget.h"
#include "ipv4widget.h"
#include "ipv6widget.h"
#include "connectionwidget.h"

#include "connection.h"
#include "settings/802-11-wireless-security.h"
#include "settings/802-11-wireless.h"
#include "settings/ipv4.h"

#include <nm-setting-connection.h>
#include <nm-setting-wireless.h>

#include "wirelessnetworkinterfaceenvironment.h"

WirelessPreferences::WirelessPreferences(bool setDefaults, const QVariantList &args, QWidget *parent)
: ConnectionPreferences(args, parent), m_securityTabIndex(0)
{
    // at least 1
    Q_ASSERT(args.count());

    QString connectionId = args[0].toString();
    kDebug() << "connection ID from arg" << connectionId << QUuid(connectionId);
    m_connection = new Knm::Connection(QUuid(connectionId), Knm::Connection::Wireless);

    QString ssid;
    QString deviceUni;
    QString apUni;
    bool shared = false;

    if (args.count() >= 3) {
        deviceUni = args[1].toString();
        apUni = args[2].toString();
        kDebug() << "DeviceUni" << deviceUni << "AP UNI" << apUni;

        if (args.count() > 3 && args[3].toString() == QLatin1String("shared")) {
            static_cast<Knm::Ipv4Setting *>(m_connection->setting(Knm::Setting::Ipv4))->setMethod(Knm::Ipv4Setting::EnumMethod::Shared);
            shared = true;
            ssid = i18nc("ssid of the wifi connection. Use ASCII letters and underscore *only* (no spaces!). Leave it unstranslated if your language does not support ASCII characters", "Shared_Wireless_Connection");
            m_connection->setAutoConnect(false);
        }
    } else {
        kWarning() << "Could not find deviceUni or AP UNI in args:" << args;
    }

    Solid::Control::AccessPointNm09 * ap = 0;
    Solid::Control::WirelessNetworkInterfaceNm09 * iface = 0;

    if (!deviceUni.isEmpty() && deviceUni != QLatin1String("/")) {
        iface = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(Solid::Control::NetworkManagerNm09::findNetworkInterface(deviceUni));
        if (iface) {
            if ( !apUni.isEmpty() && apUni != QLatin1String("/")) {
                ap = iface->findAccessPoint(apUni);
                if (ap) {
                    ssid = ap->ssid();

                    /* To prevent Wireless80211Widget::readConfig() from changing mode
                       back to infrastructure. */
                    Knm::WirelessSetting * setting = static_cast<Knm::WirelessSetting *>(m_connection->setting(Knm::Setting::Wireless));
                    switch (ap->mode()) {
                    case Solid::Control::WirelessNetworkInterfaceNm09::Adhoc:
                        setting->setMode(Knm::WirelessSetting::EnumMode::adhoc);
                        break;
                    case Solid::Control::WirelessNetworkInterfaceNm09::ApMode:
                        setting->setMode(Knm::WirelessSetting::EnumMode::apMode);
                        break;
                    default:
                        setting->setMode(Knm::WirelessSetting::EnumMode::infrastructure);
                    }
                } else {
                    ssid = apUni;
                }
            }
        }
    } else if (shared) {
        // FIXME: we need to check the correct wifi device associated to this connection when creating it.
        // If there is only one wifi card in the system then the code below is correct but it may test the
        // wrong device if there is more than one.
        Solid::Control::WirelessNetworkInterfaceNm09 * w = 0;
        foreach (Solid::Control::NetworkInterfaceNm09 *d, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
            if (d->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
                w = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(d);
                if (w->wirelessCapabilities().testFlag(Solid::Control::WirelessNetworkInterfaceNm09::ApCap)) {
                    Knm::WirelessSetting * setting = static_cast<Knm::WirelessSetting *>(m_connection->setting(Knm::Setting::Wireless));
                    setting->setMode(Knm::WirelessSetting::EnumMode::apMode);
                    iface = w;
                    break;
                }
            }
        }
        if (!iface) {
            Knm::WirelessSetting * setting = static_cast<Knm::WirelessSetting *>(m_connection->setting(Knm::Setting::Wireless));
            setting->setMode(Knm::WirelessSetting::EnumMode::adhoc);
        }
    }

    m_wirelessWidget = new Wireless80211Widget(m_connection, ssid, shared, this);
    connect(m_wirelessWidget, SIGNAL(ssidSelected(Solid::Control::WirelessNetworkInterfaceNm09*,Solid::Control::AccessPointNm09*)),
            this, SLOT(setDefaultName(Solid::Control::WirelessNetworkInterfaceNm09*,Solid::Control::AccessPointNm09*)));

    m_securityWidget = new WirelessSecuritySettingWidget(m_connection, iface, ap, this);
    connect(m_wirelessWidget, SIGNAL(ssidSelected(Solid::Control::WirelessNetworkInterfaceNm09*,Solid::Control::AccessPointNm09*)),
            m_securityWidget, SLOT(setIfaceAndAccessPoint(Solid::Control::WirelessNetworkInterfaceNm09*,Solid::Control::AccessPointNm09*)));

    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);
    IpV6Widget * ipv6Widget = new IpV6Widget(m_connection, this);

    m_contents->setConnection(m_connection);
    if (shared) {
        m_contents->setDefaultName(i18n("Shared Wireless Connection"));
    } else {
        m_contents->setDefaultName(ssid.isEmpty() ? i18n("New Wireless Connection") : ssid);
    }

    connect (m_contents->connectionSettingsWidget(), SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));
    addToTabWidget(m_wirelessWidget);
    m_securityTabIndex = addToTabWidget(m_securityWidget);
    addToTabWidget(ipv4Widget);
    addToTabWidget(ipv6Widget);

    if ( setDefaults )
    {
        // for defaults the security is most interesting
        m_contents->connectionSettingsWidget()->setCurrentIndex( 1 );
    }
}

WirelessPreferences::~WirelessPreferences()
{
}

WirelessPreferences::WirelessPreferences(Knm::Connection *con, QWidget *parent)
: ConnectionPreferences(QVariantList(), parent), m_securityTabIndex(0)
{

    if (!con)
    {
        kDebug() << "Connection pointer is NULL, creating a new connection.";
        m_connection = new Knm::Connection(QUuid::createUuid(), Knm::Connection::Wireless);
    }
    else
        m_connection = con;

    QString connectionId = m_connection->uuid().toString();
    m_contents->setConnection(m_connection);

    m_wirelessWidget = new Wireless80211Widget(m_connection, NULL, false, this);
    connect(m_wirelessWidget, SIGNAL(ssidSelected(Solid::Control::WirelessNetworkInterfaceNm09*,Solid::Control::AccessPointNm09*)),
            this, SLOT(setDefaultName(Solid::Control::WirelessNetworkInterfaceNm09*,Solid::Control::AccessPointNm09*)));

    m_securityWidget = new WirelessSecuritySettingWidget(m_connection, NULL, NULL, this);

    IpV4Widget * ipv4Widget = new IpV4Widget(m_connection, this);
    IpV6Widget * ipv6Widget = new IpV6Widget(m_connection, this);

    connect (m_contents->connectionSettingsWidget(), SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    addToTabWidget(m_wirelessWidget);
    m_securityTabIndex = addToTabWidget(m_securityWidget);
    addToTabWidget(ipv4Widget);
    addToTabWidget(ipv6Widget);

    /*
    if ( setDefaults )
    {
        // for defaults the security is most interesting
        m_contents->connectionSettingsWidget()->setCurrentIndex( 1 );
    }
    */
}

void WirelessPreferences::setDefaultName(Solid::Control::WirelessNetworkInterfaceNm09 * wiface, Solid::Control::AccessPointNm09 * ap)
{
    Q_UNUSED(wiface);
    if(ap) {
        m_contents->setDefaultName(ap->ssid());
    }
}

bool WirelessPreferences::needsEdits() const
{
    return false;
}

void WirelessPreferences::tabChanged(int index)
{
    if (index == m_securityTabIndex && m_wirelessWidget->enteredSsidIsDirty()) {
        Solid::Control::WirelessNetworkInterfaceNm09 * ifaceForSsid = 0;
        Solid::Control::AccessPointNm09 * apForSsid = 0;
        // look up AP given by m_wirelessWidget, and set it on m_securityWidget
        QByteArray hwAddr = m_wirelessWidget->selectedInterfaceHardwareAddress();
        QString ssid = m_wirelessWidget->enteredSsid();
        if (!ssid.isEmpty()) {
            // find the S::C::WNI for this ssid
            // if hwAddr set, take that one
            // else, take the first one that can see this ssid
            foreach (Solid::Control::NetworkInterfaceNm09 * iface,
                    Solid::Control::NetworkManagerNm09::networkInterfaces()) {
                if (iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
                    Solid::Control::WirelessNetworkInterfaceNm09 * candidate = static_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(iface);
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

        // Ignore that in edit mode
        // m_securityWidget->setIfaceAndAccessPoint(ifaceForSsid, apForSsid);
        m_wirelessWidget->setEnteredSsidClean();
    }
}
// vim: sw=4 sts=4 et tw=100
