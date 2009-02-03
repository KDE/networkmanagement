/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2008 Sebastian Kügler <sebas@kde.org>

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

#include "networkmanagerpopup.h"

#include <NetworkManager.h>
#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-gsm.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>

#include <QtDBus>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QSignalMapper>
#include <QPushButton>

#include <KDebug>
#include <KIconLoader>
#include <KLocale>
#include <KNotification>
#include <KPushButton>

#include <solid/control/networkmanager.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

#include "events.h"
#include "../libs/marshalarguments.h"
#include "interfacegroup.h"
#include "remoteconnection.h"
#include "networkmanagersettings.h"
#include "vpnconnectiongroup.h"

NetworkManagerPopup::NetworkManagerPopup(QWidget *parent)
    : QWidget(parent),
    m_connectionActivationSignalMapper(new QSignalMapper(this)),
    m_connectionDeactivationSignalMapper(new QSignalMapper(this))
{
    m_wiredHeader = 0;
    m_ethernetGroup = 0;
    m_wirelessHeader = 0;
    m_ethernetGroup = 0;
    m_vpnHeader = 0;
    m_vpnGroup = 0;
    m_cdmaGroup = 0;
    m_gsmGroup = 0;

    setMinimumWidth(300);
    qDBusRegisterMetaType<QMap<QString, QVariant> >();
    qDBusRegisterMetaType<QMap<QString, QMap<QString, QVariant> > >();

    // containing vertical linear layout
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0,0,0,0);
    //   a vertical list of appropriate connections
    //     header label
    m_connectionLayout = new QVBoxLayout(0);
    m_connectionLayout->setContentsMargins(0,0,0,0);
    m_wiredHeader = new QLabel(this);
    m_wiredHeader->setText(i18nc("Label for connection list popup","Wired Networks"));
    m_connectionLayout->addWidget(m_wiredHeader);
    m_userSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS), this);
    m_userSettings->setObjectName("user-settings-service");
    m_systemSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS), this);
    m_systemSettings->setObjectName("system-settings-service");

    m_ethernetGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee8023, m_userSettings, m_systemSettings, this);
    m_ethernetGroup->setObjectName("ethernet-interface-group");
    m_ethernetGroup->init();
    m_connectionLayout->addWidget(m_ethernetGroup);

    m_wirelessHeader = new QLabel(this);
    m_wirelessHeader->setText(i18nc("Label for wifi networks in popup","Wireless Networks"));
    m_wifiGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee80211, m_userSettings, m_systemSettings, this);
    m_wifiGroup->setObjectName("wifi-interface-group");
    m_wifiGroup->init();

    m_gsmGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Gsm, m_userSettings, m_systemSettings, this);
    m_gsmGroup->setObjectName("gsm-interface-group");
    m_gsmGroup->init();

    m_cdmaGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Cdma, m_userSettings, m_systemSettings, this);
    m_cdmaGroup->setObjectName("gsm-interface-group");
    m_cdmaGroup->init();

    //InterfaceGroup *cdmaGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Cdma, this);
    //InterfaceGroup *pppoeGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Serial, this);
    m_vpnHeader = new QLabel(this);
    m_vpnHeader->setText(i18nc("Label for vpn connections in popup","VPN Connections"));
    m_vpnGroup = new VpnConnectionGroup(m_userSettings, m_systemSettings, this);
    m_vpnGroup->setObjectName("vpn-interface-group");
    m_vpnGroup->init();
    connect(m_vpnGroup, SIGNAL(hideClicked()), SIGNAL(hideVpnGroup()));
    m_connectionLayout->addWidget(m_wirelessHeader);

    //m_connectionLayout->setItemSpacing(1, 30);
    m_connectionLayout->addWidget(m_wifiGroup);
    m_connectionLayout->addWidget(m_gsmGroup);
    m_connectionLayout->addWidget(m_cdmaGroup);
    if (m_vpnGroup->isEmpty()) {
        kDebug() << "VPN is empty, hiding it";
        m_vpnHeader->hide();
        m_vpnGroup->hide();
    }

    m_connectionLayout->addWidget(m_vpnHeader);
    m_connectionLayout->addWidget(m_vpnGroup);
    m_gsmGroup->show();
    m_cdmaGroup->show();
    //m_connectionLayout->addItem(cdmaGroup);
    //m_connectionLayout->addItem(pppoeGroup);
    m_layout->addLayout(m_connectionLayout);

    //QGraphicsGridLayout * gridLayout = new QGraphicsGridLayout(m_layout);
    m_btnManageConnections = new QPushButton(this);
    //m_btnManageConnections->setDrawBackground(true);
    m_btnManageConnections->setMinimumWidth(160);
    m_btnManageConnections->setMaximumHeight(24);
    //m_btnManageConnections->setOrientation(Qt::Horizontal);
    m_btnManageConnections->setIcon(SmallIcon("networkmanager"));
    m_btnManageConnections->setText(i18nc("Button text for showing the Manage Connections KCModule", "Manage Connections..."));

    //gridLayout->addItem(m_btnManageConnections, 0, 0, 1, 2);
    m_layout->addWidget(m_btnManageConnections);
    //m_lblRfkill = new QLabel(this);
    //m_lblRfkill->nativeWidget()->setWordWrap(false);
    //sets the label text
    //managerWirelessHardwareEnabledChanged(Solid::Control::NetworkManager::isWirelessHardwareEnabled());

    //gridLayout->addItem(m_lblRfkill, 1, 0, 1, 2);
    managerWirelessEnabledChanged(Solid::Control::NetworkManager::isWirelessEnabled());
    //m_layout->addItem(gridLayout);
    setLayout(m_layout);
    // connect up the buttons and the manager's signals
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)),
            this, SLOT(managerStatusChanged(Solid::Networking::Status)));
    //QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
    //        this, SLOT(networkInterfaceAdded(const QString&)));
    //QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
    //        this, SLOT(networkInterfaceRemoved(const QString&)));
    QObject::connect(m_btnManageConnections, SIGNAL(clicked()),
            this, SIGNAL(manageConnections()));
    QObject::connect(m_connectionActivationSignalMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(activateConnection(const QString&)));
    QObject::connect(m_connectionDeactivationSignalMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(deactivateConnection(const QString&)));


    QObject::connect(m_ethernetGroup, SIGNAL(updateLayout()), this, SLOT(updateLayout()));
    QObject::connect(m_wifiGroup, SIGNAL(updateLayout()), this, SLOT(updateLayout()));
    updateLayout();
    m_wifiGroup->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());
}

NetworkManagerPopup::~NetworkManagerPopup()
{
}

void NetworkManagerPopup::updateLayout()
{
    /* This is a big hack, from an email by Riccardo, let's see if this
     helps a bit updating the sizes
    */
    updateGeometry();
//    resize(sizeHint(Qt::PreferredSize));
//    setMinimumSize(sizeHint(Qt::MinimumSize));

    //kDebug() << "============================== LAYOUT UPDATED.";
}

void NetworkManagerPopup::setShowWired(bool show)
{
    m_showWired = show;
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Ieee8023)) {
        m_wiredHeader->show();
        m_ethernetGroup->show();

    } else {
        m_wiredHeader->hide();
        m_ethernetGroup->hide();
    }
}

void NetworkManagerPopup::setShowWireless(bool show)
{
    m_showWireless = show;
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Ieee80211)) {
        m_wirelessHeader->show();
        m_wifiGroup->show();

    } else {
        m_wirelessHeader->hide();
        m_wifiGroup->hide();
    }
}

void NetworkManagerPopup::setShowVpn(bool show)
{
    m_showVpn = show;
    if (show) {
        m_vpnHeader->show();
        m_vpnGroup->show();

    } else {
        m_vpnHeader->hide();
        m_vpnGroup->hide();
    }
}

void NetworkManagerPopup::setShowCellular(bool show)
{
    m_showCellular = show;
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Cdma)) {
        m_cdmaGroup->show();

    } else {
        m_cdmaGroup->hide();
    }
    if (show && hasInterfaceOfType(Solid::Control::NetworkInterface::Gsm)) {
        m_gsmGroup->show();

    } else {
        m_gsmGroup->hide();
    }
}

void NetworkManagerPopup::managerWirelessEnabledChanged(bool enabled)
{
    if (m_wifiGroup) {
        m_wifiGroup->setEnabled(enabled);
    }
}

void NetworkManagerPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        KNotification::event(Event::RfOn, i18nc("Notification for radio kill switch turned on", "Wireless hardware enabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
        //m_lblRfkill->setText(i18nc("Label text when hardware wireless is enabled", "Wireless hardware is enabled"));
    } else {
        //m_lblRfkill->setText(i18nc("Label text when hardware wireless is not enabled", "Wireless hardware is disabled"));
        KNotification::event(Event::RfOff, i18nc("Notification for radio kill switch turned on", "Wireless hardware disabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
    }
    updateLayout();
}

void NetworkManagerPopup::userNetworkingEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setNetworkingEnabled(enabled);
}

void NetworkManagerPopup::userWirelessEnabledChanged(bool enabled)
{
    kDebug() << enabled;
    Solid::Control::NetworkManager::setWirelessEnabled(enabled);
}

void NetworkManagerPopup::activateConnection(const QString& connection)
{
    kDebug() << connection;
    updateLayout();
#if 0
    NetworkManagerSettings* service;
    if (connection.startsWith(QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS))) {
        service = m_systemSettings;
    } else {
        service = m_userSettings;
    }
    // get the actual connection interface
    RemoteConnection * connectionIface =
        service->findConnection(connection.section('%', 1) );
    if (connectionIface) {
        QVariantMapMap settings = connectionIface->GetSettings();
        foreach (QString key, settings.keys()) {
            kDebug() << key << " - " << settings.value(key);
        }
    }
    QStringList possibleInterfaces = interfacesForConnection(connectionIface);
    // for now, always take the first
    // eventually popup a dialog to ask which one to use
    if (possibleInterfaces.count()) {
        kDebug() << "Activating connection " << service->service() + " " +connectionIface->path() << " on device " << possibleInterfaces.first();
        Solid::Control::NetworkManager::activateConnection(possibleInterfaces.first(), service->service() + " " + connectionIface->path(), QVariantMap());
    }
#endif
}

void NetworkManagerPopup::deactivateConnection(const QString& connection)
{
    kDebug() << connection;
    updateLayout();
}

void NetworkManagerPopup::managerStatusChanged(Solid::Networking::Status status)
{
    if (Solid::Networking::Unknown == status ) {
        m_ethernetGroup->hide();
        m_wifiGroup->hide();
        m_cdmaGroup->hide();
        m_gsmGroup->hide();
    } else {
        m_ethernetGroup->show();
        m_wifiGroup->show();
        m_cdmaGroup->hide();
        m_gsmGroup->show();
    }
}

void NetworkManagerPopup::setWirelessNetworkDisplayLimit(uint limit)
{
    m_numberOfWlans = limit;
    m_wifiGroup->setNetworksLimit(m_numberOfWlans);
}

bool NetworkManagerPopup::hasInterfaceOfType(Solid::Control::NetworkInterface::Type type)
{
    foreach (Solid::Control::NetworkInterface * interface,
            Solid::Control::NetworkManager::networkInterfaces()) {

        if (interface->type() == type) {
            return true;
        }
    }
    return false;
}

#include "networkmanagerpopup.moc"

// vim: sw=4 sts=4 et tw=100
