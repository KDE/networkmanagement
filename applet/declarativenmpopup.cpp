/*
Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>

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

#include "declarativenmpopup.h"

#include <KDebug>

#include <KStandardDirs>

#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>

#include "activatableitem.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remotewirelessinterfaceconnection.h"

// More own includes
#include "interfaceitem.h"
#include "wirelessinterfaceitem.h"
#include "vpninterfaceitem.h"
#include "activatablelistwidget.h"
#include "interfacedetailswidget.h"
#include "typelistwidget.h"
#include "uiutils.h"
#include "paths.h"


DeclarativeNMPopup::DeclarativeNMPopup(QGraphicsWidget *parent) : Plasma::DeclarativeWidget(parent)
{
    listModel = new ConnectionsListModel(parent);

    this->setInitializationDelayed(true);
    this->engine()->rootContext()->setContextProperty("connectionsListModel", listModel);
    this->setQmlPath(KStandardDirs::locate("data", "networkmanagement/qml/NMPopup.qml"));

    this->engine()->rootContext()->setContextProperty("wirelessVisible", QVariant(false));

    connect(this, SIGNAL(finished()), this, SLOT(qmlCreationFinished()));
    connect(NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wwanEnabledChanged(bool)),
            this, SLOT(managerWwanEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wwanHardwareEnabledChanged(bool)),
            this, SLOT(managerWwanHardwareEnabledChanged(bool)));

    foreach (NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
        addInterfaceInternal(iface);
        kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
    }

    readConfig();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(readConfig()));
    dbus.connect("org.kde.kded", "/org/kde/networkmanagement", "org.kde.networkmanagement", "ReloadConfig", this, SLOT(readConfig()));
}

void DeclarativeNMPopup::qmlCreationFinished()
{
    connect(this->rootObject(), SIGNAL(enableWireless(bool)), this, SLOT(updateWireless(bool)));
}

void DeclarativeNMPopup::connectionAdded(ConnectionItem *connection)
{
    listModel->appendRow(connection);
}

void DeclarativeNMPopup::connectionRemoved(ConnectionItem *connection)
{
    listModel->removeItem(connection);
}

void DeclarativeNMPopup::updateWireless(bool checked)
{
    if (NetworkManager::isWirelessEnabled() != checked) {
        NetworkManager::setWirelessEnabled(checked);
    }
}

void DeclarativeNMPopup::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    this->engine()->rootContext()->setContextProperty("wirelessChecked", enabled);
    if (enabled) {
        this->engine()->rootContext()->setContextProperty("wirelessEnabled", enabled);
    }
}

void DeclarativeNMPopup::updateHasWireless(bool checked)
{
    //kDebug() << "UPDATE!!!!!!!!!!!!";
    bool hasWireless = true;
    if (!NetworkManager::isWirelessHardwareEnabled() ||
        !NetworkManager::isNetworkingEnabled() ||
        !NetworkManager::isWirelessEnabled() || !checked) {

        //kDebug () << "networking enabled?" << NetworkManager::isNetworkingEnabled();
        //kDebug () << "wireless hardware enabled?" << NetworkManager::isWirelessHardwareEnabled();
        //kDebug () << "wireless enabled?" << NetworkManager::isWirelessEnabled();

        // either networking is disabled, or wireless is disabled
        hasWireless = false;
    }
    //solid is too slow, we need to see if the checkbox was checked by the user
    if (checked)
        hasWireless = true;
    kDebug() << "After chckboxn" << hasWireless;

    m_hasWirelessInterface = hasWireless;

    if (!m_hasWirelessInterface) {
        kDebug() << "no ifaces";
        hasWireless = false;
    }
    this->engine()->rootContext()->setContextProperty("wirelessVisible", hasWireless);
}


void DeclarativeNMPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    this->engine()->rootContext()->setContextProperty("wirelessEnabled", enabled);
}

void DeclarativeNMPopup::readConfig()
{
    kDebug();
    KNetworkManagerServicePrefs::self()->readConfig();

    //m_networkingCheckBox->setChecked(NetworkManager::isNetworkingEnabled());
    this->engine()->rootContext()->setContextProperty("wirelessChecked", NetworkManager::isWirelessEnabled());

    /* There is a bug in Solid < 4.6.2 where it does not emit the wirelessHardwareEnabledChanged signal.
     * So we always enable the wireless checkbox for versions below 4.6.2. */
#if KDE_IS_VERSION(4,6,2)
    this->engine()->rootContext()->setContextProperty("wirelessChecked", NetworkManager::isWirelessHardwareEnabled());
#else
    //this->engine()->rootContext()->setContextProperty("wirelessEnabled", true);
#endif

    /*m_showMoreButton->setEnabled(NetworkManager::isNetworkingEnabled() &&
                                 NetworkManager::isWirelessEnabled());*/

    /**
    m_wwanCheckBox->nativeWidget()->setCheckState(NetworkManager::isWwanEnabled() ? Qt::Checked : Qt::Unchecked);
    m_wwanCheckBox->setEnabled(NetworkManager::isWwanHardwareEnabled());

    foreach(InterfaceItem * i, m_interfaces) {
        i->setNameDisplayMode(InterfaceItem::InterfaceName);
    }

    QString version = NetworkManager::version();
    if (version.isEmpty()) {
        if (!m_warning) {
            m_warning = new Plasma::Label(this);
        }
        m_warning->setText(i18nc("Warning about wrong NetworkManager version", "NetworkManager is not running. Please start it."));
        m_tab1Layout->addItem(m_warning, 10, 0);
    } else if (compareVersions(version, QString(MINIMUM_NM_VERSION_REQUIRED)) < 0) {
        if (!m_warning) {
            m_warning = new Plasma::Label(this);
        }
        m_warning->setText(i18nc("Warning about wrong NetworkManager version", "We need at least NetworkManager-%1 to work properly, found '%2'. Please upgrade to a newer version.", QString(MINIMUM_NM_VERSION_REQUIRED), version));
        m_tab1Layout->addItem(m_warning, 10, 0);
    } else if (m_warning) {
        m_warning->deleteLater();
        m_warning = 0;
    }
    **/
}

void DeclarativeNMPopup::addInterfaceInternal(NetworkManager::Device *iface)
{
    if (!iface) {
        // the interface might be gone in the meantime...
        return;
    }
    /**
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ifaceItem = 0;
        if (iface->type() == NetworkManager::Device::Wifi) {
            // Create the wireless interface item
            WirelessInterfaceItem* wifiItem = 0;
            wifiItem = new WirelessInterfaceItem(static_cast<NetworkManager::WirelessDevice *>(iface), m_activatables, InterfaceItem::InterfaceName, m_leftWidget);
            ifaceItem = wifiItem;
            wifiItem->setEnabled(NetworkManager::isWirelessEnabled());
            kDebug() << "WiFi added";
            connect(wifiItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        } else {
            // Create the interfaceitem
            ifaceItem = new InterfaceItem(static_cast<NetworkManager::WiredDevice *>(iface), m_activatables, InterfaceItem::InterfaceName, m_leftWidget);
            connect(ifaceItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        }
        connect(ifaceItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
        connect(ifaceItem, SIGNAL(clicked(NetworkManager::Device*)),
                m_connectionList,  SLOT(addInterface(NetworkManager::Device*)));*/
    /**
        connect(ifaceItem, SIGNAL(hoverEnter(QString)), m_connectionList, SLOT(hoverEnter(QString)));
        connect(ifaceItem, SIGNAL(hoverLeave(QString)), m_connectionList, SLOT(hoverLeave(QString)));

        // Catch connection changes
        connect(iface, SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)), this, SLOT(handleConnectionStateChange(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)));
        m_interfaceLayout->addItem(ifaceItem);
        m_interfaces.insert(iface->uni(), ifaceItem);
    }
    */
    updateHasWireless();
    //updateHasWwan();
}
