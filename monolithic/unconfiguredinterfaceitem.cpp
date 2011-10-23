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

#include "unconfiguredinterfaceitem.h"

#include <QGridLayout>
#include <QLabel>

#include <KGlobalSettings>
#include <KLocale>
#include <kdeversion.h>

#include <Solid/Device>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include <unconfiguredinterface.h>
#include <uiutils.h>

#include "activatableitem_p.h"

class UnconfiguredInterfaceItemPrivate : public ActivatableItemPrivate
{
public:
    QLabel * subtitleLabel;
};

UnconfiguredInterfaceItem::UnconfiguredInterfaceItem(Knm::UnconfiguredInterface * unconfigured, QWidget * parent)
: ActivatableItem(*new UnconfiguredInterfaceItemPrivate, unconfigured, parent)
{
    Q_D(UnconfiguredInterfaceItem);
    QString deviceText = UiUtils::interfaceNameLabel(unconfigured->deviceUni());

    setText(deviceText);

    d->subtitleLabel = new QLabel(this);
    d->subtitleLabel->setFont(KGlobalSettings::toolBarFont());
    d->subtitleLabel->setText(i18nc("Text for menu item for setting up devices which until now do not have any connections", "Create network connection..."));
    d->outerLayout->addWidget(d->subtitleLabel, 1, 1, 1, 1);

    d->activeIcon->setPixmap(pixmap());
    d->activeIcon->show();
    // disconnects us from to the activatable's activate slot, because this is handled indirectly by
    // KnetworkManagerTrayIcon now.
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(unconfigured->deviceUni());
    if (iface) {
        if (iface->type() == Solid::Control::NetworkInterface::Wifi) {
            disconnect(this, SIGNAL(clicked()), unconfigured, SLOT(activate()));
        }
    }
}

UnconfiguredInterfaceItem::~UnconfiguredInterfaceItem()
{

}


Knm::UnconfiguredInterface * UnconfiguredInterfaceItem::unconfiguredInterface() const
{
    return qobject_cast<Knm::UnconfiguredInterface*>(activatable());
}


QString UnconfiguredInterfaceItem::iconName() const
{
    // TODO a more specific icon could be identified using Solid eg external phones, pcmcia cards,
    // usb adaptors...
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(activatable()->deviceUni());
    QString icon;
    if (iface) {
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ethernet:
                icon = QLatin1String("network-wired");
                break;
            case Solid::Control::NetworkInterface::Wifi:
                icon = QLatin1String("network-wireless");
                break;
            case Solid::Control::NetworkInterface::Serial:
                icon = QLatin1String("modem");
                break;
            case Solid::Control::NetworkInterface::Gsm:
                icon = QLatin1String("phone");
                break;
            case Solid::Control::NetworkInterface::Cdma:
                icon = QLatin1String("phone");
                break;
            default:
                break;
        }
    }
    return icon;
}

// vim: sw=4 sts=4 et tw=100
