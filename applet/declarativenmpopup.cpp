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

#include <KToolInvocation>
#include <KStandardDirs>

#include <QtDeclarative>

#include "activatableitem.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remotewirelessinterfaceconnection.h"

// More own includes
#include "declarativeinterfaceitem.h"
#include "wirelessinterfaceitem.h"
#include "vpninterfaceitem.h"
#include "activatablelistwidget.h"
#include "interfacedetailswidget.h"
#include "typelistwidget.h"
#include "uiutils.h"
#include "paths.h"


DeclarativeNMPopup::DeclarativeNMPopup(RemoteActivatableList * activatableList, QGraphicsWidget *parent) :
    Plasma::DeclarativeWidget(parent),
    m_activatables(activatableList)
{
    listModel = new ConnectionsListModel(m_activatables, parent);
    interfaceListModel = new InterfacesListModel(parent);

    qmlRegisterType<InterfaceDetailsWidget>("InterfaceDetails", 0, 1, "InterfaceDetailsWidget");

    this->setInitializationDelayed(true);
    this->engine()->rootContext()->setContextProperty("connectionsListModel", listModel);
    this->engine()->rootContext()->setContextProperty("interfacesListModel", interfaceListModel);
    this->setQmlPath(KStandardDirs::locate("data", "networkmanagement/qml/NMPopup.qml"));

    this->engine()->rootContext()->setContextProperty("wirelessVisible", QVariant(false));
    this->engine()->rootContext()->setContextProperty("mobileVisible", QVariant(false));

    connect(listModel, SIGNAL(showInterfaceDetails(QString)), SLOT(showInterfaceDetails(QString)));
    connect(interfaceListModel, SIGNAL(updateTraffic(NetworkManager::Device *)), this, SLOT(manageUpdateTraffic(NetworkManager::Device *)));
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

    connect(NetworkManager::notifier(), SIGNAL(deviceAdded(QString)),
            SLOT(interfaceAdded(QString)));
    connect(NetworkManager::notifier(), SIGNAL(deviceRemoved(QString)),
            SLOT(interfaceRemoved(QString)));

    readConfig();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(readConfig()));
    dbus.connect("org.kde.kded", "/org/kde/networkmanagement", "org.kde.networkmanagement", "ReloadConfig", this, SLOT(readConfig()));
}

void DeclarativeNMPopup::qmlCreationFinished()
{
    connect(this->rootObject(), SIGNAL(enableWireless(bool)), this, SLOT(updateWireless(bool)));
    connect(this->rootObject(), SIGNAL(enableMobile(bool)), this, SLOT(updateMobile(bool)));
    connect(this->rootObject(), SIGNAL(settingsClicked()), this, SLOT(manageConnections()));
    connect(this->rootObject(), SIGNAL(noDeviceSelected()), this, SLOT(manageSelection()));

}

void DeclarativeNMPopup::managerWwanEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wwan enable state" << enabled;
    this->engine()->rootContext()->setContextProperty("mobileChecked", enabled ? Qt::Checked : Qt::Unchecked);
    if (enabled) {
        this->engine()->rootContext()->setContextProperty("mobileEnabled", enabled);
    }
}

void DeclarativeNMPopup::interfaceAdded(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        return;
    }
    NetworkManager::Device * iface = NetworkManager::findNetworkInterface(uni);
    if (iface) {
        kDebug() << "Interface Added:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
        addInterfaceInternal(iface);
    }
}

void DeclarativeNMPopup::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        // To prevent crashes when the interface removed is the one in interfaceDetailsWidget.
        // the m_iface pointer in interfaceDetailsWidget become invalid in this case.
        /**
        if (m_interfaceDetailsWidget && uni == m_interfaceDetailsWidget->getLastIfaceUni()) {
            m_interfaceDetailsWidget->setInterface(0, false);
            // Since it is invalid go back to "main" window.
            m_connectionsTabBar->setCurrentIndex(ConnectionsTabIndex);
        }
        **/
        DeclarativeInterfaceItem* item = m_interfaces.take(uni);
        interfaceListModel->removeItem(item);
        //connect(item, SIGNAL(disappearAnimationFinished()), this, SLOT(deleteInterfaceItem()));
        //item->disappear();
        updateHasWireless();
        updateHasWwan();
    }
}

void DeclarativeNMPopup::manageUpdateTraffic(NetworkManager::Device *device)
{
    kDebug() << "handle traffic changes";
    if(this->rootObject()) {
        this->rootObject()->findChild<InterfaceDetailsWidget*>("traffic")->setInterface(device);
        this->rootObject()->findChild<InterfaceDetailsWidget*>("traffic")->setUpdateEnabled(true);
        QMetaObject::invokeMethod(this->rootObject(), "detailsWidget");
        listModel->setDeviceToFilter(device);
    }
}

void DeclarativeNMPopup::managerWwanHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wwan enable switch state changed" << enabled;
    this->engine()->rootContext()->setContextProperty("mobileVisible", enabled);
}

void DeclarativeNMPopup::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "--icon" << "networkmanager" << "kcm_networkmanagement" << "kcm_networkmanagement_tray";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void DeclarativeNMPopup::manageSelection()
{
    listModel->setDeviceToFilter(0);
}

void DeclarativeNMPopup::connectionAdded(ConnectionItem *connection)
{
    listModel->updateConnectionsList();
}

void DeclarativeNMPopup::connectionRemoved(ConnectionItem *connection)
{
    listModel->updateConnectionsList();
}

void DeclarativeNMPopup::updateWireless(bool checked)
{
    if (NetworkManager::isWirelessEnabled() != checked) {
        NetworkManager::setWirelessEnabled(checked);
    }
}

void DeclarativeNMPopup::updateMobile(bool checked)
{
    if (NetworkManager::isWwanEnabled() != checked) {
        NetworkManager::setWwanEnabled(checked);
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
    } else {
        listModel->removeHiddenItem();
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
    } else {
        //listModel->insertHiddenItem();
    }
    this->engine()->rootContext()->setContextProperty("wirelessVisible", hasWireless);
}


void DeclarativeNMPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    this->engine()->rootContext()->setContextProperty("wirelessEnabled", enabled);
    //if(!enabled) listModel->removeHiddenItem();
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
    this->engine()->rootContext()->setContextProperty("wirelessEnabled", QVariant(true));
#endif

    /*m_showMoreButton->setEnabled(NetworkManager::isNetworkingEnabled() &&
                                 NetworkManager::isWirelessEnabled());*/

    this->engine()->rootContext()->setContextProperty("mobileChecked", NetworkManager::isWwanEnabled());
    this->engine()->rootContext()->setContextProperty("mobileEnabled", NetworkManager::isWwanHardwareEnabled());
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

    if (!m_interfaces.contains(iface->uni())) {
        DeclarativeInterfaceItem * ifaceItem = 0;
        if (iface->type() == NetworkManager::Device::Wifi) {
            // Create the wireless interface item
            //WirelessInterfaceItem* wifiItem = 0;
            //wifiItem = new WirelessInterfaceItem(static_cast<NetworkManager::WirelessDevice *>(iface), m_activatables, InterfaceItem::InterfaceName, m_leftWidget);
            ifaceItem = new DeclarativeInterfaceItem(iface, m_activatables, DeclarativeInterfaceItem::InterfaceName, this);
            //wifiItem->setEnabled(NetworkManager::isWirelessEnabled());
            kDebug() << "WiFi added";
            //connect(wifiItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        } else {
            // Create the interfaceitem
            kDebug() << "Interface Item added";
            ifaceItem = new DeclarativeInterfaceItem(iface, m_activatables, DeclarativeInterfaceItem::InterfaceName, this);
            //connect(ifaceItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        }
       // connect(ifaceItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
        //connect(ifaceItem, SIGNAL(clicked(NetworkManager::Device*)),
         //       m_connectionList,  SLOT(addInterface(NetworkManager::Device*)));*/

        //connect(ifaceItem, SIGNAL(hoverEnter(QString)), m_connectionList, SLOT(hoverEnter(QString)));
        //connect(ifaceItem, SIGNAL(hoverLeave(QString)), m_connectionList, SLOT(hoverLeave(QString)));

        // Catch connection changes
        connect(iface, SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)), this, SLOT(handleConnectionStateChange(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)));
        //m_interfaceLayout->addItem(ifaceItem);
        m_interfaces.insert(iface->uni(), ifaceItem);
        interfaceListModel->appendRow(ifaceItem);
    }
    updateHasWireless();
    updateHasWwan();
}

void DeclarativeNMPopup::updateHasWwan()
{
    bool hasWwan = false;
    foreach (DeclarativeInterfaceItem* ifaceitem, m_interfaces) {
        NetworkManager::ModemDevice* iface = qobject_cast<NetworkManager::ModemDevice *>(ifaceitem->interface());
        if (iface) {
            hasWwan = true;
            break;
        }
    }
    if (hasWwan) {
        this->engine()->rootContext()->setContextProperty("mobileVisible", QVariant(true));
    } else {
        this->engine()->rootContext()->setContextProperty("mobileVisible", QVariant(false));
    }
}

void DeclarativeNMPopup::showInterfaceDetails(const QString & uni)
{
    DeclarativeInterfaceItem * ifaceItem = m_interfaces.value(uni, 0);
    if (!ifaceItem) {
        return;
    }
    manageUpdateTraffic(ifaceItem->interface());
}
