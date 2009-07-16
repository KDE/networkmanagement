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

#include "activatableitem.h"
#include "wirelessinterfaceconnection.h"
#include "wirelessitem.h"
#include "wirelessnetworkitem.h"

class WirelessStatusPrivate
{
public:
    Solid::Control::AccessPoint::Capabilities capabilities;
    Solid::Control::AccessPoint::WpaFlags wpaFlags;
    Solid::Control::AccessPoint::WpaFlags rsnFlags;
    QLabel * security;
    QProgressBar * strength;
};

class SmallProgressBar : public QProgressBar
{
public:
    SmallProgressBar(QWidget * parent = 0)
        : QProgressBar(parent)
    {
    }
    QSize sizeHint() const
    {
        return QSize(50,20/* 20 must be about minimum for Oxygen style at least as it won't go any smaller */);
    }
};


WirelessStatus::WirelessStatus(ActivatableItem * item)
: QObject(item), d_ptr(new WirelessStatusPrivate)
{
    Q_D(WirelessStatus);
    d->security = new QLabel(0);
    d->strength = new SmallProgressBar(0);
    d->strength->setTextVisible(false);
    d->strength->setRange(0, 100);
    d->strength->setGeometry(d->strength->x(),d->strength->y(), 50, 15);
    d->capabilities = 0;
    d->wpaFlags = 0;
    d->rsnFlags = 0;

    // discover the type of the activatable and connect its signals
    Knm::WirelessNetworkItem * wni = qobject_cast<Knm::WirelessNetworkItem*>(item->activatable());
    if (wni) {
        d->capabilities = wni->capabilities();
        d->wpaFlags = wni->wpaFlags();
        d->rsnFlags = wni->rsnFlags();
        connect(wni, SIGNAL(strengthChanged(int)), d->strength, SLOT(setValue(int)));
        d->strength->setValue(wni->strength());
    } else {
        Knm::WirelessInterfaceConnection * wic = qobject_cast<Knm::WirelessInterfaceConnection*>(item->activatable());
        if (wic) {
            d->capabilities = wic->capabilities();
            d->wpaFlags = wic->wpaFlags();
            d->rsnFlags = wic->rsnFlags();
            connect(wic, SIGNAL(strengthChanged(int)), d->strength, SLOT(setValue(int)));
            d->strength->setValue(wic->strength());
        }
    }

    setSecurity();

    item->addIcon(d->security);
    item->addIcon(d->strength);
}

WirelessStatus::~WirelessStatus()
{
    Q_D(WirelessStatus);
    delete d->security;
    delete d->strength;
    delete d_ptr;
}


void WirelessStatus::setSecurity()
{
    Q_D(WirelessStatus);
    // TODO: this was done by a clueless (coolo)
    if ( d->wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep40 ) ||
            d->wpaFlags.testFlag( Solid::Control::AccessPoint::PairWep104 )
            || (d->wpaFlags == 0 && d->capabilities.testFlag(Solid::Control::AccessPoint::Privacy))) {
        d->security->setPixmap(SmallIcon("security-medium"));
        d->security->setToolTip(i18nc("tooltip for WEP security", "WEP"));
        //m_security = QLatin1String("wep");
    } else if ( d->wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
            d->wpaFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) ) {
        d->security->setPixmap(SmallIcon("security-high"));
        d->security->setToolTip(i18nc("tooltip for WPA-PSK security", "WPA-PSK"));
        //m_security = QLatin1String("wpa-psk");
    } else if ( d->rsnFlags.testFlag( Solid::Control::AccessPoint::KeyMgmtPsk ) ||
            d->rsnFlags.testFlag( Solid::Control::AccessPoint::PairTkip ) ||
            d->rsnFlags.testFlag( Solid::Control::AccessPoint::PairCcmp ) ) {
        d->security->setPixmap(SmallIcon("security-high"));
        d->security->setToolTip(i18nc("tooltip for WPA-PSK security", "WPA-PSK"));
        //m_security = QLatin1String("wpa-psk");
    } else if ( d->wpaFlags.testFlag( Solid::Control::AccessPoint::KeyMgmt8021x ) ||
            d->wpaFlags.testFlag( Solid::Control::AccessPoint::GroupCcmp ) ) {
        d->security->setPixmap(SmallIcon("security-high"));
        d->security->setToolTip(i18nc("tooltip for WPA-EAP security", "WPA-EAP"));
        //m_security = QLatin1String("wpa-eap");
    } else {
        d->security->setPixmap(SmallIcon("security-low"));
        d->security->setToolTip(i18nc("tooltip for no security", "Insecure network"));
    }
}

// vim: sw=4 sts=4 et tw=100
