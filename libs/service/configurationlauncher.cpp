/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "configurationlauncher.h"

#include <QDBusInterface>
#include <QList>
#include <QPair>
#include <QTimer>

#include <KDebug>
#include <KToolInvocation>
#include <KStandardDirs>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkmodeminterface.h>
#include <wirelessnetworkinterfaceenvironment.h>

#include "unconfiguredinterface.h"
#include "wirelessnetwork.h"
#include "wirelessinterfaceconnection.h"

typedef QPair<QString,QString> PendingNetwork;
class ConfigurationLauncherPrivate
{
public:
    // list of <ssid,deviceUni> tuples showing which wireless networks are pending connections
    QList<PendingNetwork> pendingNetworks;
    // list of deviceUnis showing which devices are pending connections (any connection will be
    // used that appears for this device)
    QStringList pendingDevices;
};

ConfigurationLauncher::ConfigurationLauncher(QObject * parent)
: QObject(parent), d_ptr(new ConfigurationLauncherPrivate)
{
}

ConfigurationLauncher::~ConfigurationLauncher()
{
}

void ConfigurationLauncher::handleAdd(Knm::Activatable *added)
{
    Q_D(ConfigurationLauncher);
    Knm::WirelessNetwork * wni = 0;
    Knm::InterfaceConnection * ic = 0;
    Knm::WirelessInterfaceConnection * wic = 0;
    Knm::UnconfiguredInterface * unco = 0;
    switch (added->activatableType()) {
        case Knm::Activatable::WirelessNetwork:
            wni = qobject_cast<Knm::WirelessNetwork*>(added);
            connect(wni, SIGNAL(activated()), this, SLOT(wirelessNetworkActivated()));
            break;
        case Knm::Activatable::InterfaceConnection:
        case Knm::Activatable::GsmInterfaceConnection:
            ic = qobject_cast<Knm::InterfaceConnection*>(added);
            if (d->pendingDevices.contains(ic->deviceUni())) {
                kDebug() << "activating new connection on" << ic->deviceUni();
                QTimer::singleShot(0, ic, SIGNAL(activated()));
                d->pendingDevices.removeOne(ic->deviceUni());
            }
            break;
        case Knm::Activatable::WirelessInterfaceConnection:
            wic = qobject_cast<Knm::WirelessInterfaceConnection*>(added);
            foreach (const PendingNetwork &pending, d->pendingNetworks) {
                if (pending.first == wic->ssid() && pending.second == wic->deviceUni()) {
                    kDebug() << "activating WIC for" << wic->ssid() << "on" << wic->deviceUni();
                    // HACK - for activate to do anything, it has to be connected to something.  
                    // However, since this method itself is a slot, the connection to
                    // activated() is made in another slot which may be called after this slot, so
                    // emitting it now is just wrong. 
                    QTimer::singleShot(0, wic, SIGNAL(activated()));
                    d->pendingNetworks.removeOne(pending);
                }
            }
            break;
        case Knm::Activatable::UnconfiguredInterface:
            unco = qobject_cast<Knm::UnconfiguredInterface*>(added);
            connect(unco, SIGNAL(activated()), this, SLOT(unconfiguredInterfaceActivated()));
            break;
        default:
            break;
    }
}

void ConfigurationLauncher::wirelessNetworkActivated()
{
    kDebug();
    Knm::WirelessNetwork * wni = qobject_cast<Knm::WirelessNetwork*>(sender());
    if (wni) {
        configureWirelessNetworkInternal(wni->ssid(), wni->deviceUni());
    }
}

void ConfigurationLauncher::configureHiddenWirelessNetwork(const QString & ssid, const QString & deviceUni)
{
    configureWirelessNetworkInternal(ssid, deviceUni);
}

void ConfigurationLauncher::configureWirelessNetworkInternal(const QString & ssid, const QString & deviceUni)
{
    Q_D(ConfigurationLauncher);
    d->pendingNetworks.append(QPair<QString,QString>(ssid, deviceUni));
    kDebug() << "watching for connection for" << ssid << "on" << deviceUni;

    QString apUni = QLatin1String("/");

    Solid::Control::WirelessNetworkInterfaceNm09 * iface = qobject_cast<Solid::Control::WirelessNetworkInterfaceNm09*>(Solid::Control::NetworkManagerNm09::findNetworkInterface(deviceUni));
    if (iface) {
        Solid::Control::WirelessNetworkInterfaceEnvironment envt(iface);
        Solid::Control::WirelessNetwork * network = envt.findNetwork(ssid);
        if (network) {
            apUni = network->referenceAccessPoint();
        }
    }

    // Call the config UI
    //kDebug() << wni->net()->referenceAccessPoint()->hardwareAddress();
    QDBusInterface kcm(QLatin1String("org.kde.NetworkManager.KCModule"), QLatin1String("/default"), QLatin1String("org.kde.kcmshell.ManageConnectionWidget"));
    if (kcm.isValid()) {
        kDebug() << "opening connection management dialog from running KCM";
        QVariantList args;

        args << deviceUni << apUni;
        kcm.call(QDBus::NoBlock, "createConnection", "802-11-wireless", QVariant::fromValue(args));
    } else {
        kDebug() << "opening connection management dialog using networkmanagement_configshell";
        QStringList args;
        QString moduleArgs =
            QString::fromLatin1("%1 %2")
            .arg(deviceUni)
            .arg(apUni);

        args << QLatin1String("create") << QLatin1String("--type") << QLatin1String("802-11-wireless") << QLatin1String("--specific-args") << moduleArgs << QLatin1String("wifi_pass");
        int ret = KToolInvocation::kdeinitExec(KGlobal::dirs()->findResource("exe", "networkmanagement_configshell"), args);
        kDebug() << ret << args;
    }
}

void ConfigurationLauncher::handleUpdate(Knm::Activatable *)
{
    // dummy impl
}

void ConfigurationLauncher::handleRemove(Knm::Activatable *)
{
    // dummy impl
}

void ConfigurationLauncher::unconfiguredInterfaceActivated()
{
    Q_D(ConfigurationLauncher);
    Knm::UnconfiguredInterface * unco = qobject_cast<Knm::UnconfiguredInterface*>(sender());
    if (unco) {
        kDebug() << "Creating a connection for" << unco->deviceUni();
        if (!d->pendingDevices.contains(unco->deviceUni())) {
            d->pendingDevices.append(unco->deviceUni());
        }

        //HACK - write proper AsString and FromString functions in the library somewhere
        Solid::Control::NetworkInterfaceNm09 * iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(unco->deviceUni());
        QString typeString;
        QString editorArgs;
        if (iface) {
            switch (iface->type()) {
                case Solid::Control::NetworkInterfaceNm09::Ethernet:
                    typeString = QLatin1String("802-3-ethernet");
                    break;
                case Solid::Control::NetworkInterfaceNm09::Wifi:
                    typeString = QLatin1String("802-11-wireless");
                    break;
                case Solid::Control::NetworkInterfaceNm09::Modem: {
                    Solid::Control::ModemNetworkInterfaceNm09 * nmModemIface = qobject_cast<Solid::Control::ModemNetworkInterfaceNm09 *>(iface);
                    if (nmModemIface) {
                        Solid::Control::ModemNetworkInterfaceNm09::ModemCapabilities subType = nmModemIface->subType();
                        switch(subType) {
                            case Solid::Control::ModemNetworkInterfaceNm09::Pots:
                                 typeString = QLatin1String("pppoe");
                                 break;
                            case Solid::Control::ModemNetworkInterfaceNm09::GsmUmts:
                                 typeString = QLatin1String("gsm");
                                 editorArgs = QLatin1String("gsm");
                                 break;
                            case Solid::Control::ModemNetworkInterfaceNm09::CdmaEvdo:
                                 typeString = QLatin1String("cdma");
                                 editorArgs = QLatin1String("cdma");
                                 break;
                            case Solid::Control::ModemNetworkInterfaceNm09::Lte:
                                 /* TODO */
                                 kWarning() << "Unhandled modem sub type: LTE"; break;
                                 break;
                            case Solid::Control::ModemNetworkInterfaceNm09::None:
                                 kWarning() << "Unhandled modem sub type: Solid::Control::ModemNetworkInterfaceNm09::None";
                                 break;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        QDBusInterface kcm(QLatin1String("org.kde.NetworkManager.KCModule"), QLatin1String("/default"), QLatin1String("org.kde.kcmshell.ConnectionEditor"));
        if (kcm.isValid()) {
            kDebug() << "opening connection management dialog from running KCM";
            QVariantList args;
            if (!editorArgs.isEmpty()) {
                args << editorArgs;
            }
            kcm.call(QDBus::NoBlock, "createConnection", typeString, QVariant::fromValue(args));
        } else {
            kDebug() << "opening connection management dialog using networkmanagement_configshell";
            QStringList args;

            args << QLatin1String("create") << QLatin1String("--type") << typeString;
            if (!editorArgs.isEmpty()) {
                args << QLatin1String("--specific-args") << editorArgs;
            }
            int ret = KToolInvocation::kdeinitExec(KGlobal::dirs()->findResource("exe", "networkmanagement_configshell"), args);
            kDebug() << ret << args;
        }
    }
}
// vim: sw=4 sts=4 et tw=100
