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

#include "interfaceconnectionitem.h"
#include "interfaceconnectionitem_p.h"

#include <QGridLayout>
#include <QLabel>
#include <QPixmap>

#include <KDebug>
#include <KLocale>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include "interfaceconnection.h"
#include "tooltipbuilder.h"

InterfaceConnectionItemPrivate::InterfaceConnectionItemPrivate()
: state(Knm::InterfaceConnection::Unknown), connectionDetailsLabel(0)
{

}

InterfaceConnectionItem::InterfaceConnectionItem(Knm::InterfaceConnection * interfaceConnection, QWidget * parent)
: ActivatableItem(*new InterfaceConnectionItemPrivate, interfaceConnection, parent)
{
    if (interfaceConnection->activationState() != Knm::InterfaceConnection::Unknown) {
        setActivationState(interfaceConnection->activationState());
    }

    setText(interfaceConnection->connectionName());

    connect(interfaceConnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
            this, SLOT(setActivationState(Knm::InterfaceConnection::ActivationState)));
}

InterfaceConnectionItem::InterfaceConnectionItem(InterfaceConnectionItemPrivate &dd, Knm::InterfaceConnection * interfaceConnection, QWidget * parent)
: ActivatableItem(dd, interfaceConnection, parent)
{
    if (interfaceConnection->activationState() != Knm::InterfaceConnection::Unknown) {
        setActivationState(interfaceConnection->activationState());
    }
    setText(interfaceConnection->connectionName());

    connect(interfaceConnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
            this, SLOT(setActivationState(Knm::InterfaceConnection::ActivationState)));
}

InterfaceConnectionItem::~InterfaceConnectionItem()
{

}

Knm::InterfaceConnection* InterfaceConnectionItem::interfaceConnection() const
{
    Q_D(const InterfaceConnectionItem);
    return qobject_cast<Knm::InterfaceConnection*>(d->activatable);
}

void InterfaceConnectionItem::setActivationState(Knm::InterfaceConnection::ActivationState state)
{
    kDebug();
    Q_D(InterfaceConnectionItem);
    switch (state) {
        case Knm::InterfaceConnection::Unknown:
            d->activeIcon->hide();
            if (d->connectionDetailsLabel) {
                d->outerLayout->removeWidget(d->connectionDetailsLabel);
                delete d->connectionDetailsLabel;
                d->connectionDetailsLabel = 0;
            }
            // clear tooltip
            setToolTip(QString());
            break;
        case Knm::InterfaceConnection::Activating:
            // set activating tooltip?
        case Knm::InterfaceConnection::Activated:
            d->activeIcon->setPixmap(pixmap());
            d->activeIcon->show();
            if (!d->connectionDetailsLabel) {
                d->connectionDetailsLabel = new QLabel(this);
                d->connectionDetailsLabel->setText(textForConnection(state));
                d->outerLayout->addWidget(d->connectionDetailsLabel, 1, 1, 1, 1);
            } else {
                d->connectionDetailsLabel->setText(textForConnection(state));
            }
            // set detailed tooltip using network interface state and ipv4
            setToolTip(ToolTipBuilder::toolTipForInterfaceConnection(interfaceConnection()));
            break;
    }
}

void InterfaceConnectionItem::changed()
{

}

QString InterfaceConnectionItem::textForConnection(Knm::InterfaceConnection::ActivationState state) const
{
    QString text;
    switch (state) {
        case Knm::InterfaceConnection::Unknown:
            text = i18nc("text for popup's unknown state", "<font size=\"-1\">Unknown</font>");
            break;
        case Knm::InterfaceConnection::Activating:
            text = i18nc("text for popup's activating state", "<font size=\"-1\">Activating</font>");
            break;
        case Knm::InterfaceConnection::Activated:
            text = i18nc("text for popup's activated state", "<font size=\"-1\">Active</font>");
            break;
    }
    return text;
}

QString InterfaceConnectionItem::iconName() const
{
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(activatable()->deviceUni());
    QString icon = QLatin1String("network-wired");
    if (iface) {
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee8023:
                // default ok
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
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
