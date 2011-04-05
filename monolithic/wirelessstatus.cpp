/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include <QWidget>
#include <QLabel>
#include <QProgressBar>

#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KLocale>

#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>

#include <activatable.h>
#include <wirelessobject.h>
#include <wirelessinterfaceconnection.h>
#include <wirelessobject.h>
#include <wirelessnetwork.h>
#include <wirelesssecurityidentifier.h>

#include "activatableitem.h"
#include "wirelessinterfaceconnectionitem.h"
#include "wirelessnetworkitem.h"

class SmallProgressBar : public QProgressBar
{
public:
    SmallProgressBar(QWidget * parent = 0)
        : QProgressBar(parent)
    {
        setMinimumHeight(10);
        setMaximumHeight(10);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    QSize sizeHint() const
    {
        return QSize(50,10);
    }
};

class WirelessStatusPrivate
{
public:
    WirelessStatusPrivate(ActivatableItem * theItem)
        : item(theItem),
        adhoc(0),
        security(new QLabel(0)),
        strength(new SmallProgressBar(0))
    {
        strength->setTextVisible(false);
        strength->setRange(0, 100);
    }

    void init(Knm::WirelessObject * obj)
    {
        if (obj->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc) {
            adhoc = new QLabel(0);
            adhoc->setPixmap(SmallIcon("nm-adhoc")); //TODO real icon name
        }
        if (obj->strength() < 0) {
            strength->hide();
        }
        strength->setValue(obj->strength());
        strength->setToolTip(i18nc("@info:tooltip signal strength", "%1%", QString::number(obj->strength())));

        if (adhoc) {
            item->addIcon(adhoc);
        }
        item->addIcon(security);
        item->addIcon(strength);

        Knm::WirelessSecurity::Type best = Knm::WirelessSecurity::best(obj->interfaceCapabilities(), true, (obj->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc), obj->apCapabilities(), obj->wpaFlags(), obj->rsnFlags());
        security->setToolTip(Knm::WirelessSecurity::shortToolTip(best));
        security->setPixmap(SmallIcon(Knm::WirelessSecurity::iconName(best)));
    }

    ActivatableItem * item;
    QLabel * adhoc;
    QLabel * security;
    QProgressBar * strength;
};


WirelessStatus::WirelessStatus(WirelessInterfaceConnectionItem * item)
: QObject(item), d_ptr(new WirelessStatusPrivate(item))
{
    Q_D(WirelessStatus);

    Knm::WirelessObject * wobj  = static_cast<Knm::WirelessObject*>(item->wirelessInterfaceConnection());
    d->init(wobj);
    
    connect(item->wirelessInterfaceConnection(), SIGNAL(strengthChanged(int)), this, SLOT(setStrength(int)));
}

WirelessStatus::WirelessStatus(WirelessNetworkItem * item)
: QObject(item), d_ptr(new WirelessStatusPrivate(item))
{
    Q_D(WirelessStatus);

    Knm::WirelessObject * wobj  = static_cast<Knm::WirelessObject*>(item->wirelessNetwork());
    d->init(wobj);

    connect(item->wirelessNetwork(), SIGNAL(strengthChanged(int)), this, SLOT(setStrength(int)));
}

WirelessStatus::~WirelessStatus()
{
    Q_D(WirelessStatus);
    delete d->security;
    delete d->strength;
    delete d_ptr;
}

void WirelessStatus::setStrength(int strength)
{
    Q_D(WirelessStatus);
    if (strength < 0) {
        d->strength->hide();
    } else {
        d->strength->show();

        Knm::WirelessNetwork * wni = qobject_cast<Knm::WirelessNetwork*>(d->item->activatable());
        if (wni) {
            d->strength->setValue(wni->strength());
            d->strength->setToolTip(i18nc("@info:tooltip signal strength", "%1%", QString::number(wni->strength())));
        } else {
            Knm::WirelessInterfaceConnection * wic = qobject_cast<Knm::WirelessInterfaceConnection*>(d->item->activatable());
            if (wic) {
                d->strength->setValue(wic->strength());
                d->strength->setToolTip(i18nc("@info:tooltip signal strength", "%1%", QString::number(wic->strength())));
            }
        }
    }
}

// vim: sw=4 sts=4 et tw=100
