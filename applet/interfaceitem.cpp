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
#include "remoteutils.h"

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
#include <libnm-qt/device.h>
#include <libnm-qt/wireddevice.h>
#include <libnm-qt/ipv4config.h>
#include <libnm-qt/manager.h>


#include "knmserviceprefs.h"
#include "nm-device-interface.cpp"
#include "nm-ip4-config-interface.cpp"


InterfaceItem::InterfaceItem(NetworkManager::Device * iface, RemoteActivatableList* activatables,  NameDisplayMode mode, QGraphicsWidget * parent) : Plasma::IconWidget(parent),
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
    m_icon->nativeWidget()->setPixmap(interfacePixmap());

    //     interface layout
    m_ifaceNameLabel = new Plasma::Label(this);
    m_ifaceNameLabel->setToolTip(tt);
    m_ifaceNameLabel->nativeWidget()->setWordWrap(true);
    m_ifaceNameLabel->setMaximumHeight(QFontMetrics(KGlobalSettings::generalFont()).height());
    m_ifaceNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 1);

    m_disconnectButton = new Plasma::PushButton(this);
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
        connect(m_iface, SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)),
                this, SLOT(handleConnectionStateChange(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)));
        if (m_iface->type() == NetworkManager::Device::Ethernet) {
            connect(m_iface, SIGNAL(carrierChanged(bool)), this, SLOT(setActive(bool)));
        }
    }
    setNameDisplayMode(mode);

    if (m_iface) {
        if (m_iface->type() == NetworkManager::Device::Ethernet) {
            NetworkManager::WiredDevice* wirediface =
                            static_cast<NetworkManager::WiredDevice*>(m_iface);
            connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(setActive(bool)));
        }
        m_state = NetworkManager::Device::UnknownState;
        connectionStateChanged(m_iface->state());
    }

    setLayout(m_layout);
    m_layout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

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

NetworkManager::Device* InterfaceItem::interface()
{
    return m_iface;
}

void InterfaceItem::setActive(bool active)
{
    Q_UNUSED(active);
    if (m_iface) {
        connectionStateChanged(m_iface->state());
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
        m_interfaceName = UiUtils::interfaceNameLabel(m_iface->uni());
    }
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
    RemoteInterfaceConnection *conn = RemoteUtils::connectionForInterface(m_activatables, m_iface);
    if (conn) {
        return conn->connectionName();
    }
    return QString();
}

void InterfaceItem::setConnectionInfo()
{
    if (m_iface) {
        currentConnectionChanged();
        connectionStateChanged(m_iface->state());
    }
}

QString InterfaceItem::currentIpAddress()
{
    if (!m_iface)
        return QString();

    if (static_cast<NetworkManager::Device::State>(m_iface->state()) != NetworkManager::Device::Activated) {
        return i18nc("label of the network interface", "No IP address.");
    }
    //willtodo: use IPv4Config
    QHostAddress addr;

    OrgFreedesktopNetworkManagerDeviceInterface devIface("org.freedesktop.NetworkManager", m_iface->uni(), QDBusConnection::systemBus());
    if (devIface.isValid()) {
        QDBusObjectPath ip4ConfigPath = devIface.ip4Config();

        OrgFreedesktopNetworkManagerIP4ConfigInterface ip4Iface("org.freedesktop.NetworkManager", ip4ConfigPath.path(), QDBusConnection::systemBus());
        if (ip4Iface.isValid()) {
            QDBusObjectPath ip4ConfigPath;

            // get the first IP address
            qDBusRegisterMetaType<QList<QList<uint> > >();
            QList<QList<uint> > addresses = ip4Iface.addresses();
            foreach (QList<uint> addressList, addresses) {
               if (addressList.count() == 3) {
                    addr.setAddress(ntohl(addressList[0]));
                    break;
                }
            }
        }
    }

    if (addr.isNull()) {
        return i18nc("label of the network interface", "IP display error.");
    }
    return addr.toString();
}


RemoteInterfaceConnection* InterfaceItem::currentConnection()
{
    RemoteInterfaceConnection* remoteconnection = RemoteUtils::connectionForInterface(m_activatables, m_iface);
    if (!remoteconnection) {
        m_currentConnection = 0;
        handleHasDefaultRouteChanged(false);
        return m_currentConnection;
    }

    //kDebug() << m_currentConnection;
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
    RemoteInterfaceConnection* remoteconnection = RemoteUtils::connectionForInterface(m_activatables, m_iface);
    if (remoteconnection) {
        if (m_currentConnection) {
            QObject::disconnect(m_currentConnection, 0, this, 0);
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

void InterfaceItem::handleConnectionStateChange(NetworkManager::Device::State new_state, NetworkManager::Device::State old_state, NetworkManager::Device::StateChangeReason reason)
{
    Q_UNUSED(old_state);
    Q_UNUSED(reason);
    connectionStateChanged(new_state);
}

void InterfaceItem::connectionStateChanged(NetworkManager::Device::State state)
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
    QString lname = UiUtils::connectionStateToString(state, connectionName());

    switch (state) {
        case NetworkManager::Device::Unavailable:
            if (m_iface->type() == NetworkManager::Device::Ethernet) {
                lname = i18nc("wired interface network cable unplugged", "Cable Unplugged");
            }
            setEnabled(false); // FIXME: tone down colors using an animation
            break;
        case NetworkManager::Device::Disconnected:
            setEnabled(true);
            setEnabled(true);
            break;
        case NetworkManager::Device::Preparing:
        case NetworkManager::Device::ConfiguringHardware:
        case NetworkManager::Device::NeedAuth:
        case NetworkManager::Device::ConfiguringIp:
        case NetworkManager::Device::CheckingIp:
        case NetworkManager::Device::WaitingForSecondaries:
        case NetworkManager::Device::Deactivating:
            setEnabled(true);
            m_disconnect = false;
            break;
        case NetworkManager::Device::Activated:
            m_disconnect = true;
            setEnabled(true);
            break;
        case NetworkManager::Device::Unmanaged:
        case NetworkManager::Device::Failed:
        case NetworkManager::Device::UnknownState:
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
        //kDebug() << m_iface->uni();
        emit disconnectInterfaceRequested(m_iface->uni());
    }
}

void InterfaceItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_iface) {
        emit hoverEnter(m_iface->uni());
    }
    IconWidget::hoverEnterEvent(event);
}

void InterfaceItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_iface) {
        emit hoverLeave(m_iface->uni());
    }
    IconWidget::hoverLeaveEvent(event);
}

// vim: sw=4 sts=4 et tw=100
