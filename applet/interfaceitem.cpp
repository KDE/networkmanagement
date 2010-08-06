/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian Kügler <sebas@kde.org>

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

#include "interfaceitem.h"
#include "uiutils.h"
#include "remoteinterfaceconnection.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"

#include <QGraphicsGridLayout>
#include <QLabel>
#include <QPainter>

#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KIcon>
#include <kdeversion.h>

#include <Plasma/Animation>
#include <Plasma/Animator>

#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/Meter>

#include <Solid/Device>
#include <Solid/NetworkInterface>
#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkmanager.h>

#include "knmserviceprefs.h"


InterfaceItem::InterfaceItem(Solid::Control::NetworkInterface * iface, RemoteActivatableList* activatables,  NameDisplayMode mode, QGraphicsWidget * parent) : Plasma::IconWidget(parent),
    m_currentConnection(0),
    m_iface(iface),
    m_activatables(activatables),
    m_icon(0),
    m_connectionNameLabel(0),
    m_nameMode(mode),
    m_enabled(false),
    m_hasDefaultRoute(false)
{
    setDrawBackground(true);
    setTextBackgroundColor(QColor(Qt::transparent));
    QString tt = i18nc("tooltip on the LHS widgets", "Click here for interface details");

    m_pixmapSize = QSize(48, 48);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setVerticalSpacing(0);
    m_layout->setColumnSpacing(0, 8);
    m_layout->setColumnSpacing(1, 4);
    m_layout->setColumnSpacing(2, 6);
    m_layout->setRowSpacing(0, 6);
    m_layout->setRowSpacing(1, 6);
    m_layout->setPreferredWidth(240);
    m_layout->setColumnFixedWidth(0, m_pixmapSize.width());
    m_layout->setColumnMinimumWidth(1, 160);
    m_layout->setColumnFixedWidth(2, 16); // FIXME: spacing?

    m_icon = new Plasma::Label(this);
    m_icon->setToolTip(tt);
    m_icon->setMinimumHeight(m_pixmapSize.height());
    m_icon->setMaximumHeight(m_pixmapSize.height());

    setMinimumHeight(m_pixmapSize.height()+6);
    m_layout->addItem(m_icon, 0, 0, 2, 1);

    QString icon;
    if (m_iface) {
        m_interfaceName = UiUtils::interfaceNameLabel(m_iface->uni());
    }
    m_icon->nativeWidget()->setPixmap(interfacePixmap());

    //     interface layout
    m_ifaceNameLabel = new Plasma::Label(this);
    m_ifaceNameLabel->setToolTip(tt);
    m_ifaceNameLabel->setText(m_interfaceName);
    m_ifaceNameLabel->nativeWidget()->setWordWrap(false);
    m_ifaceNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 1);

    m_disconnectButton = new Plasma::PushButton(this);
    m_disconnectButton->setMaximumHeight(16);
    m_disconnectButton->setMaximumWidth(16);
    m_disconnectButton->setIcon(KIcon("dialog-close"));
    m_disconnectButton->setToolTip(i18n("Disconnect"));
    m_disconnectButton->hide();
    // forward disconnect signal
    connect(m_disconnectButton, SIGNAL(clicked()), this, SLOT(emitDisconnectInterfaceRequest()));

    m_layout->addItem(m_disconnectButton, 0, 2, 1, 1, Qt::AlignRight);

    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setToolTip(tt);
    m_connectionNameLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionNameLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_connectionNameLabel, 1, 1, 1, 1);

    //       security
    m_connectionInfoIcon = new Plasma::Label(this);
    m_connectionInfoIcon->setMinimumHeight(16);
    m_connectionInfoIcon->setMinimumWidth(16);
    m_connectionInfoIcon->setMaximumHeight(16);
    m_connectionInfoIcon->nativeWidget()->setPixmap(KIcon("security-low").pixmap(16, 16));
    m_connectionInfoIcon->hide(); // hide by default, we'll enable it later

    m_layout->addItem(m_connectionInfoIcon, 1, 2, 1, 1, Qt::AlignRight); // check...

    if (m_iface) {
        connect(m_iface, SIGNAL(connectionStateChanged(int,int,int)),
                this, SLOT(handleConnectionStateChange(int,int,int)));
        connect(m_iface, SIGNAL(linkUpChanged(bool)), this, SLOT(setActive(bool)));
    }
    setNameDisplayMode(mode);

    if (m_iface) {
        if (m_iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
            Solid::Control::WiredNetworkInterface* wirediface =
                            static_cast<Solid::Control::WiredNetworkInterface*>(m_iface);
            connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(setActive(bool)));
	}
        connectionStateChanged(m_iface->connectionState());
    }

    setNameDisplayMode(mode);
    setLayout(m_layout);
    m_layout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

    // Fade in when this widget appears
    Plasma::Animation* fadeAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    fadeAnimation->setTargetWidget(this);
    fadeAnimation->setProperty("startOpacity", 0.0);
    fadeAnimation->setProperty("targetOpacity", 1.0);
    fadeAnimation->setProperty("Duration", 2000);

    fadeAnimation->start();
}

InterfaceItem::~InterfaceItem()
{
    Plasma::Animation* fadeAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    fadeAnimation->setTargetWidget(this);
    fadeAnimation->setProperty("startOpacity", 1.0);
    fadeAnimation->setProperty("targetOpacity", 0.0);
    fadeAnimation->setProperty("Duration", 2000);

    //fadeAnimation->setTargetOpacity(1.0);
    fadeAnimation->start();
}

QString InterfaceItem::label()
{
    return m_ifaceNameLabel->text();
}

Solid::Control::NetworkInterface* InterfaceItem::interface()
{
    return m_iface;
}

void InterfaceItem::setActive(bool active)
{
    kDebug() << "+ + + + + + Active?" << active;
    if (m_iface) {
        connectionStateChanged(m_iface->connectionState());
    }
}

void InterfaceItem::setEnabled(bool enable)
{
    m_enabled = enable;
    m_icon->setEnabled(enable);
    //m_connectionInfoLabel->setEnabled(enable);
    m_connectionNameLabel->setEnabled(enable);
    m_ifaceNameLabel->setEnabled(enable);
    m_disconnectButton->setEnabled(enable);
    m_connectionInfoIcon->setEnabled(enable);
    if (!enable) {
        m_connectionInfoIcon->hide();
    }
}

void InterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if (m_nameMode == InterfaceName) {
        m_ifaceNameLabel->setText(QString("<b>%1</b>").arg(m_interfaceName));
    } else if (m_nameMode == HardwareName) {
        if (m_iface) {
            m_ifaceNameLabel->setText(QString("<b>%1</b>").arg(m_iface->interfaceName()));
        } else {
            m_ifaceNameLabel->setText(i18nc("generic label for an interface", "<b>Network Interface</b>"));
        }

    } else {
        m_ifaceNameLabel->setText(i18nc("network interface name unknown", "<b>Unknown Network Interface</b>"));
    }
}

InterfaceItem::NameDisplayMode InterfaceItem::nameDisplayMode() const
{
    return m_nameMode;
}

QString InterfaceItem::connectionName()
{
    // Default active connection's name is empty, room for improvement?
    RemoteInterfaceConnection *conn = UiUtils::connectionForInterface(m_activatables, m_iface);
    if (conn) {
        return conn->connectionName();
    }
    return QString();
}

void InterfaceItem::setConnectionInfo()
{
    if (m_iface) {
        currentConnectionChanged();
        connectionStateChanged(m_iface->connectionState());
    }
}

QString InterfaceItem::currentIpAddress()
{
    if (m_iface && m_iface->connectionState() != Solid::Control::NetworkInterface::Activated) {
        return i18nc("label of the network interface", "No IP address.");
    }
    Solid::Control::IPv4Config ip4Config = m_iface->ipV4Config();
    QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
    if (addresses.isEmpty()) {
        return i18nc("label of the network interface", "IP display error.");
    }
    QHostAddress addr(addresses.first().address());
    return addr.toString();
}


RemoteInterfaceConnection* InterfaceItem::currentConnection()
{
    kDebug() << m_currentConnection;
    if (m_currentConnection && m_currentConnection->activationState() != Knm::InterfaceConnection::Unknown) {
        return m_currentConnection;
    } else {
        currentConnectionChanged();
        return m_currentConnection;
    }
}

void InterfaceItem::setActivatableList(RemoteActivatableList* activatables)
{
    m_activatables = activatables;
}

void InterfaceItem::currentConnectionChanged()
{
    RemoteInterfaceConnection* remoteconnection = UiUtils::connectionForInterface(m_activatables, m_iface);
    if (remoteconnection) {
        if (m_currentConnection) {
            QObject::disconnect(m_currentConnection, SIGNAL(hasDefaultRouteChanged(bool)));
        }
        m_currentConnection = remoteconnection;

        connect(m_currentConnection, SIGNAL(hasDefaultRouteChanged(bool)),
                                        SLOT(handleHasDefaultRouteChanged(bool)));
        handleHasDefaultRouteChanged(m_currentConnection->hasDefaultRoute());
        return;
    }
    handleHasDefaultRouteChanged(false);
    m_currentConnection = 0;
    return;
}

void InterfaceItem::handleHasDefaultRouteChanged(bool changed)
{
    if (m_hasDefaultRoute == changed) {
        //return;
    }
    m_hasDefaultRoute = changed;
    //kDebug() << "Default Route changed!!" << changed;
    if (m_icon) {
        m_icon->nativeWidget()->setPixmap(interfacePixmap());
    }
    update();
}


void InterfaceItem::pppStats(uint in, uint out)
{
    kDebug() << "PPP Stats. in:" << in << "out:" << out;
}

void InterfaceItem::activeConnectionsChanged()
{
    setConnectionInfo();
}

void InterfaceItem::slotClicked()
{
    emit clicked(m_iface);
}

void InterfaceItem::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED(old_state);
    Q_UNUSED(reason);
    connectionStateChanged((Solid::Control::NetworkInterface::ConnectionState)new_state);
}

void InterfaceItem::handleConnectionStateChange(int new_state)
{
    connectionStateChanged((Solid::Control::NetworkInterface::ConnectionState)new_state);
}

void InterfaceItem::connectionStateChanged(Solid::Control::NetworkInterface::ConnectionState state)
{
    if (m_state == state) {
        return;
    }
    m_state = state;
    // TODO:
    // get the active connections
    // check if any of them affect our interface
    // setActiveConnection on ourself
    // button to connect, disconnect
    m_disconnect = false;
    // Name and info labels
    QString lname = UiUtils::connectionStateToString(state, connectionName());

    switch (state) {
        case Solid::Control::NetworkInterface::Unavailable:
            if (m_iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
                lname = i18nc("wired interface network cable unplugged", "Cable Unplugged");
            }
            setEnabled(false); // FIXME: tone down colors using an animation
            break;
        case Solid::Control::NetworkInterface::Disconnected:
            setEnabled(true);
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::Preparing:
        case Solid::Control::NetworkInterface::Configuring:
        case Solid::Control::NetworkInterface::NeedAuth:
        case Solid::Control::NetworkInterface::IPConfig:
            setEnabled(true);
            m_disconnect = false;
            break;
        case Solid::Control::NetworkInterface::Activated:
            m_disconnect = true;
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterface::Unmanaged:
        case Solid::Control::NetworkInterface::Failed:
        case Solid::Control::NetworkInterface::UnknownState:
            setEnabled(false);
            break;
    }

    // Update connect button
    if (!m_disconnect) {
        //m_disconnectButton->setIcon("dialog-ok");
        //m_disconnectButton->setToolTip(i18n("Connect"));
        m_disconnectButton->hide();
    } else {
        m_disconnectButton->setIcon(KIcon("dialog-close"));
        m_disconnectButton->setToolTip(i18nc("tooltip on disconnect icon", "Disconnect"));
        m_disconnectButton->show();
    }

    m_connectionNameLabel->setText(lname);
    m_icon->nativeWidget()->setPixmap(interfacePixmap());

    kDebug() << "State changed" << lname;
    currentConnectionChanged();
    emit stateChanged();
}

QPixmap InterfaceItem::interfacePixmap(const QString &icon) {
    // Which pixmap should we display with the notification?
    QString overlayIcon = icon;
    if (overlayIcon.isEmpty()) {
        overlayIcon = "network-defaultroute";
    }
    //kDebug() << "painting icon" << overlayIcon;
    QPixmap pmap = KIcon(UiUtils::iconName(m_iface)).pixmap(m_pixmapSize);
    //QPixmap pmap = KIcon(icon).pixmap(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
    if (m_hasDefaultRoute && !pmap.isNull()) {
        QPainter p(&pmap);
        p.drawPixmap(QRect(2,2,18,18), KIcon(overlayIcon).pixmap(QSize(18,18)));
    }
    return pmap;
}

void InterfaceItem::emitDisconnectInterfaceRequest()
{
    if (m_iface) {
        kDebug() << m_iface->uni();
        emit disconnectInterfaceRequested(m_iface->uni());
    }
}

// vim: sw=4 sts=4 et tw=100
