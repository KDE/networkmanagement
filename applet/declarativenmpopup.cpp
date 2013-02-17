/*
Copyright 2012 Arthur de Souza Ribeiro <arthurdesribeiro@gmail.com>
Copyright 2013 Lamarque V. Souza <lamarque@kde.org>

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
#include <Plasma/Separator>

#include <QtDeclarative>

#include <QtNetworkManager/manager.h>

#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remotewirelessinterfaceconnection.h"

// More own includes
#include "declarativeinterfaceitem.h"
#include "interfacedetailswidget.h"
#include "uiutils.h"
#include "paths.h"


DeclarativeNMPopup::DeclarativeNMPopup(RemoteActivatableList * activatableList, QGraphicsWidget *parent) :
    Plasma::DeclarativeWidget(parent),
    m_activatables(activatableList),
    m_interfaceDetails(0)
{
    listModel = new ConnectionsListModel(m_activatables, parent);
    interfaceListModel = new InterfacesListModel(parent);

    qmlRegisterType<InterfaceDetailsWidget>("InterfaceDetails", 0, 1, "InterfaceDetailsWidget");
    qmlRegisterType<Plasma::Separator>("Separator", 0, 1, "Separator");

    setInitializationDelayed(true);
    m_rootContext = engine()->rootContext();
    m_rootContext->setContextProperty("connectionsListModel", listModel);
    m_rootContext->setContextProperty("interfacesListModel", interfaceListModel);
    m_rootContext->setContextProperty("wirelessVisible", QVariant(false));
    m_rootContext->setContextProperty("mobileVisible", QVariant(false));
    m_rootContext->setContextProperty("warningLabel", QVariant(QString()));
    setQmlPath(KStandardDirs::locate("data", "networkmanagement/qml/NMPopup.qml"));

    connect(listModel, SIGNAL(showInterfaceDetails(QString)), SLOT(showInterfaceDetails(QString)));
    connect(interfaceListModel, SIGNAL(updateTraffic(DeclarativeInterfaceItem*)), this, SLOT(manageUpdateTraffic(DeclarativeInterfaceItem*)));
    connect(this, SIGNAL(finished()), this, SLOT(qmlCreationFinished()));
    connect(NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wwanEnabledChanged(bool)),
            this, SLOT(managerWwanEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wwanHardwareEnabledChanged(bool)),
            this, SLOT(managerWwanHardwareEnabledChanged(bool)));

    foreach(NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
        addInterfaceInternal(iface);
        kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
    }

    connect(NetworkManager::notifier(), SIGNAL(deviceAdded(QString)),
            SLOT(interfaceAdded(QString)));
    connect(NetworkManager::notifier(), SIGNAL(deviceRemoved(QString)),
            SLOT(interfaceRemoved(QString)));
    connect(NetworkManager::notifier(), SIGNAL(networkingEnabledChanged(bool)),
            SLOT(managerNetworkingEnabledChanged(bool)));

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(readConfig()));
    dbus.connect("org.kde.kded", "/org/kde/networkmanagement", "org.kde.networkmanagement", "ReloadConfig", this, SLOT(readConfig()));
}

void DeclarativeNMPopup::qmlCreationFinished()
{
    m_interfaceDetails = rootObject()->findChild<InterfaceDetailsWidget*>("interfaceDetails");
    connect(rootObject(), SIGNAL(enableWireless(bool)), this, SLOT(updateWireless(bool)));
    connect(rootObject(), SIGNAL(enableMobile(bool)), this, SLOT(updateMobile(bool)));
    connect(rootObject(), SIGNAL(settingsClicked()), this, SLOT(manageConnections()));
    connect(rootObject(), SIGNAL(noDeviceSelected()), this, SLOT(manageSelection()));
    connect(rootObject(), SIGNAL(adjustSize(int,int)), this, SLOT(changeSize(int,int)));
    QMetaObject::invokeMethod(rootObject(), "updateSize");
    readConfig();
    addVpnInterface();
}

void DeclarativeNMPopup::changeSize(int width, int height)
{
    setMaximumWidth(width);
    setMaximumHeight(height);
}

void DeclarativeNMPopup::managerWwanEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wwan enable state" << enabled;
    m_rootContext->setContextProperty("mobileChecked", enabled ? Qt::Checked : Qt::Unchecked);
    if (enabled) {
        m_rootContext->setContextProperty("mobileEnabled", enabled);
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
        if (m_interfaceDetails && uni == m_interfaceDetails->getLastIfaceUni()) {
            m_interfaceDetails->setInterface(0, false);
        }
        DeclarativeInterfaceItem* item = m_interfaces.take(uni);
        interfaceListModel->removeItem(item);
        updateHasWireless();
        updateHasWwan();
    }
}

void DeclarativeNMPopup::manageUpdateTraffic(DeclarativeInterfaceItem *device)
{
    kDebug() << "handle traffic plotter changes";
    if (rootObject()) {
        m_interfaceDetails->setInterface(device->interface());
        m_interfaceDetails->setUpdateEnabled(true);
        QMetaObject::invokeMethod(rootObject(), "showDetailsWidget");
        listModel->setDeviceToFilter(device->interface(), device->type() == "vpn");
    }
}

void DeclarativeNMPopup::managerWwanHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wwan enable switch state changed" << enabled;
    m_rootContext->setContextProperty("mobileVisible", enabled);
}

void DeclarativeNMPopup::addVpnInterface()
{
    m_vpnItem = new DeclarativeInterfaceItem(0, m_activatables, DeclarativeInterfaceItem::InterfaceName, this);
    interfaceListModel->appendRow(m_vpnItem);
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

void DeclarativeNMPopup::updateWireless(bool checked)
{
    if (NetworkManager::isWirelessEnabled() != checked) {
        NetworkManager::setWirelessEnabled(checked);
    }
}

DeclarativeInterfaceItem* DeclarativeNMPopup::vpnItem()
{
    return m_vpnItem;
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
    m_rootContext->setContextProperty("wirelessChecked", enabled);
    if (enabled) {
        m_rootContext->setContextProperty("wirelessEnabled", enabled);
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

    m_hasWirelessInterface = hasWireless;

    if (!m_hasWirelessInterface) {
        hasWireless = false;
    }
    m_rootContext->setContextProperty("wirelessVisible", hasWireless);
}


void DeclarativeNMPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_rootContext->setContextProperty("wirelessEnabled", enabled);
}

void DeclarativeNMPopup::managerNetworkingEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed networking enable state" << enabled;
    networkingEnabledToggled(enabled);
}

void DeclarativeNMPopup::networkingEnabledToggled(bool checked)
{
    if (checked) {
        m_rootContext->setContextProperty("warningLabel", QVariant(QString()));
    }

    // Switch networking on / off
    if (NetworkManager::isNetworkingEnabled() != checked) {
        NetworkManager::setNetworkingEnabled(checked);
    }
}

void DeclarativeNMPopup::readConfig()
{
    kDebug();
    KNetworkManagerServicePrefs::self()->readConfig();
    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));

    if (config.readEntry(QLatin1String("DetailKeys"), QStringList()).isEmpty()) {
        QStringList keys;
        keys << "interface:type"
             << "interface:status"
             << "ipv4:address"
             << "interface:bitrate"
             << "interface:name"
             << "interface:hardwareaddress"
             << "interface:driver"
             << "wireless:ssid"
             << "wireless:accesspoint"
             << "wireless:band"
             << "wireless:channel"
             << "mobile:operator"
             << "mobile:quality"
             << "mobile:technology";

        config.writeEntry(QLatin1String("DetailKeys"), keys);
        config.sync();
    }

    m_rootContext->setContextProperty("wirelessChecked", NetworkManager::isWirelessEnabled());
    m_rootContext->setContextProperty("wirelessEnabled", NetworkManager::isWirelessHardwareEnabled());

    m_rootContext->setContextProperty("mobileChecked", NetworkManager::isWwanEnabled());
    m_rootContext->setContextProperty("mobileEnabled", NetworkManager::isWwanHardwareEnabled());

    foreach(DeclarativeInterfaceItem * i, m_interfaces) {
        i->setNameDisplayMode(DeclarativeInterfaceItem::InterfaceName);
    }

    QString version = NetworkManager::version();
    if (version.isEmpty()) {
        m_rootContext->setContextProperty("warningLabel", QVariant(i18nc("Warning about wrong NetworkManager version", "NetworkManager is not running. Please start it.")));
    } else if (NetworkManager::compareVersion(QString(MINIMUM_NM_VERSION_REQUIRED)) < 0) {
        m_rootContext->setContextProperty("warningLabel", QVariant(i18nc("Warning about wrong NetworkManager version", "We need at least NetworkManager-%1 to work properly, found '%2'. Please upgrade to a newer version.", QString(MINIMUM_NM_VERSION_REQUIRED), version)));
    } else {
        m_rootContext->setContextProperty("warningLabel", QVariant(QString()));
    }
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
            ifaceItem = new DeclarativeInterfaceItem(iface, m_activatables, DeclarativeInterfaceItem::InterfaceName, this);
            kDebug() << "WiFi added";
        } else {
            // Create the interfaceitem
            kDebug() << "Interface Item added";
            ifaceItem = new DeclarativeInterfaceItem(iface, m_activatables, DeclarativeInterfaceItem::InterfaceName, this);
        }
        m_interfaces.insert(iface->uni(), ifaceItem);
        interfaceListModel->appendRow(ifaceItem);
    }
    updateHasWireless();
    updateHasWwan();
}

void DeclarativeNMPopup::updateHasWwan()
{
    bool hasWwan = false;
    foreach(DeclarativeInterfaceItem * ifaceitem, m_interfaces) {
        NetworkManager::ModemDevice* iface = qobject_cast<NetworkManager::ModemDevice *>(ifaceitem->interface());
        if (iface) {
            hasWwan = true;
            break;
        }
    }
    m_rootContext->setContextProperty("mobileVisible", hasWwan);
}

void DeclarativeNMPopup::showInterfaceDetails(const QString & uni)
{
    DeclarativeInterfaceItem * ifaceItem = m_interfaces.value(uni, 0);
    if (!ifaceItem) {
        return;
    }
    manageUpdateTraffic(ifaceItem);
}
