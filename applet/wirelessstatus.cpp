/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Sebastian Kügler <sebas@kde.org>

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

#include "wirelessstatus.h"

#include <KDebug>
#include <KIcon>
#include <KLocale>

//#include <Plasma/IconWidget>
//#include <Plasma/Meter>

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessaccesspoint.h>


#include <activatable.h>
#include <remotewirelessobject.h>
#include <remotewirelessinterfaceconnection.h>
#include <remotewirelessnetwork.h>
#include <wirelesssecurityidentifier.h>

#include "activatableitem.h"
//#include "wirelessinterfaceconnectionitem.h"
#include "wirelessnetworkitem.h"

class WirelessStatusPrivate
{
public:
    WirelessStatusPrivate()
        :
        ssid(QString()),
        securityIcon(QString()),
        securityTooltip(QString()),
        strength(-1),
        adhoc(false),
        iface(0),
        activeAccessPoint(0),
        activatable(0)
    {
    }

    void init(RemoteWirelessObject* wobj)
    {
        if (!wobj)
            return;

        if (wobj->operationMode() == Solid::Control::WirelessNetworkInterfaceNm09::Adhoc) {
            adhoc = true;
            //adhoc->setIcon(QIcon("nm-adhoc"));
        }
        ssid = wobj->ssid();
        bool isShared = false;
        if (activatable) {
            isShared = activatable->isShared();
        }
        Knm::WirelessSecurity::Type best = Knm::WirelessSecurity::best(wobj->interfaceCapabilities(), !isShared, (wobj->operationMode() == Solid::Control::WirelessNetworkInterfaceNm09::Adhoc), wobj->apCapabilities(), wobj->wpaFlags(), wobj->rsnFlags());
        //security->setToolTip(Knm::WirelessSecurity::shortToolTip(best));
        securityIcon = Knm::WirelessSecurity::iconName(best);
        securityTooltip = Knm::WirelessSecurity::shortToolTip(best);

    }

    void init(Solid::Control::WirelessNetworkInterfaceNm09 * wiface)
    {
        iface = wiface;
    }

    QList<Solid::Control::AccessPointNm09*> availableAccessPoints() const
    {
        QList<Solid::Control::AccessPointNm09*> retVal;
        if (!iface) {
            return retVal;
        }
        AccessPointNm09List aps = iface->accessPoints(); //NOTE: AccessPointList is a QStringList
        foreach (const QString &ap, aps) {
            Solid::Control::AccessPointNm09 *accesspoint = iface->findAccessPoint(ap);
            if(accesspoint) {
                retVal << accesspoint;
            }
        }
        return retVal;
    }

    QString ssid;
    QString securityIcon;
    QString securityTooltip;
    int strength;
    bool adhoc;

    Solid::Control::WirelessNetworkInterfaceNm09 * iface;
    Solid::Control::AccessPointNm09 * activeAccessPoint;
    RemoteActivatable* activatable;
};


WirelessStatus::WirelessStatus(RemoteWirelessNetwork * remote)
: QObject(), d_ptr(new WirelessStatusPrivate())
{
    Q_D(WirelessStatus);
    d->activatable = remote;
    RemoteWirelessObject * wobj  = static_cast<RemoteWirelessObject*>(remote);
    d->init(wobj);

    setStrength(remote->strength());
    //connect(m_remote, SIGNAL(changed()), SLOT(update()));
    //connect(m_remote, SIGNAL(changed()), SLOT(stateChanged()));
    connect(remote, SIGNAL(strengthChanged(int)), SLOT(setStrength(int)));

    //connect(item->wirelessInterfaceConnection(), SIGNAL(strengthChanged(int)), this, SLOT(setStrength(int)));
}

void WirelessStatus::init(RemoteWirelessObject* wobj)
{
    Q_D(WirelessStatus);
    d->init(wobj);
}

WirelessStatus::WirelessStatus(Solid::Control::WirelessNetworkInterfaceNm09 * iface)
: QObject(), d_ptr(new WirelessStatusPrivate())
{
    Q_D(WirelessStatus);

    connect(iface, SIGNAL(activeAccessPointChanged(QString)),
        SLOT(activeAccessPointChanged(QString)));

    d->init(iface);
    activeAccessPointChanged(d->iface->uni());
}

WirelessStatus::~WirelessStatus()
{
    delete d_ptr;
}

int WirelessStatus::strength()
{
    Q_D(WirelessStatus);
    return d->strength;
}

RemoteActivatable* WirelessStatus::activatable()
{
    Q_D(WirelessStatus);
    return d->activatable;
}

QString WirelessStatus::securityIcon()
{
    Q_D(WirelessStatus);
    return d->securityIcon;
}

QString WirelessStatus::ssid()
{
    Q_D(WirelessStatus);
    return d->ssid;
}

QString WirelessStatus::securityTooltip()
{
    Q_D(WirelessStatus);
    return d->securityTooltip;

}

bool WirelessStatus::isAdhoc()
{
    Q_D(WirelessStatus);
    return d->adhoc;
}

void WirelessStatus::setStrength(int strength)
{
    Q_D(WirelessStatus);
    if (strength != d->strength) {
        d->strength = strength;
        //kDebug() << d->ssid <<  "strength changed:" << d->strength;
        emit strengthChanged(d->strength);
    }
}

void WirelessStatus::activeAccessPointChanged(const QString &uni)
{
    Q_D(WirelessStatus);
    kDebug() << "*** AP changed:" << uni << "***";
    // this is not called when the device is deactivated..
    if (d->activeAccessPoint) {
        d->activeAccessPoint->disconnect(this);
        d->activeAccessPoint = 0;
    }
    if (uni != "/") {
        d->activeAccessPoint = d->iface->findAccessPoint(uni);
        if (d->activeAccessPoint) {
            kDebug() << "new:" << d->activeAccessPoint->ssid();
            setStrength(d->activeAccessPoint->signalStrength());
            d->ssid = d->activeAccessPoint->ssid();
            connect(d->activeAccessPoint, SIGNAL(signalStrengthChanged(int)), SLOT(setStrength(int)));
            connect(d->activeAccessPoint, SIGNAL(destroyed(QObject*)),
                    SLOT(accessPointDestroyed(QObject*)));
        }
    } else {
        setStrength(0);
    }
}

void WirelessStatus::accessPointDestroyed(QObject* ap)
{
    Q_D(WirelessStatus);
    kDebug() << "*** AP gone ***";
    if (ap == d->activeAccessPoint) {
        d->activeAccessPoint = 0;
        setStrength(0);
    }
}

// vim: sw=4 sts=4 et tw=100
