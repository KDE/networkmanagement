/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008,2009 Sebastian Kügler <sebas@kde.org>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

#include "activatableitem.h"
#include <kdebug.h>
#include "remoteactivatable.h"

#include <QPainter>
#include <QGraphicsSceneHoverEvent>
#include <QSize>
#include <QTimer>

#include <KIcon>
#include <KNotification>
#include <KGlobalSettings>

#include <Plasma/Animation>
#include <Plasma/Animator>

#include <solid/control/networkmanager.h>

#include "../libs/service/events.h"

K_GLOBAL_STATIC_WITH_ARGS(KComponentData, s_networkManagementComponentData, ("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration))
static const int m_iconSize = 48;
int rowHeight = qMax(30, QFontMetrics(KGlobalSettings::generalFont()).height()+10);
int maxConnectionNameWidth = QFontMetrics(KGlobalSettings::generalFont()).width("12345678901234567890123");

ActivatableItem::ActivatableItem(RemoteActivatable *remote, QGraphicsItem * parent) : Plasma::IconWidget(parent),
    m_activatable(remote),
    m_hasDefaultRoute(false),
    m_deleting(false),
    spacing(4),
    m_connectButton(0)
{
    setDrawBackground(true);
    setTextBackgroundColor(QColor(Qt::transparent));

    RemoteInterfaceConnection *remoteconnection = interfaceConnection();
    if (remoteconnection) {
        connect(remoteconnection, SIGNAL(hasDefaultRouteChanged(bool)),
                SLOT(handleHasDefaultRouteChanged(bool)));
        connect(remoteconnection, SIGNAL(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)),
                SLOT(activationStateChanged(Knm::InterfaceConnection::ActivationState,Knm::InterfaceConnection::ActivationState)));
        connect(remoteconnection, SIGNAL(changed()), SLOT(connectionChanged()));
    }

    // Fade in when this widget appears
    Plasma::Animation* fadeAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    fadeAnimation->setTargetWidget(this);
    fadeAnimation->setProperty("startOpacity", 0.0);
    fadeAnimation->setProperty("targetOpacity", 1.0);
    fadeAnimation->start();
    setMinimumHeight(rowHeight);
    setMaximumHeight(rowHeight);
}

ActivatableItem::~ActivatableItem()
{
}

void ActivatableItem::disappear()
{
    if (m_deleting) {
        return;
    }
    m_activatable = 0;
    m_deleting = true;
    // Fade out when this widget appears
    Plasma::Animation* disappearAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    disappearAnimation->setTargetWidget(this);
    disappearAnimation->setProperty("startOpacity", 1.0);
    disappearAnimation->setProperty("targetOpacity", 0.0);
    //disappearAnimation->setProperty("duration", 2000);
    disappearAnimation->start();
    connect(disappearAnimation, SIGNAL(finished()), this, SIGNAL(disappearAnimationFinished()));
}

void ActivatableItem::emitClicked()
{
    if (m_activatable) {
        RemoteInterfaceConnection * remote = interfaceConnection();
        if (remote && (remote->activationState() == Knm::InterfaceConnection::Activating ||
                       remote->activationState() == Knm::InterfaceConnection::Activated)) {
            emit showInterfaceDetails(remote->deviceUni());
        } else {
            QTimer::singleShot(0, m_activatable, SLOT(activate()));
        }
        emit clicked(this);
    }

    QTimer::singleShot(0, this, SLOT(notifyNetworkingState()));
}

void ActivatableItem::notifyNetworkingState()
{
    if (!Solid::Control::NetworkManagerNm09::isNetworkingEnabled()) {
        KNotification::event(Event::NetworkingDisabled, i18nc("@info:status Notification when the networking subsystem (NetworkManager, etc) is disabled", "Networking system disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, *s_networkManagementComponentData)->sendEvent();
    } else if (!Solid::Control::NetworkManagerNm09::isWirelessEnabled() &&
               m_activatable &&
               m_activatable->activatableType() == Knm::Activatable::WirelessInterfaceConnection) {
        KNotification::event(Event::RfOff, i18nc("@info:status Notification for radio kill switch turned off", "Wireless hardware disabled"), KIcon("network-wireless").pixmap(QSize(m_iconSize,m_iconSize)), 0, KNotification::CloseOnTimeout, *s_networkManagementComponentData)->sendEvent();
    }
}

RemoteInterfaceConnection* ActivatableItem::interfaceConnection() const
{
    return qobject_cast<RemoteInterfaceConnection*>(m_activatable);
}

void ActivatableItem::handleHasDefaultRouteChanged(bool has)
{
    // do something nice to show that this connection has the default route
    m_hasDefaultRoute = has;
    update();
}

void ActivatableItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Plasma::IconWidget::paint(painter, option, widget);
    if (m_hasDefaultRoute) {
        // TODO: this draws the pixmap behind the connection icon. This is the same
        // problem described in a comment in networkmanager.cpp:NetworkManagerApplet::paintInterface.
        painter->drawPixmap(QRect(4,4,12,12), KIcon("network-defaultroute").pixmap(QSize(16,16)));
    }
}

void ActivatableItem::activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState)
{
    Q_UNUSED(oldState);
    m_state = newState;

    if (!m_connectButton) {
        return;
    }

    // Update the view of the connection, manipulate font based on activation state.
    kDebug() << newState;
    QFont f = m_connectButton->font();
    switch (newState) {
        //Knm::InterfaceConnectihon::ActivationState
        case Knm::InterfaceConnection::Activated:
            kDebug() << "activated";
            f.setBold(true);
            f.setItalic(false);
            break;
        case Knm::InterfaceConnection::Unknown:
            kDebug() << "unknown";
            f.setBold(false);
            f.setItalic(false);
            break;
        case Knm::InterfaceConnection::Activating:
            kDebug() << "activatING....";
            f.setBold(false);
            f.setItalic(true);
    }
    m_connectButton->setFont(f);
}

void ActivatableItem::connectionChanged()
{
    if (!m_connectButton || !interfaceConnection()) {
        return;
    }

    m_connectButton->setText(interfaceConnection()->connectionName(true));
}

void ActivatableItem::hoverEnter()
{
    hoverEnterEvent(new QGraphicsSceneHoverEvent());
}

void ActivatableItem::hoverLeave()
{
    hoverLeaveEvent(new QGraphicsSceneHoverEvent());
}
// vim: sw=4 sts=4 et tw=100
