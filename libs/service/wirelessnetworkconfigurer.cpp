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

#include "wirelessnetworkconfigurer.h"

#include <QDBusInterface>
#include <QList>
#include <QPair>
#include <QTimer>

#include <KDebug>
#include <KToolInvocation>

#include "wirelessnetworkitem.h"
#include "wirelessinterfaceconnection.h"

typedef QPair<QString,QString> PendingNetwork;
class WirelessNetworkConfigurerPrivate
{
public:
    // list of <ssid,deviceUni> tuples
    QList<PendingNetwork> pendingNetworks;
};

WirelessNetworkConfigurer::WirelessNetworkConfigurer(QObject * parent)
: ActivatableObserver(parent), d_ptr(new WirelessNetworkConfigurerPrivate)
{
}

WirelessNetworkConfigurer::~WirelessNetworkConfigurer()
{
}

void WirelessNetworkConfigurer::handleAdd(Knm::Activatable *added)
{
    Q_D(WirelessNetworkConfigurer);
    Knm::WirelessNetworkItem * wni = qobject_cast<Knm::WirelessNetworkItem*>(added);
    // listen to the WNI
    if (wni) {
        connect(wni, SIGNAL(activated()), this, SLOT(wirelessNetworkItemActivated()));
    } else {
        Knm::WirelessInterfaceConnection * wic = qobject_cast<Knm::WirelessInterfaceConnection*>(added);
        // check if it's WIC we're waiting for
        if (wic) {
            foreach (PendingNetwork pending, d->pendingNetworks) {
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
        }
    }
}

void WirelessNetworkConfigurer::wirelessNetworkItemActivated()
{
    Knm::WirelessNetworkItem * wni = qobject_cast<Knm::WirelessNetworkItem*>(sender());
    if (wni) {
        configureWirelessNetworkInternal(wni->ssid(), wni->deviceUni(), wni->wpaFlags(), wni->rsnFlags());
    }
}

void WirelessNetworkConfigurer::configureHiddenWirelessNetwork(const QString & ssid, const QString & deviceUni)
{
    configureWirelessNetworkInternal(ssid, deviceUni, 0, 0);
}

void WirelessNetworkConfigurer::configureWirelessNetworkInternal(const QString & ssid, const QString & deviceUni, Solid::Control::AccessPoint::WpaFlags wpaFlags, Solid::Control::AccessPoint::WpaFlags rsnFlags)
{
    Q_D(WirelessNetworkConfigurer);
    d->pendingNetworks.append(QPair<QString,QString>(ssid, deviceUni));
    kDebug() << "watching for connection for" << ssid << "on" << deviceUni;

    // Call the config UI
    int caps = 1;
    //kDebug() << wni->net()->referenceAccessPoint()->hardwareAddress();
    QDBusInterface kcm(QLatin1String("org.kde.NetworkManager.KCModule"), QLatin1String("/default"), QLatin1String("org.kde.kcmshell.ConnectionEditor"));
    if (kcm.isValid()) {
        kDebug() << "opening connection management dialog from running KCM";
        QVariantList args;

        args << ssid << caps << (uint)wpaFlags << (uint)rsnFlags;
        kcm.call(QDBus::NoBlock, "createConnection", "802-11-wireless", QVariant::fromValue(args));
    } else {
        kDebug() << "opening connection management dialog using networkmanagement_configshell";
        QStringList args;
        QString escapedSsid = ssid;
        QString moduleArgs =
            QString::fromLatin1("'%1' %2 %3 %4")
            .arg(escapedSsid.replace('\'', "\\'"))
            .arg(caps)
            .arg(wpaFlags)
            .arg(rsnFlags);

        args << QLatin1String("create") << QLatin1String("--type") << QLatin1String("802-11-wireless") << QLatin1String("--specific-args") << moduleArgs << QLatin1String("wifi_pass");
        int ret = KToolInvocation::kdeinitExec("networkmanagement_configshell", args);
        kDebug() << ret << args;
    }
}

void WirelessNetworkConfigurer::handleUpdate(Knm::Activatable *)
{
    // dummy impl
}

void WirelessNetworkConfigurer::handleRemove(Knm::Activatable *)
{
    // dummy impl
}
// vim: sw=4 sts=4 et tw=100
