/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008, 2009 Sebastian K?gler <sebas@kde.org>

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

#include <QGraphicsGridLayout>
#include <QLabel>

#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>
#include <KIcon>
#include <kdeversion.h>

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


InterfaceItem::InterfaceItem(Solid::Control::NetworkInterface * iface, NameDisplayMode mode, QGraphicsItem * parent) : Plasma::IconWidget(parent),
    m_iface(iface),
    m_connectionNameLabel(0),
    m_connectionInfoLabel(0),
    m_nameMode(mode),
    m_enabled(false),
    m_unavailableText(i18nc("Label for network interfaces that cannot be activated", "Unavailable"))
{
    setDrawBackground(true);
    //setAcceptHoverEvents(false);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_layout = new QGraphicsGridLayout(this);
    m_layout->setVerticalSpacing(0);
    m_layout->setColumnSpacing(0, 8);
    m_layout->setColumnSpacing(1, 4);
    m_layout->setColumnSpacing(2, 6);
    m_layout->setRowSpacing(0, 6);
    m_layout->setRowSpacing(1, 6);
    m_layout->setRowSpacing(2, 6);
    m_layout->setPreferredWidth(240);
    m_layout->setColumnFixedWidth(0, 48);
    m_layout->setColumnMinimumWidth(1, 104);
    m_layout->setColumnFixedWidth(2, 60); // FIXME: spacing?
    m_layout->setColumnFixedWidth(3, 22); // FIXME: spacing?

    m_icon = new Plasma::IconWidget(this);
    m_icon->setMinimumHeight(48);
    m_icon->setMaximumHeight(48);
    //m_icon->setAcceptHoverEvents(false);
    m_layout->addItem(m_icon, 0, 0, 2, 1);

    QString icon;

    m_interfaceName = UiUtils::interfaceNameLabel(m_iface->uni());
    kDebug() << "^^^^^^^^^^^^^^^^^" << m_interfaceName;

    m_icon->setIcon(UiUtils::iconName(m_iface));

    setDrawBackground(true);
    //     interface layout
    m_ifaceNameLabel = new Plasma::Label(this);
    m_ifaceNameLabel->setText(m_interfaceName);
    m_ifaceNameLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_ifaceNameLabel, 0, 1, 1, 2);

    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setMaximumHeight(22);
    m_connectButton->setMaximumWidth(22);
    m_connectButton->setIcon("dialog-cancel");
    m_connectButton->setToolTip(i18n("Disconnect"));
    m_connectButton->hide(); // Shown when hovered

    connect(m_connectButton, SIGNAL(clicked()), this, SLOT(connectButtonClicked()));

    m_layout->addItem(m_connectButton, 0, 2, 1, 1, Qt::AlignRight);

    //     active connection name
    m_connectionNameLabel = new Plasma::Label(this);
    m_connectionNameLabel->setText(i18n("[not updated yet]")); // TODO: check connection status
    m_connectionNameLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionNameLabel->nativeWidget()->setWordWrap(false);
    m_layout->addItem(m_connectionNameLabel, 1, 1, 1, 2);

    //       IP address
    m_connectionInfoLabel = new Plasma::Label(this);
    m_connectionInfoLabel->nativeWidget()->setFont(KGlobalSettings::smallestReadableFont());
    m_connectionInfoLabel->nativeWidget()->setWordWrap(false);
    m_connectionInfoLabel->setText(i18n("<b>IP Address:</b> dum.my.ip.addr"));
    m_layout->addItem(m_connectionInfoLabel, 2, 1, 1, 2, Qt::AlignCenter);

    //       security
    m_connectionInfoIcon = new Plasma::IconWidget(this);
    m_connectionInfoIcon->setMinimumHeight(22);
    m_connectionInfoIcon->setMinimumWidth(22);
    m_connectionInfoIcon->setMaximumHeight(22);
    //m_connectionInfoIcon->setAcceptHoverEvents(false);
    m_layout->addItem(m_connectionInfoIcon, 2, 3, 1, 1, Qt::AlignRight);
    m_connectionInfoIcon->hide(); // hide by default, we'll enable it later

    // Forward state between icon and this widget
    connect(m_icon, SIGNAL(pressed(bool)), this, SLOT(setPressed(bool)));
    connect(this, SIGNAL(pressed(bool)), m_icon, SLOT(setPressed(bool)));
    connect(m_icon, SIGNAL(clicked()), this, SLOT(itemClicked()));

    connect(this, SIGNAL(clicked()), this, SLOT(itemClicked()));

    connect(m_iface, SIGNAL(connectionStateChanged(int)),
            this, SLOT(connectionStateChanged(int)));
    connect(m_iface, SIGNAL(connectionStateChanged(int,int,int)),
            this, SLOT(handleConnectionStateChange(int,int,int)));
    connect(m_iface, SIGNAL(linkUpChanged(bool)), this, SLOT(setActive(bool)));

    if (m_iface->type() == Solid::Control::NetworkInterface::Ieee8023) {
        Solid::Control::WiredNetworkInterface* wirediface =
                        static_cast<Solid::Control::WiredNetworkInterface*>(m_iface);
        connect(wirediface, SIGNAL(carrierChanged(bool)), this, SLOT(setActive(bool)));
    }
    setNameDisplayMode(mode);

    connectionStateChanged(m_iface->connectionState());
    setLayout(m_layout);
    m_layout->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

}

void InterfaceItem::itemClicked()
{
    emit clicked(m_iface->type());
}

QString InterfaceItem::label()
{
    return m_ifaceNameLabel->text();
}

void InterfaceItem::setActive(bool active)
{
    kDebug() << "+ + + + + + Active?" << active;
    connectionStateChanged(m_iface->connectionState());
}

void InterfaceItem::setEnabled(bool enable)
{
    m_enabled = enable;
    m_icon->setEnabled(enable);
    m_connectionInfoLabel->setEnabled(enable);
    m_connectionNameLabel->setEnabled(enable);
    m_ifaceNameLabel->setEnabled(enable);
    m_connectButton->setEnabled(enable);
    m_connectionInfoIcon->setEnabled(enable);
}

InterfaceItem::~InterfaceItem()
{
}

void InterfaceItem::setNameDisplayMode(NameDisplayMode mode)
{
    m_nameMode = mode;
    if (m_nameMode == InterfaceName) {
        m_ifaceNameLabel->setText(QString("<b>%1</b>").arg(m_interfaceName));
    } else if (m_nameMode == HardwareName) {
        m_ifaceNameLabel->setText(QString("<b>%1</b>").arg(m_iface->interfaceName()));

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
    return QString();
}

void InterfaceItem::setConnectionInfo()
{
    connectionStateChanged(m_iface->connectionState());
    return;
    if (m_connectionInfoLabel && m_connectionNameLabel) {
        if (m_iface->connectionState() == Solid::Control::NetworkInterface::Activated) {
            if (connectionName().isEmpty()) {
                m_connectionNameLabel->setText(i18nc("interface is connected", "Connected"));
            } else {
                m_connectionNameLabel->setText(i18nc("wireless interface is connected", "Connected to %1", connectionName()));
            }
            m_connectionInfoLabel->setText(i18nc("ip address of the network interface", "Address: %1", currentIpAddress()));
            //kDebug() << "addresses non-empty" << m_currentIp;
        }
    }
}

QString InterfaceItem::currentIpAddress()
{
    if (m_iface->connectionState() != Solid::Control::NetworkInterface::Activated) {
        return i18n("No IP address.");
    }
    Solid::Control::IPv4Config ip4Config = m_iface->ipV4Config();
    QList<Solid::Control::IPv4Address> addresses = ip4Config.addresses();
    if (addresses.isEmpty()) {
        return i18n("IP display error.");
    }
    QHostAddress addr(addresses.first().address());
    return addr.toString();
}

void InterfaceItem::pppStats(uint in, uint out)
{
    kDebug() << "PPP Stats. in:" << in << "out:" << out;
}

void InterfaceItem::activeConnectionsChanged()
{
    setConnectionInfo();
}

void InterfaceItem::handleConnectionStateChange(int new_state, int old_state, int reason)
{
    Q_UNUSED(old_state);
    Q_UNUSED(reason);
    connectionStateChanged((Solid::Control::NetworkInterface::ConnectionState)new_state);
}

void InterfaceItem::connectionStateChanged(Solid::Control::NetworkInterface::ConnectionState state)
{
    // TODO:
    // get the active connections
    // check if any of them affect our interface
    // setActiveConnection on ourself

    // button to connect, disconnect
    m_disconnect = false;
    // Name and info labels
    QString lname = UiUtils::connectionStateToString(state);
    QString linfo;

    switch (state) {
        case Solid::Control::NetworkInterface::Unavailable:
            setEnabled(false);
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
            m_disconnect = true;
            break;
        case Solid::Control::NetworkInterface::Activated:
            if (connectionName().isEmpty()) {
                lname = i18nc("wireless interface is connected", "Connected");
            } else {
                lname = i18nc("wireless interface is connected", "Connected to %1", connectionName());
            }
            linfo = i18nc("ip address of the network interface", "Address: %1", currentIpAddress());
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
        //m_connectButton->setIcon("dialog-ok");
        //m_connectButton->setToolTip(i18n("Connect"));
        m_connectButton->hide();
    } else {
        m_connectButton->setIcon("dialog-cancel");
        m_connectButton->setToolTip(i18n("Disconnect"));
        m_connectButton->show();
    }
    m_connectionNameLabel->setText(lname);
    m_connectionInfoLabel->setText(linfo);

    //kDebug() << "State changed" << lname << linfo;

    emit stateChanged();
}

QPixmap InterfaceItem::statePixmap(const QString &icon) {
    // Which pixmap should we display with the notification?
    return KIcon(icon).pixmap(QSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium));
}

// vim: sw=4 sts=4 et tw=100
