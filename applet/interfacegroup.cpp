/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "interfacegroup.h"

#include <NetworkManager.h>
#include <nm-setting-connection.h>
#include <nm-setting-wireless.h>

#include <QGraphicsLinearLayout>
#include <KDebug>
#include <KNotification>
#include <KToolInvocation>

#include <solid/control/networkserialinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>

#include "events.h"
#include "interfaceitem.h"
#include "connectionitem.h"
#include "wirelessconnectionitem.h"
#include "connectioninspector.h"
#include "networkmanagersettings.h"
#include "remoteconnection.h"
#include "serialinterfaceitem.h"
#include "wiredinterfaceitem.h"
#include "wirelessinterfaceitem.h"
#include "wirelessenvironment.h"
#include "wirelessnetworkitem.h"
#include "hiddenwirelessnetworkitem.h"
#include "mergedwireless.h"

QDebug operator<<(QDebug s, const AbstractWirelessNetwork*wl )
{
    s.nospace() << "Wireless(" << qPrintable(wl->ssid() ) << "," << wl->strength() << ")";
    return s.space();
}

bool wirelessNetworkGreaterThanStrength(AbstractWirelessNetwork* n1, AbstractWirelessNetwork * n2);

InterfaceGroup::InterfaceGroup(Solid::Control::NetworkInterface::Type type,
                               NetworkManagerSettings * userSettings,
                               NetworkManagerSettings * systemSettings,
                               QGraphicsWidget * parent)
    : ConnectionList(userSettings, systemSettings, parent), m_type(type),
      m_wirelessEnvironment(new WirelessEnvironmentMerged(this)),
      m_interfaceLayout(new QGraphicsLinearLayout(Qt::Vertical)),
      m_networkLayout(new QGraphicsLinearLayout(Qt::Vertical)),
      m_numberOfWlans( 1 )
{
    if (m_type == Solid::Control::NetworkInterface::Ieee80211) {
        m_enabled = Solid::Control::NetworkManager::isWirelessEnabled();
    } else {
        m_enabled = Solid::Control::NetworkManager::isNetworkingEnabled();
    }
    connect(m_wirelessEnvironment, SIGNAL(networkAppeared(const QString&)), SLOT(refreshConnectionsAndNetworks()));
    connect(m_wirelessEnvironment, SIGNAL(networkDisappeared(const QString&)), SLOT(refreshConnectionsAndNetworks()));
    connect(this, SIGNAL(connectionListUpdated()), SLOT(refreshConnectionsAndNetworks()));

    m_layout->setSpacing(0);
    m_interfaceLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_interfaceLayout->setSpacing(4);
    m_networkLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_networkLayout->setSpacing(0);
    kDebug() << "TYPE" << m_type;

    if (m_type == Solid::Control::NetworkInterface::Gsm) {
        setMinimumSize(QSize(285, 60)); // WTF?
    }
}

InterfaceGroup::~InterfaceGroup()
{
    qDeleteAll( m_interfaces );
}

void InterfaceGroup::setEnabled(bool enable)
{
    m_enabled = enable;
    foreach (InterfaceItem * item, m_interfaces) {
        item->setEnabled(enable);
    }
    updateNetworks();
}

void InterfaceGroup::setupHeader()
{
    m_layout->insertItem(0, m_interfaceLayout);
    // create an interfaceItem for each interface of our type
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        if (iface->type() == interfaceType()) {
            addInterfaceInternal(iface);
            //kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
        }
    }
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(interfaceAdded(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(interfaceRemoved(const QString&)));

    if (m_interfaces.count() == 0) {
        hide();
    }
    setLayout(m_layout);
}

void InterfaceGroup::setupFooter()
{
    m_layout->addItem(m_networkLayout);
    if (m_type == Solid::Control::NetworkInterface::Ieee80211) {
        m_hiddenItem = new HiddenWirelessNetworkItem(this);
        m_hiddenItem->setupItem();
        m_layout->addItem(m_hiddenItem);

        connect(m_hiddenItem, SIGNAL(clicked(AbstractConnectableItem*)),
                SLOT(connectToWirelessNetwork(AbstractConnectableItem*)));
    }
    //kDebug() << m_interfaces.keys() << "Footer update";
    //kDebug() << m_interfaces.keys() << m_networks.keys() << "Footer .. CONNECT";
    connect(this, SIGNAL(connectionListUpdated()), SLOT(updateConnections()));
}

void InterfaceGroup::updateConnections()
{
    updateNetworks();
    QStringList watched = m_wirelessInspector->watchedNetworks();
    if (watched.isEmpty())
        return;

    ServiceConnectionHash::iterator i = m_connections.begin();
    while (i != m_connections.end()) {
        ConnectionItem * item = i.value();
        QVariantMapMap settings = item->connection()->settings();
        QString ssid = settings[ NM_SETTING_WIRELESS_SETTING_NAME ] [ NM_SETTING_WIRELESS_SSID ].toString();
        if (watched.contains(ssid))
        {
            activateConnection( item );
            m_wirelessInspector->removeWatchForNetwork(ssid);
            break;
        }
        ++i;
    }
}

void InterfaceGroup::updateNetworks()
{
    //kDebug();
    // empty the layout
    foreach (WirelessNetworkItem * i, m_networks) {
        m_networkLayout->removeItem(i);
        delete i;
    }
    m_networks.clear();

/*
    //FIXME: Qt 4.4 hack
    delete m_networkLayout;
    m_networkLayout = new QGraphicsLinearLayout(Qt::Vertical);
    m_layout->addItem(m_networkLayout);
    m_networkLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
*/
    if (m_enabled) {
        foreach (AbstractWirelessNetwork * i, networksToShow()) {
            addWirelessNetworkInternal(i->ssid());
        }
        //kDebug() << "Now ... " << m_networks.keys();
    } else {
        //kDebug() << "Interface disabled ................ :-(";
    }
    /*
    m_networkLayout->invalidate();
    m_interfaceLayout->invalidate();
    m_layout->invalidate();
    */
    m_networkLayout->updateGeometry();
    m_interfaceLayout->updateGeometry();
    m_layout->updateGeometry();

}

void InterfaceGroup::setNetworksLimit( int wlans )
{
    int old = m_numberOfWlans;
    m_numberOfWlans = wlans;
    if (old != m_numberOfWlans) {
        updateNetworks();
    }
}


QList<AbstractWirelessNetwork*> InterfaceGroup::networksToShow()
{
    QList<AbstractWirelessNetwork*> allNetworks;
    QList<AbstractWirelessNetwork*> topNetworks;

    //kDebug() << "m_conn empty?" << m_connections.isEmpty() << "m_userSettings" << m_userSettings->isValid();

    // check whether we have a connection for every ssid seen, if so, don't show it.
    foreach (const QString &ssid, m_wirelessEnvironment->networks()) {
        AbstractWirelessNetwork * net = m_wirelessEnvironment->findNetwork(ssid);
        // trying out excluding networks based on any existing connection
        bool connectionForNetworkExists = false;
        foreach (ConnectionItem * connectionItem, m_connections) {
            RemoteConnection * conn = connectionItem->connection();
            QVariantMapMap settings = conn->settings();
            // is it wireless?
            QVariantMap connectionSetting = settings.value(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
            if (connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_TYPE)).toString() == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
                QVariantMap wirelessSetting = settings.value(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME));
                // does this connection match the ssid?
                QString connectionSsid = wirelessSetting.value(QLatin1String(NM_SETTING_WIRELESS_SSID)).toString();
                connectionForNetworkExists |= (connectionSsid == ssid);
            }
        }
        if (!connectionForNetworkExists) {
            allNetworks.append(net);
        }
    }

    // sort by strength
    qSort(allNetworks.begin(), allNetworks.end(), wirelessNetworkGreaterThanStrength);

    //return the configured number of networks to show
    topNetworks = allNetworks.mid(0, m_numberOfWlans);
    return topNetworks;
}

void InterfaceGroup::addInterfaceInternal(Solid::Control::NetworkInterface* iface)
{
    Q_ASSERT(iface);
    if (!m_interfaces.contains(iface->uni())) {
        if (iface->type() != m_type) {
            return;
        }
        InterfaceItem * interface = 0;
        ConnectionInspector * inspector = 0;

        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee80211:
            {
                WirelessInterfaceItem * wirelessinterface = 0;
                wirelessinterface = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
                wirelessinterface->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());

                // keep track of rf kill changes
                QObject::disconnect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)), this, 0 );
                QObject::disconnect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)), this, 0 );
                QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
                        this, SLOT(setEnabled(bool)));
                QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
                        this, SLOT(setEnabled(bool)));

                m_wirelessEnvironment->addWirelessEnvironment(wirelessinterface->wirelessEnvironment());
                interface = wirelessinterface;
                inspector = m_wirelessInspector = new WirelessConnectionInspector(static_cast<Solid::Control::WirelessNetworkInterface*>(iface), wirelessinterface->wirelessEnvironment());
                kDebug() << "WiFi added";
                break;
            }
            case Solid::Control::NetworkInterface::Serial:
            {
                interface = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                        m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                inspector = new PppoeConnectionInspector;
                break;
            }
            case Solid::Control::NetworkInterface::Gsm:
            {
                interface = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                        m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                // TODO: When ModemManager support is added, connect signals from the SII to
                // reassesConnectionList
                inspector = new GsmConnectionInspector;
                break;
            }
            case Solid::Control::NetworkInterface::Cdma:
            {
                interface = new SerialInterfaceItem(static_cast<Solid::Control::SerialNetworkInterface *>(iface),
                        m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                inspector = new CdmaConnectionInspector;
                // TODO: When ModemManager support is added, connect signals from the SII to
                // reassesConnectionList
                break;
            }
            default:
            case Solid::Control::NetworkInterface::Ieee8023:
            {
                WiredInterfaceItem * wiredinterface = 0;
                interface = wiredinterface = new WiredInterfaceItem(static_cast<Solid::Control::WiredNetworkInterface *>(iface),
                        m_userSettings, m_systemSettings, InterfaceItem::InterfaceName, this);
                inspector = new WiredConnectionInspector(static_cast<Solid::Control::WiredNetworkInterface*>(iface));
                break;
            }
        }
        interface->setConnectionInspector(inspector);
        interface->setEnabled(m_enabled);
        m_interfaceLayout->addItem(interface);
        m_interfaces.insert(iface->uni(), interface);
        //m_interfaceLayout->invalidate();
        m_interfaceLayout->updateGeometry();
        updateNetworks();
    }
    show();
    emit updateLayout();
}

void InterfaceGroup::addWirelessNetworkInternal(const QString & ssid)
{
    //kDebug() << "Adding network:" << ssid << m_networks.keys();
    if (!m_networks.contains(ssid)) {
        AbstractWirelessNetwork * net = m_wirelessEnvironment->findNetwork(ssid);
        WirelessNetworkItem * netItem = new WirelessNetworkItem(net, this);
        netItem->setupItem();
        m_networkLayout->addItem(netItem);
        m_networks.insert(ssid, netItem);
        connect(netItem, SIGNAL(clicked(AbstractConnectableItem*)),
                SLOT(connectToWirelessNetwork(AbstractConnectableItem*)));
    }
}

bool InterfaceGroup::accept(RemoteConnection * conn) const
{
    // there was some code I don't remember the purpose of in addConnectionInternal before it was refactored here
    // that cast to WirelessConnectionItem, then did the same interface accept() loop.  ...?
    bool accepted = false;
    foreach (InterfaceItem * iface, m_interfaces) {
        //kDebug() << conn << iface->connectionInspector()->accept(conn);
        if (iface->connectionInspector()->accept(conn)) {
            accepted = true;
            break;
        }
    }
    return accepted;
}

ConnectionItem * InterfaceGroup::createItem(RemoteConnection* connection)
{
    ConnectionItem * ci = 0;
    if (m_type == Solid::Control::NetworkInterface::Ieee80211) {
        // get connection ssid.  In theory we know that the connection _has_ an ssid because the
        // WirelessConnectionInspector accepted it.
        QVariantMapMap settings = connection->settings();
        if ( settings.contains(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME))) {
            QVariantMap connectionSetting = settings.value(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME));
            if (connectionSetting.contains(QLatin1String(NM_SETTING_WIRELESS_SSID))) {
                QString ssid = connectionSetting.value(QLatin1String(NM_SETTING_WIRELESS_SSID)).toString();
                WirelessConnectionItem * wi = new WirelessConnectionItem(connection, this);
                ci = wi;
                wi->setNetwork(m_wirelessEnvironment->findNetwork(ssid));
            }
        }
    } else {
        ci = new ConnectionItem(connection, this);
    }
    Q_ASSERT(ci);
    ci->setupItem();

    return ci;
}

Solid::Control::NetworkInterface::Type InterfaceGroup::interfaceType() const
{
    return m_type;
}

void InterfaceGroup::interfaceAdded(const QString& uni)
{
    if (m_interfaces.keys().contains(uni)) {
        return;
    }
    kDebug() << "Interface Added.";
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
    addInterfaceInternal(iface);
    KNotification::event(Event::HwAdded, i18nc("Notification for hardware added", "Network interface %1 attached", iface->interfaceName()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    updateNetworks();
    emit updateLayout();
}

void InterfaceGroup::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        InterfaceItem * item = m_interfaces.take(uni);
        m_interfaceLayout->removeItem(item);
        KNotification::event(Event::HwRemoved, i18nc("Notification for hardware removed", "Network interface removed"), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
        delete item;
        reassess();
    }
    emit updateLayout();
}

void InterfaceGroup::refreshConnectionsAndNetworks()
{
    //kDebug() << "Refreshing";
    updateNetworks();
    reassess();
}

void InterfaceGroup::activateConnection(AbstractConnectableItem* item)
{
    // tell the manager to activate the connection
    // which device??
    // HACK - take the first one
    ConnectionItem * ci = qobject_cast<ConnectionItem*>(item);
    QHash<QString, InterfaceItem *>::const_iterator i = m_interfaces.constBegin();
    if ( i != m_interfaces.constEnd()) {
        QString firstDeviceUni = i.key();
        Solid::Control::NetworkManager::activateConnection(firstDeviceUni, ci->connection()->service() + " " + ci->connection()->path(), QVariantMap());
        KNotification::event(Event::Connecting, i18nc("Notification text when activating a connection","Connecting %1", ci->connection()->id()), QPixmap(), 0, KNotification::CloseOnTimeout, KComponentData("networkmanagement", "networkmanagement", KComponentData::SkipMainComponentRegistration));
    }
    // if the manager updates the interface's state, we should then refresh the list of
    // connections(remove any active connections from the list
    // What about a connection that could be activated on 2 devices?
    // Ideally we should keep them around
    updateNetworks();
}

void InterfaceGroup::connectToWirelessNetwork(AbstractConnectableItem* item)
{
    AbstractWirelessNetworkItem * wni = qobject_cast<AbstractWirelessNetworkItem*>(item);

    if (wni && wni->net() ) {
        int caps = 0, wpaFlags = 0, rsnFlags = 0;
        if ( wni->net()->referenceAccessPoint()) {
            caps = wni->net()->referenceAccessPoint()->capabilities();
            wpaFlags = wni->net()->referenceAccessPoint()->wpaFlags();
            rsnFlags = wni->net()->referenceAccessPoint()->rsnFlags();
        }
        //kDebug() << wni->net()->referenceAccessPoint()->hardwareAddress();
        QDBusInterface kcm(QLatin1String("org.kde.NetworkManager.KCModule"), QLatin1String("/default"), QLatin1String("org.kde.kcmshell.ConnectionEditor"));
        if (kcm.isValid()) {
            kDebug() << "opening connection management dialog from running KCM";
            QVariantList args;

            args << wni->net()->ssid() << caps << wpaFlags << rsnFlags;
            m_wirelessInspector->watchForNetworkConnection(wni->net()->ssid());
            kcm.call(QDBus::NoBlock, "createConnection", "802-11-wireless", QVariant::fromValue(args));
        } else {
            kDebug() << "opening connection management dialog using networkmanagement_configshell";
            QStringList args;
            QString moduleArgs =
                QString::fromLatin1("'%1' %2 %3 %4")
                .arg(wni->net()->ssid().replace('\'', "\\'"))
                .arg(caps)
                .arg(wpaFlags)
                .arg(rsnFlags);

            args << QLatin1String("create") << QLatin1String("--type") << QLatin1String("802-11-wireless") << QLatin1String("--specific-args") << moduleArgs << QLatin1String("wifi_pass");
            m_wirelessInspector->watchForNetworkConnection(wni->net()->ssid());
            int ret = KToolInvocation::kdeinitExec("networkmanagement_configshell", args);
            kDebug() << ret << args;
        }
    } else {
        kDebug() << "item was not an AbstractWirelessNetworkItem!";
    }
}

bool wirelessNetworkGreaterThanStrength(AbstractWirelessNetwork* n1, AbstractWirelessNetwork * n2)
{
    return n1->strength() > n2->strength();
}

void InterfaceGroup::popupEvent(bool show)
{
    kDebug() << show;
    m_hiddenItem->resetSsidEntry();
}
// vim: sw=4 sts=4 et tw=100
