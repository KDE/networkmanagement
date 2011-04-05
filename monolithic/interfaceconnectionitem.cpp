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
along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include "interfaceconnectionitem.h"
#include "interfaceconnectionitem_p.h"

#include <QGridLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>

#include <KDebug>
#include <KLocale>
#include <KIcon>
#include <KIconLoader>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include "interfaceconnection.h"
#include "tooltipbuilder.h"

InterfaceConnectionItemPrivate::InterfaceConnectionItemPrivate()
: state(Knm::InterfaceConnection::Unknown), connectionLayout(0), connectionDetailsLabel(0), defaultRouteLabel(0), disconnectButton(0)
{
}

InterfaceConnectionItem::InterfaceConnectionItem(Knm::InterfaceConnection * interfaceConnection, QWidget * parent)
: ActivatableItem(*new InterfaceConnectionItemPrivate, interfaceConnection, parent)
{
    Q_D(InterfaceConnectionItem);

    d->connectionLayout = new QHBoxLayout(this);
    d->connectionLayout->setObjectName("connectionLayout");
    d->outerLayout->addLayout(d->connectionLayout, 1, 1, 1, 1);

    d->defaultRouteLabel = new QLabel(this);
    d->defaultRouteLabel->setPixmap(SmallIcon("emblem-favorite"));
    d->defaultRouteLabel->setToolTip(i18nc("@info:tooltip Tooltip for indicator that connection supplies the network default route", "Default"));
    d->defaultRouteLabel->setVisible(interfaceConnection->hasDefaultRoute());
    addIcon(d->defaultRouteLabel);

    connect(interfaceConnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
            this, SLOT(setActivationState(Knm::InterfaceConnection::ActivationState)));
    connect(interfaceConnection, SIGNAL(hasDefaultRouteChanged(bool)),
            this, SLOT(setHasDefaultRoute(bool)));
    connect(interfaceConnection, SIGNAL(changed()),
            this, SLOT(changed()));

    setActivationState(interfaceConnection->activationState());

    setText(interfaceConnection->connectionName());
}

InterfaceConnectionItem::InterfaceConnectionItem(InterfaceConnectionItemPrivate &dd, Knm::InterfaceConnection * interfaceConnection, QWidget * parent)
: ActivatableItem(dd, interfaceConnection, parent)
{
    Q_D(InterfaceConnectionItem);

    d->connectionLayout = new QHBoxLayout(this);
    d->connectionLayout->setObjectName("connectionLayout");
    d->outerLayout->addLayout(d->connectionLayout, 1, 1, 1, 1);

    d->defaultRouteLabel = new QLabel(this);
    d->defaultRouteLabel->setPixmap(SmallIcon("emblem-favorite"));
    d->defaultRouteLabel->setToolTip(i18nc("@info:tooltip Tooltip for indicator that connection supplies the network default route", "Default"));
    d->defaultRouteLabel->setVisible(interfaceConnection->hasDefaultRoute());
    addIcon(d->defaultRouteLabel);

    connect(interfaceConnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState)),
            this, SLOT(setActivationState(Knm::InterfaceConnection::ActivationState)));
    connect(interfaceConnection, SIGNAL(hasDefaultRouteChanged(bool)),
            this, SLOT(setHasDefaultRoute(bool)));
    connect(interfaceConnection, SIGNAL(changed()),
            this, SLOT(changed()));

    setActivationState(interfaceConnection->activationState());

    setText(interfaceConnection->connectionName());
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
    Q_D(InterfaceConnectionItem);
    if (state != d->state) {
        d->state = state;
        switch (state) {
            case Knm::InterfaceConnection::Unknown:
                if (d->connectionDetailsLabel) {
                    d->connectionLayout->removeWidget(d->connectionDetailsLabel);
                    delete d->connectionDetailsLabel;
                    d->connectionDetailsLabel = 0;
                }
                if (d->disconnectButton) {
                    d->connectionLayout->removeWidget(d->disconnectButton);
                    delete d->disconnectButton;
                    d->disconnectButton = 0;
                }
                // clear tooltip
                setToolTip(QString());
                break;
            case Knm::InterfaceConnection::Activating:
            case Knm::InterfaceConnection::Activated:
                Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(activatable()->deviceUni());
                if (!d->connectionDetailsLabel) {
                    d->connectionDetailsLabel = new QLabel(this);
                    d->connectionLayout->addWidget(d->connectionDetailsLabel);
                }
                if (iface) {
                    if (!d->disconnectButton) {
                        d->disconnectButton = new QPushButton(this);
                        d->disconnectButton->setIcon(KIcon("process-stop"));
                        int buttonSize = qMax(d->connectionDetailsLabel->sizeHint().height(), int(KIconLoader::SizeSmall));
                        d->disconnectButton->setFixedSize(buttonSize, buttonSize);

                        d->disconnectButton->setToolTip(i18nc("@info:tooltip network connection disconnect button tooltip", "Disconnect"));
                        d->connectionLayout->addWidget(d->disconnectButton);
                        connect(d->disconnectButton, SIGNAL(clicked()), this, SLOT(disconnectClicked()));
                    }
                }
                d->connectionDetailsLabel->setText(textForConnection(d->state));
                // set detailed tooltip using network interface state and ipv4
                updateToolTip();
                break;
        }
    }
}

void InterfaceConnectionItem::setHasDefaultRoute(bool hasDefaultRoute)
{
    Q_D(InterfaceConnectionItem);
    d->defaultRouteLabel->setVisible(hasDefaultRoute);
}

void InterfaceConnectionItem::changed()
{
    Q_D(InterfaceConnectionItem);
    Knm::InterfaceConnection * ic = qobject_cast<Knm::InterfaceConnection*>(d->activatable);
    if (ic) {
        setText(ic->connectionName());
        d->activeIcon->setPixmap(pixmap());
    }
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
    return interfaceConnection()->iconName();
}

void InterfaceConnectionItem::disconnectClicked()
{
    kDebug();
    interfaceConnection()->deactivate();
}

void InterfaceConnectionItem::updateToolTip()
{
    setToolTip(ToolTipBuilder::toolTipForInterfaceConnection(interfaceConnection()));
}
// vim: sw=4 sts=4 et tw=100
