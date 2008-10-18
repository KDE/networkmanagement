/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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
#include <QGraphicsGridLayout>
#include <QGraphicsLinearLayout>
#include <QLabel>
#include <QSignalMapper>

#include <KDebug>
#include <KLocale>
#include <KPushButton>
#include <KToolInvocation>

#include <Plasma/Label>
#include <Plasma/PushButton>

#include <solid/control/networkmanager.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

#include "../libs/marshalarguments.h"
#include "remoteconnection.h"
#include "networkmanagersettings.h"

NetworkManagerPopup::NetworkManagerPopup(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
    m_connectionActivationSignalMapper(new QSignalMapper(this)),
    m_connectionDeactivationSignalMapper(new QSignalMapper(this))
{
    qDBusRegisterMetaType<QMap<QString, QVariant> >();
    qDBusRegisterMetaType<QMap<QString, QMap<QString, QVariant> > >();

    // containing vertical linear layout
    m_layout = new QGraphicsLinearLayout(Qt::Vertical);
    //   a vertical list of appropriate connections
    //     header label
    m_connectionLayout = new QGraphicsLinearLayout(Qt::Vertical, m_layout);
    Plasma::Label * header = new Plasma::Label(this);
    header->setText(i18nc("Label for connection list popup","<b>Network Connections</b>"));
    m_connectionLayout->addItem(header);
    m_userSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_USER_SETTINGS), this);
    m_systemSettings = new NetworkManagerSettings(QLatin1String(NM_DBUS_SERVICE_SYSTEM_SETTINGS), this);
    populateConnectionList(m_userSettings);
    populateConnectionList(m_systemSettings);
    m_layout->addItem(m_connectionLayout);

    //   then a grid of status labels and buttons
    // +----------------------------+
    // |              | [Manage...] |
    // | Wireless hw switch status  |
    // | [Networking] | [Wireless]  |
    // +----------------------------|
    QGraphicsGridLayout * gridLayout = new QGraphicsGridLayout(m_layout);
    m_btnManageConnections = new Plasma::PushButton(this);
    m_btnManageConnections->setText(i18nc("Button text for showing the Manage Connections KCModule", "Manage..."));
    gridLayout->addItem(m_btnManageConnections, 0, 1, 1, 1);
    //m_lblRfkill = new Plasma::Label(this);
    //m_lblRfkill->nativeWidget()->setWordWrap(false);
    //sets the label text
    //managerWirelessHardwareEnabledChanged(Solid::Control::NetworkManager::isWirelessHardwareEnabled());

    //gridLayout->addItem(m_lblRfkill, 1, 0, 1, 2);
    m_btnEnableNetworking = new Plasma::PushButton(this);
    m_btnEnableWireless = new Plasma::PushButton(this);
    m_btnEnableNetworking->nativeWidget()->setCheckable(true);
    m_btnEnableWireless->nativeWidget()->setCheckable(true);
    managerWirelessEnabledChanged(Solid::Control::NetworkManager::isWirelessEnabled());
    m_btnEnableNetworking->setText(i18nc("Label for pushbutton enabling networking", "Networking"));
    m_btnEnableWireless->setText(i18nc("Label for pushbutton enabling wireless", "Wireless"));
    gridLayout->addItem(m_btnEnableNetworking, 2, 0, 1, 2);
    gridLayout->addItem(m_btnEnableWireless, 3, 0, 1, 2);
    m_layout->addItem(gridLayout);
    setLayout(m_layout);
    // connect up the buttons and the manager's signals
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            this, SLOT(networkInterfaceAdded(const QString&)));
    QObject::connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            this, SLOT(networkInterfaceRemoved(const QString&)));
    QObject::connect(m_btnManageConnections, SIGNAL(clicked()),
            this, SLOT(manageConnections()));
    QObject::connect(m_btnEnableNetworking->nativeWidget(), SIGNAL(toggled(bool)),
            this, SLOT(userNetworkingEnabledChanged(bool)));
    QObject::connect(m_btnEnableWireless->nativeWidget(), SIGNAL(toggled(bool)),
            this, SLOT(userWirelessEnabledChanged(bool)));
    QObject::connect(m_connectionActivationSignalMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(activateConnection(const QString&)));
    QObject::connect(m_connectionDeactivationSignalMapper, SIGNAL(mapped(const QString&)),
            this, SLOT(deactivateConnection(const QString&)));
}

NetworkManagerPopup::~NetworkManagerPopup()
{
    delete m_layout;
}

void NetworkManagerPopup::populateConnectionList(NetworkManagerSettings * service)
{
    foreach (QString connectionPath, service->connections() ) {
        RemoteConnection * connection = service->findConnection(connectionPath);
        QVariantMapMap settings = connection->GetSettings();
        QVariantMap connectionSetting = settings.value("connection");
        if (connectionIsAppropriate(settings)) {
            Plasma::PushButton * connectionButton = new Plasma::PushButton(this);
            m_connectionActivationSignalMapper->setMapping(connectionButton, service->service() + QChar('%') + connectionPath);
            QObject::connect(connectionButton, SIGNAL(clicked()), m_connectionActivationSignalMapper, SLOT(map()));
            connectionButton->setText(connectionSetting.value("id").toString());
            m_connectionLayout->addItem(connectionButton);
        }
    }
}

class ConnectionInspector
{
public:
    virtual ~ConnectionInspector() {
    }

    virtual bool accept(const QVariantMapMap& connection) = 0;
    QString connectionType(const QVariantMapMap& connection)
    {
        QString type;
        if ( connection.contains(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME))) {
            QVariantMap connectionSetting = connection.value(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
            if (connectionSetting.contains(QLatin1String(NM_SETTING_CONNECTION_TYPE))) {
                type = connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_TYPE)).toString();
            }
        }
        return type;
    }
};

class WiredConnectionInspector : public ConnectionInspector
{
public:
    WiredConnectionInspector(Solid::Control::WiredNetworkInterface* iface) : m_iface(iface) { }
    bool accept(const QVariantMapMap& connection)
    {
        return (connectionType(connection) == QLatin1String(NM_SETTING_WIRED_SETTING_NAME) && m_iface->carrier());
    }
private:
    Solid::Control::WiredNetworkInterface *m_iface;
};

class WirelessConnectionInspector : public ConnectionInspector
{
public:
    WirelessConnectionInspector(Solid::Control::WirelessNetworkInterface* iface) : m_iface(iface) { }
    bool accept(const QVariantMapMap& connection)
    {
        bool acceptable = false;
        if (connectionType(connection) == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
            // check if the essid in the connection matches one of the access points returned by NM
            // on this device.
            // If an AP is hiding the essid, but one of the Settings services provides a connection
            // with this essid, NM will add the essid to the AP object, so we can use this technique
            // even for hidden networks

            QString ssid;
            if ( connection.contains(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME))) {
                QVariantMap connectionSetting = connection.value(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME));
                if (connectionSetting.contains(QLatin1String(NM_SETTING_WIRELESS_SSID))) {
                    ssid = connectionSetting.value(QLatin1String(NM_SETTING_WIRELESS_SSID)).toString();
                    foreach (QString accessPointUni, m_iface->accessPoints()) {
                        Solid::Control::AccessPoint * ap = m_iface->findAccessPoint(accessPointUni);
                        if (ap->ssid() == ssid) {
                            acceptable = true;
                        }
                    }
                }
            }
        }
        return acceptable;
    }
private:
    Solid::Control::WirelessNetworkInterface *m_iface;
};

class GsmConnectionInspector : public ConnectionInspector
{
public:
    bool accept(const QVariantMapMap& connection)
    {
        return connectionType(connection) == QLatin1String(NM_SETTING_GSM_SETTING_NAME);
    }
};

class CdmaConnectionInspector : public ConnectionInspector
{
public:
    bool accept(const QVariantMapMap& connection)
    {
        return connectionType(connection) == QLatin1String(NM_SETTING_CDMA_SETTING_NAME);
    }
};

class PppoeConnectionInspector : public ConnectionInspector
{
public:
    bool accept(const QVariantMapMap& connection)
    {
        return connectionType(connection) == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME);
    }
};

class ConnectionInspectorFactory
{
public:
    ConnectionInspector *connectionInspector(Solid::Control::NetworkInterface* iface)
    {
        ConnectionInspector * inspector = 0;
        if (!m_inspectors.contains(iface)) {
            switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee8023:
                inspector = new WiredConnectionInspector(static_cast<Solid::Control::WiredNetworkInterface*>(iface));
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
                inspector = new WirelessConnectionInspector(static_cast<Solid::Control::WirelessNetworkInterface*>(iface));
                break;
            case Solid::Control::NetworkInterface::Serial:
                inspector = new PppoeConnectionInspector;
                break;
            case Solid::Control::NetworkInterface::Gsm:
                inspector = new GsmConnectionInspector;
                break;
            case Solid::Control::NetworkInterface::Cdma:
                inspector = new CdmaConnectionInspector;
                break;
            default:
                kDebug() << "Unhandled network interface type : " << iface->type();
            }
            m_inspectors.insert(iface, inspector);
        } else {
            inspector = m_inspectors.value(iface);
        }
        Q_ASSERT(inspector);
        return inspector;
    }
private:
    QMap<Solid::Control::NetworkInterface*, ConnectionInspector*> m_inspectors;
};

bool NetworkManagerPopup::connectionIsAppropriate(const QVariantMapMap& connection) const
{
    kDebug() << connection;
    bool acceptable = false;
    ConnectionInspectorFactory cif;
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        ConnectionInspector * inspector = cif.connectionInspector(iface);
        kDebug() << "Testing " << connection.value(NM_SETTING_CONNECTION_SETTING_NAME).value(NM_SETTING_CONNECTION_ID).toString() << " with iface " << iface->uni();

        if (inspector->accept(connection)) {
            kDebug() << "  accepted";
            acceptable = true;
            break;
        }
    }
    if (!acceptable) {
        kDebug() << "No network interface accepted " << connection.value(NM_SETTING_CONNECTION_SETTING_NAME).value(NM_SETTING_CONNECTION_ID).toString();
    }
    return acceptable;
}

void NetworkManagerPopup::networkInterfaceAdded(const QString&)
{

}

void NetworkManagerPopup::networkInterfaceRemoved(const QString&)
{

}

void NetworkManagerPopup::accessPointAppeared(const QString &)
{

}

void NetworkManagerPopup::overallStatusChanged(Solid::Networking::Status)
{

}

void NetworkManagerPopup::managerWirelessEnabledChanged(bool enabled)
{
    m_btnEnableWireless->nativeWidget()->setChecked(enabled);
}

void NetworkManagerPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    if (enabled) {
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is enabled", "Wireless hardware is enabled"));
    } else {
        m_lblRfkill->setText(i18nc("Label text when hardware wireless is not enabled", "Wireless hardware is disabled"));
    }
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

void NetworkManagerPopup::manageConnections()
{
    kDebug() << "opening connection management dialog";
    QStringList args;
    args << "kcm_knetworkmanager";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}

void NetworkManagerPopup::activateConnection(const QString& connection)
{
    kDebug() << connection;
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
}

void NetworkManagerPopup::deactivateConnection(const QString& connection)
{
    kDebug() << connection;
}

QStringList NetworkManagerPopup::interfacesForConnection(OrgFreedesktopNetworkManagerSettingsConnectionInterface* connection) const
{
    QStringList matchingInterfaces;
    QVariantMapMap settings = connection->GetSettings();
    QVariantMap connectionSetting = settings.value(QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME));
    const Solid::Control::NetworkInterface::Type type =
        typeForConnection(connectionSetting.value(QLatin1String(NM_SETTING_CONNECTION_TYPE)).toString());
    foreach (Solid::Control::NetworkInterface * interface,
            Solid::Control::NetworkManager::networkInterfaces()) {
        if (interface->type() == type) {
            matchingInterfaces.append(interface->uni());
        }
    }
    return matchingInterfaces;
}

Solid::Control::NetworkInterface::Type NetworkManagerPopup::typeForConnection(const QString &connectionString) const
{
    kDebug() << "converting connection type string " << connectionString << " to Type";
    if (connectionString == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        return Solid::Control::NetworkInterface::Gsm;
    } else if (connectionString == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        return Solid::Control::NetworkInterface::Serial;
    } else if (connectionString == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        return Solid::Control::NetworkInterface::Cdma;
    } else if (connectionString == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        return Solid::Control::NetworkInterface::Ieee8023;
    } else if (connectionString == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        return Solid::Control::NetworkInterface::Ieee80211;
    } else {
        kWarning() << "Connection has unrecognised type string " << connectionString;
    }
    return Solid::Control::NetworkInterface::UnknownType;
}

#include "networkmanagerpopup.moc"

// vim: sw=4 sts=4 et tw=100
