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
    QLabel * wiredHeader = new QLabel(this);
    wiredHeader->setText(i18nc("Label for connection list popup","Wired Networks"));
    //m_notRunning = new QLabel(this);
    //m_notRunning->setText(i18nc("Label for when NetworkManager is not running","The NetworkManager service is not running."));
    m_connectionLayout->addWidget(wiredHeader);
    //m_connectionLayout->addItem(m_notRunning);
    if (Solid::Control::NetworkManager::status() != Solid::Networking::Unknown) {
        ;//m_notRunning->hide();
    }
    m_userSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS), this);
    m_userSettings->setObjectName("user-settings-service");
    m_systemSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS), this);
    m_systemSettings->setObjectName("system-settings-service");

    m_ethernetGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee8023, m_userSettings, m_systemSettings, this);
    m_ethernetGroup->setObjectName("ethernet-interface-group");
    m_ethernetGroup->init();
    m_connectionLayout->addWidget(m_ethernetGroup);

    QLabel * wirelessHeader = new QLabel(this);
    wirelessHeader->setText(i18nc("Label for wifi networks in popup","Wireless Networks"));
    m_wifiGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Ieee80211, m_userSettings, m_systemSettings, this);
    m_wifiGroup->setObjectName("wifi-interface-group");
    m_wifiGroup->init();
    m_gsmGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Gsm, m_userSettings, m_systemSettings, this);
    m_gsmGroup->setObjectName("gsm-interface-group");
    m_gsmGroup->init();

    //InterfaceGroup *cdmaGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Cdma, this);
    //InterfaceGroup *pppoeGroup = new InterfaceGroup(Solid::Control::NetworkInterface::Serial, this);
    QLabel * vpnHeader = new QLabel(this);
    vpnHeader->setText(i18nc("Label for vpn connections in popup","VPN Connections"));
    VpnConnectionGroup * vpnGroup = new VpnConnectionGroup(m_userSettings, m_systemSettings, this);
    vpnGroup->setObjectName("vpn-interface-group");
    vpnGroup->init();
    m_connectionLayout->addWidget(wirelessHeader);

    //m_connectionLayout->setItemSpacing(1, 30);
    m_connectionLayout->addWidget(m_wifiGroup);
    m_connectionLayout->addWidget(m_gsmGroup);
    m_connectionLayout->addWidget(vpnHeader);
    m_connectionLayout->addWidget(vpnGroup);
    m_gsmGroup->show();
    //m_connectionLayout->addItem(cdmaGroup);
    //m_connectionLayout->addItem(pppoeGroup);
    m_layout->addLayout(m_connectionLayout);

    //   then a grid of status labels and buttons
    // +----------------------------+
    // |              | [Manage...] |
    // | Wireless hw switch status  |
    // | [Networking] | [Wireless]  |
    // +----------------------------|
    //QGraphicsGridLayout * gridLayout = new QGraphicsGridLayout(m_layout);
    m_btnManageConnections = new QPushButton(this);
    //m_btnManageConnections->setDrawBackground(true);
    m_btnManageConnections->setMaximumWidth(140);
    m_btnManageConnections->setMaximumHeight(24);
    //m_btnManageConnections->setOrientation(Qt::Horizontal);
    m_btnManageConnections->setIcon(SmallIcon("networkmanager"));
    m_btnManageConnections->setText(i18nc("Button text for showing the Manage Connections KCModule", "Manage..."));

    //gridLayout->addItem(m_btnManageConnections, 0, 0, 1, 2);
    m_layout->addWidget(m_btnManageConnections);
    //m_lblRfkill = new QLabel(this);
    //m_lblRfkill->nativeWidget()->setWordWrap(false);
    //sets the label text
    //managerWirelessHardwareEnabledChanged(Solid::Control::NetworkManager::isWirelessHardwareEnabled());

    //gridLayout->addItem(m_lblRfkill, 1, 0, 1, 2);
    //m_btnEnableNetworking = new QCheckBox(this);
    m_btnEnableWireless = new QCheckBox(this);
    managerWirelessEnabledChanged(Solid::Control::NetworkManager::isWirelessEnabled());
    //m_btnEnableNetworking->setText(i18nc("Label for pushbutton enabling networking", "All Networking"));
    m_btnEnableWireless->setText(i18nc("Label for checkbox enabling wireless", "Wireless"));
    //gridLayout->addItem(m_btnEnableNetworking, 1, 0, 1, 1);
    //gridLayout->addItem(m_btnEnableWireless, 1, 0, 1, 2);
    //m_layout->addItem(gridLayout);
    m_layout->addWidget(m_btnEnableWireless);
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
    //QObject::connect(m_btnEnableNetworking, SIGNAL(toggled(bool)),
    //        this, SLOT(userNetworkingEnabledChanged(bool)));
    QObject::connect(m_btnEnableWireless, SIGNAL(toggled(bool)),
            this, SLOT(userWirelessEnabledChanged(bool)));
    QObject::connect(m_connectionActivationSignalMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(activateConnection(const QString&)));
    QObject::connect(m_connectionDeactivationSignalMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(deactivateConnection(const QString&)));


    QObject::connect(m_ethernetGroup, SIGNAL(updateLayout()), this, SLOT(updateLayout()));
    QObject::connect(m_wifiGroup, SIGNAL(updateLayout()), this, SLOT(updateLayout()));
    updateLayout();
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

void NetworkManagerPopup::managerWirelessEnabledChanged(bool enabled)
{
    m_btnEnableWireless->setChecked(enabled);
}

void NetworkManagerPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        KNotification::event(Event::RfOn, i18nc("Notification for radio kill switch turned on", "Wireless hardware enabled"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("knetworkmanager", "knetworkmanager", KComponentData::SkipMainComponentRegistration));
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is enabled", "Wireless hardware is enabled"));
    } else {
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is not enabled", "Wireless hardware is disabled"));
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
        m_gsmGroup->hide();
        //m_notRunning->show();
    } else {
        m_ethernetGroup->show();
        m_wifiGroup->show();
        m_gsmGroup->show();
        //m_notRunning->hide();
    }
}

#include "networkmanagerpopup.moc"

// vim: sw=4 sts=4 et tw=100
