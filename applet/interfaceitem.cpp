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
#include "remoteactivatablelist.h"
#include "remoteinterfaceconnection.h"

#include <arpa/inet.h>

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
#include <solid/control/networkipv4confignm09.h>
#include <solid/control/networkmanager.h>

#include "knmserviceprefs.h"
#include "nm-device-interface.cpp"
#include "nm-ip4-config-interface.cpp"


InterfaceItem::InterfaceItem(Solid::Control::NetworkInterfaceNm09 * iface, RemoteActivatableList* activatables,  NameDisplayMode mode, QGraphicsWidget * parent) : Plasma::IconWidget(parent),
    m_currentConnection(0),
    m_iface(iface),
    m_activatables(activatables),
    m_icon(0),
    m_connectionNameLabel(0),
    m_nameMode(mode),
    m_enabled(false),
    m_hasDefaultRoute(false),
    m_starting(true)
{
    connect(m_activatables, SIGNAL(disappeared()), this, SLOT(serviceDisappeared()));
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)), SLOT(activatableAdded(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), SLOT(activatableRemoved(RemoteActivatable*)));
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
    m_layout->setColumnFixedWidth(2, 16); // FIXME: spacing?

    m_icon = new Plasma::Label(this);
    m_icon->setToolTip(tt);
    m_icon->setMinimumHeight(m_pixmapSize.height());
    m_icon->setMaximumHeight(m_pixmapSize.height());

    setMinimumHeight(m_pixmapSize.height()+6);
    m_layout->addItem(m_icon, 0, 0, 2, 1);
    m_icon->nativeWidget()->setPixmap(interfacePixmap());

    //     interface layout
    m_ifaceNameLabel = new Plasma::Label(this);
    m_ifaceNameLabel->setToolTip(tt);
    m_ifaceNameLabel->nativeWidget()->setWordWrap(true);
    m_ifaceNameLabel->setMaximumHeight(QFontMetrics(KGlobalSettings::generalFont()).height());
    m_ifaceNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 1);

    m_disconnectButton = new Plasma::IconWidget(this);
    m_disconnectButton->setMaximumHeight(16);
    m_disconnectButton->setMaximumWidth(16);
    m_disconnectButton->setIcon(KIcon("dialog-close"));
    m_disconnectButton->setToolTip(i18nc("tooltip on disconnect icon", "Disconnect"));
    m_disconnectButton->hide();
    m_disconnect = false;
    // forward disconnect signal
    connect(m_disconnectButton, SIGNAL(clicked()), this, SLOT(emitDisconnectInterfaceRequest()));

    m_layout->addItem(m_disconnectButton, 0, 2, 1, 1, Qt::AlignRight);

    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setToolTip(tt);
    m_connectionNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_connectionNameLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionNameLabel->nativeWidget()->setWordWrap(false);
    m_connectionNameLabel->nativeWidget()->setMaximumWidth(210);
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
        connect(m_iface.data(), SIGNAL(connectionStateChanged(int,int,int)),
                this, SLOT(handleConnectionStateChange(int,int,int)));
        connect(m_iface.data(), SIGNAL(linkUpChanged(bool)), this, SLOT(setActive(bool)));
    }
    setNameDisplayMode(mode);

    if (m_iface) {
        if (m_iface.data()->type() == Solid::Control::NetworkInterfaceNm09::Ethernet) {
            Solid::Control::WiredNetworkInterfaceNm09* wirediface =
                            static_cast<Solid::Control::WiredNetworkInterfaceNm09*>(m_iface.data());
            connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(setActive(bool)));
        }
        m_state = Solid::Control::NetworkInterfaceNm09::UnknownState;
        connectionStateChanged(static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(m_iface.data()->connectionState()));
    }

    m_layout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setLayout(m_layout);

    connect(this, SIGNAL(clicked()), this, SLOT(slotClicked()));

    qreal targetOpacity = m_enabled ? 1.0 : 0.7;
    // Fade in when this widget appears
    Plasma::Animation* fadeAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    fadeAnimation->setTargetWidget(this);
    fadeAnimation->setProperty("startOpacity", 0.0);
    fadeAnimation->setProperty("targetOpacity", targetOpacity);
    fadeAnimation->start();
    m_starting = false;
}

InterfaceItem::~InterfaceItem()
{
}

void InterfaceItem::disappear()
{
    Plasma::Animation* fadeAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    fadeAnimation->setTargetWidget(this);
    fadeAnimation->setProperty("startOpacity", 1.0);
    fadeAnimation->setProperty("targetOpacity", 0.0);
    fadeAnimation->start();
    connect(fadeAnimation, SIGNAL(finished()), this, SIGNAL(disappearAnimationFinished()));
}

void InterfaceItem::showItem(QGraphicsWidget* widget, bool show)
{
    Plasma::Animation* fadeAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    fadeAnimation->setTargetWidget(widget);
    widget->show();
    if (show) {
        fadeAnimation->setProperty("startOpacity", 0.0);
        fadeAnimation->setProperty("targetOpacity", 1.0);
    } else {
        fadeAnimation->setProperty("startOpacity", 1.0);
        fadeAnimation->setProperty("targetOpacity", 0.0);
    }
    fadeAnimation->start();
}

QString InterfaceItem::label()
{
    return m_ifaceNameLabel->text();
}

Solid::Control::NetworkInterfaceNm09* InterfaceItem::interface()
{
    return m_iface.data();
}

void InterfaceItem::setActive(bool active)
{
    Q_UNUSED(active);
    if (m_iface) {
        connectionStateChanged(static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(m_iface.data()->connectionState()));
    }
}

void InterfaceItem::setEnabled(bool enable)
{
    m_enabled = enable;
    Plasma::Animation* fadeAnimation = Plasma::Animator::create(Plasma::Animator::FadeAnimation);
    fadeAnimation->setTargetWidget(this);
    if (enable) {
        showItem(m_connectionInfoIcon, false);
        fadeAnimation->setProperty("startOpacity", 0.7);
        fadeAnimation->setProperty("targetOpacity", 1.0);
    } else {
        fadeAnimation->setProperty("startOpacity", 1.0);
        fadeAnimation->setProperty("targetOpacity", 0.7);
    }
    if (!m_starting) {
        // we only animate when setEnabled is not called during initialization,
        // as that would conflict with the appear animation
        fadeAnimation->start();
    }
}

void InterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if (m_iface) {
        m_interfaceName = UiUtils::interfaceNameLabel(m_iface.data()->uni());
    }
    if (m_nameMode == InterfaceName) {
        m_ifaceNameLabel->setText(QString("<b>%1</b>").arg(m_interfaceName));
    } else if (m_nameMode == HardwareName) {
        if (m_iface) {
            m_ifaceNameLabel->setText(QString("<b>%1</b>").arg(m_iface.data()->interfaceName()));
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
    if (m_currentConnection) {
        return m_currentConnection->connectionName();
    }
    return QString();
}

void InterfaceItem::setConnectionInfo()
{
    if (m_iface) {
        currentConnectionChanged();
        connectionStateChanged(static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(m_iface.data()->connectionState()));
    }
}

QString InterfaceItem::currentIpAddress()
{
    if (!m_iface)
        return QString();

    if (static_cast<Solid::Control::NetworkInterfaceNm09::ConnectionState>(m_iface.data()->connectionState()) != Solid::Control::NetworkInterfaceNm09::Activated) {
        return i18nc("label of the network interface", "No IP address.");
    }

    QHostAddress addr;

    OrgFreedesktopNetworkManagerDeviceInterface devIface(NM_DBUS_SERVICE, m_iface.data()->uni(), QDBusConnection::systemBus());
    if (devIface.isValid()) {
        addr.setAddress(ntohl(devIface.ip4Address()));
    }

    if (addr.isNull()) {
        return i18nc("label of the network interface", "IP display error.");
    }
    return addr.toString();
}


RemoteInterfaceConnection* InterfaceItem::currentConnection()
{
    return m_currentConnection;
}

void InterfaceItem::setActivatableList(RemoteActivatableList* activatables)
{
    m_activatables = activatables;
}

void InterfaceItem::currentConnectionChanged()
{
    updateCurrentConnection(m_activatables->connectionForInterface(m_iface.data()));
}

void InterfaceItem::updateCurrentConnection(RemoteInterfaceConnection * ic)
{
    if (ic) {
        if (m_currentConnection) {
            QObject::disconnect(m_currentConnection, 0, this, 0);
        }
        m_currentConnection = ic;

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
    m_hasDefaultRoute = changed;
    //kDebug() << "Default Route changed!!" << changed;
    m_icon->nativeWidget()->setPixmap(interfacePixmap());
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
    emit clicked(m_iface.data());
}

void InterfaceItem::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED(old_state);
    Q_UNUSED(reason);
    connectionStateChanged((Solid::Control::NetworkInterfaceNm09::ConnectionState)new_state);
}

void InterfaceItem::handleConnectionStateChange(int new_state)
{
    connectionStateChanged((Solid::Control::NetworkInterfaceNm09::ConnectionState)new_state);
}

void InterfaceItem::connectionStateChanged(Solid::Control::NetworkInterfaceNm09::ConnectionState state, bool updateConnection)
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
    bool old_disco = m_disconnect;

    m_disconnect = false;
    // Name and info labels
    QString lname;
    if (updateConnection) {
        currentConnectionChanged();
    }
    if (m_currentConnection) {
        lname = UiUtils::connectionStateToString(state, m_currentConnection->connectionName());
    } else {
        lname = UiUtils::connectionStateToString(state, QString());
        // to allow updating connection's name in the next call of connectionStateChanged()
        // even if the state has not changed.
        m_state = Solid::Control::NetworkInterfaceNm09::UnknownState;
    }

    switch (state) {
        case Solid::Control::NetworkInterfaceNm09::Unavailable:
            if (m_iface.data()->type() == Solid::Control::NetworkInterfaceNm09::Ethernet) {
                lname = i18nc("wired interface network cable unplugged", "Cable Unplugged");
            }
            setEnabled(false); // FIXME: tone down colors using an animation
            break;
        case Solid::Control::NetworkInterfaceNm09::Disconnected:
        case Solid::Control::NetworkInterfaceNm09::Deactivating:
            setEnabled(true);
            break;
        case Solid::Control::NetworkInterfaceNm09::Preparing:
        case Solid::Control::NetworkInterfaceNm09::Configuring:
        case Solid::Control::NetworkInterfaceNm09::NeedAuth:
        case Solid::Control::NetworkInterfaceNm09::IPConfig:
        case Solid::Control::NetworkInterfaceNm09::IPCheck:
        case Solid::Control::NetworkInterfaceNm09::Secondaries:
        case Solid::Control::NetworkInterfaceNm09::Activated:
            setEnabled(true);
            m_disconnect = true;
            break;
        case Solid::Control::NetworkInterfaceNm09::Unmanaged:
        case Solid::Control::NetworkInterfaceNm09::Failed:
        case Solid::Control::NetworkInterfaceNm09::UnknownState:
            setEnabled(false);
            break;
    }

    // Update connect button
    if (old_disco != m_disconnect) {
        showItem(m_disconnectButton, m_disconnect);
    }
    m_connectionNameLabel->setText(lname);
    m_icon->nativeWidget()->setPixmap(interfacePixmap());

    //kDebug() << "State changed" << lname;
    emit stateChanged();
}

QPixmap InterfaceItem::interfacePixmap(const QString &icon) {
    // Which pixmap should we display with the notification?
    QString overlayIcon = icon;
    if (overlayIcon.isEmpty()) {
        overlayIcon = "network-defaultroute";
    }
    //kDebug() << "painting icon" << overlayIcon;
    QPixmap pmap = KIcon(UiUtils::iconName(m_iface.data())).pixmap(m_pixmapSize);
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
        //kDebug() << m_iface->uni();
        emit disconnectInterfaceRequested(m_iface.data()->uni());
    }
}

void InterfaceItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_iface) {
        emit hoverEnter(m_iface.data()->uni());
    }
    IconWidget::hoverEnterEvent(event);
}

void InterfaceItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_iface) {
        emit hoverLeave(m_iface.data()->uni());
    }
    IconWidget::hoverLeaveEvent(event);
}

void InterfaceItem::serviceDisappeared()
{
    m_currentConnection = 0;
}

void InterfaceItem::activatableAdded(RemoteActivatable * activatable)
{
    if (m_iface && RemoteActivatableList::isConnectionForInterface(activatable, m_iface.data())) {
        updateCurrentConnection(qobject_cast<RemoteInterfaceConnection*>(activatable));

        /* Sometimes the activatableAdded signal arrives after the connectionStateChanged
           signal, so update the interface state here but do not search for current connection
           since it is already known. */
        connectionStateChanged(m_iface.data()->connectionState(), false);
    }
}

void InterfaceItem::activatableRemoved(RemoteActivatable * activatable)
{
    if (activatable == m_currentConnection) {
        m_currentConnection = 0;
    }
}
// vim: sw=4 sts=4 et tw=100
