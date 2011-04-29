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

#include "connectioneditor.h"

//#include <nm-setting-cdma.h>
//#include <nm-setting-gsm.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>

#include <QDBusInterface>
#include <QDir>
// debug only
#include <QFile>

#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KSharedConfig>
#include <KLocale>
#include <KPushButton>

#include "connectionprefs.h"
#include "wiredpreferences.h"
#include "wirelesspreferences.h"
#include "gsmconnectioneditor.h"
#include "cdmaconnectioneditor.h"
#include "bluetoothconnectioneditor.h"
#include "pppoepreferences.h"
#include "vpnpreferences.h"

//storage
#include "connection.h"
#include "connectionpersistence.h"
#include "knmserviceprefs.h"

ConnectionEditor::ConnectionEditor(QObject * parent) : QObject(parent)
{

}

ConnectionEditor::~ConnectionEditor()
{
}

void ConnectionEditor::editConnection(Knm::Connection::Type type, const QVariantList &args)
{
    KDialog configDialog(0);
    configDialog.setCaption(i18nc("Edit connection dialog caption", "Edit Network Connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));

    ConnectionPreferences * cprefs = editorForConnectionType(false, &configDialog, type, args);
    connect(cprefs, SIGNAL(valid(bool)), &configDialog, SLOT(enableButtonOk(bool)));
    configDialog.setMainWidget(cprefs);

    cprefs->load();
    cprefs->validate();

    // For KAuth (KDE's Polkit wrapper): this seems to have no effect in the Ok button though.
    configDialog.button(KDialog::Ok)->setAuthAction(QLatin1String("org.freedesktop.network-manager-settings.system.modify"));

    if ( cprefs && configDialog.exec() == QDialog::Accepted ) {
        QStringList changedConnections;
        changedConnections << cprefs->connection()->uuid();
        cprefs->save();
        persist(cprefs->connection());
        updateService(changedConnections);
        emit connectionsChanged();
    }
}

Knm::Connection *ConnectionEditor::editConnection(Knm::Connection *con)
{

    Knm::Connection *cprefsCon = NULL;
    KDialog configDialog(0);
    configDialog.setCaption(i18nc("Edit connection dialog caption", "Edit Network Connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));

    ConnectionPreferences * cprefs = editorForConnectionType(false, &configDialog, con);
    connect(cprefs, SIGNAL(valid(bool)), &configDialog, SLOT(enableButtonOk(bool)));
    configDialog.setMainWidget(cprefs);

    cprefs->load();
    cprefs->validate();

    if ( cprefs && configDialog.exec() == QDialog::Accepted ) {
        cprefs->save();
        cprefsCon = cprefs->connection();

        kDebug() << "Edit dialog accepted: Connection name: " << cprefsCon->name() << "type: " << cprefsCon->typeAsString(cprefsCon->type()) << "uuid: " << cprefsCon->uuid().toString() << "iconname: "<< cprefsCon->iconName();

        foreach(Knm::Setting *set, cprefsCon->settings())
            kDebug() << "    Setting name: " << set->name();

        return cprefs->connection();
    }

    return NULL;
}


Knm::Connection *ConnectionEditor::createConnection(bool useDefaults, Knm::Connection::Type type, const QVariantList &otherArgs, const bool autoAccept)
{
    Knm::Connection *cprefsCon = NULL;
    KDialog configDialog(0);
    configDialog.setCaption(i18nc("Add connection dialog caption", "Add Network Connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));

    QVariantList args;

    QString connectionId = QUuid::createUuid().toString();

#if 0
    // Check if there is already a connection with this uuid, if so, pick a new uuid
    // If 50 runs have passed, something is really fishy as we only get existing uuid
    // try at max
    // We check the newly created uuid against the list of configured connections
    QString connectionPath(KStandardDirs::locateLocal("data",
                Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH));
    const QStringList connectionFiles = QDir(connectionPath).entryList();

    // try really hard to get a unique id
    for (int i = 0; i < 999; i++) {
        if (connectionFiles.contains(connectionId)) {
            //kDebug() << i << "Woops, duplicate connnection ID" << connectionId << ". Creating another one.";
            connectionId = QUuid::createUuid().toString();
            if (i > 50) {
                kWarning() << "We have " << i << " times gotten existing uuids, QUuid QUuid::createUuid() something is fishy here: " << connectionId;
            }
        } else {
            kDebug() << "found new uuid which is not used yet:" << connectionId;
            break; // stop searching
        }
    }
    // Let's hope the connection ID is unique now...
#endif

    args << connectionId;
    args += otherArgs;
    ConnectionPreferences * cprefs = editorForConnectionType(useDefaults, &configDialog, type, args);

    if (!cprefs) {
        return NULL;
    }

    connect(cprefs, SIGNAL(valid(bool)), &configDialog, SLOT(enableButtonOk(bool)));
    cprefs->load();
    cprefs->validate();

    configDialog.setMainWidget(cprefs);

    if ( autoAccept || configDialog.exec() == QDialog::Accepted ) {
        // update the connection from the UI and save it to a file in appdata/connections
        cprefs->save();

        // update our rcfile (Must happen after cprefs->save())
        //persist(cprefs->connection());
        //updateService();
        //emit connectionsChanged();
        //
        cprefsCon = cprefs->connection();

        kDebug() << "Add dialog accepted: Connection name: " << cprefsCon->name() << "type: " << cprefsCon->typeAsString(cprefsCon->type()) << "uuid: " << cprefsCon->uuid().toString() << "iconname: "<< cprefsCon->iconName();

        foreach(Knm::Setting *set, cprefsCon->settings())
            kDebug() << "    Setting name: " << set->name();

        return cprefs->connection();
    }
    return NULL;
}

void ConnectionEditor::persist(Knm::Connection* connection)
{
    // add to the service prefs
    QString name = connection->name();
    QString type = Knm::Connection::typeAsString(connection->type());
    KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
    KConfigGroup config(prefs->config(), QLatin1String("Connection_") + connection->uuid());
    QStringList connectionIds = prefs->connections();
    // check if already present, we may be editing an existing Connection
    if (!connectionIds.contains(connection->uuid()))
    {
        connectionIds << connection->uuid();
        prefs->setConnections(connectionIds);
    }
    config.writeEntry("Name", name);
    config.writeEntry("Type", type);
    prefs->writeConfig();
}

ConnectionPreferences * ConnectionEditor::editorForConnectionType(bool setDefaults, QWidget * parent,
                                                                  Knm::Connection::Type type,
                                                                  const QVariantList & args) const
{
    kDebug() << args;
    ConnectionPreferences * wid = 0;
    switch (type) {
        case Knm::Connection::Wired:
            wid = new WiredPreferences(args, parent);
            break;
        case Knm::Connection::Wireless:
            wid = new WirelessPreferences(setDefaults, args, parent);
            break;
        case Knm::Connection::Cdma:
            wid = new CdmaConnectionEditor(args, parent);
            break;
        case Knm::Connection::Gsm:
            wid = new GsmConnectionEditor(args, parent);
            break;
        case Knm::Connection::Bluetooth:
            wid = new BluetoothConnectionEditor(args, parent);
            break;
        case Knm::Connection::Vpn:
            wid = new VpnPreferences(args, parent);
            break;
        case Knm::Connection::Pppoe:
            wid = new PppoePreferences(args, parent);
            break;
        default:
            break;
    }
    return wid;
}

ConnectionPreferences * ConnectionEditor::editorForConnectionType(bool setDefaults, QWidget * parent,
                                                                  Knm::Connection *con) const
{
    Q_UNUSED(setDefaults);
    ConnectionPreferences * wid = 0;
    switch (con->type()) {
        case Knm::Connection::Wired:
            wid = new WiredPreferences(con, parent);
            break;
        case Knm::Connection::Wireless:
            wid = new WirelessPreferences(con, parent);
            break;
        case Knm::Connection::Cdma:
            wid = new CdmaConnectionEditor(con, parent);
            break;
        case Knm::Connection::Gsm:
            wid = new GsmConnectionEditor(con, parent);
            break;
        case Knm::Connection::Bluetooth:
            wid = new BluetoothConnectionEditor(con, parent);
            break;
        case Knm::Connection::Vpn:
            wid = new VpnPreferences(con, parent);
            break;
        case Knm::Connection::Pppoe:
            wid = new PppoePreferences(con, parent);
            break;
        default:
            break;
    }
    return wid;
}

void ConnectionEditor::updateService(const QStringList & changedConnections) const
{
    kDebug() << changedConnections;
    QDBusInterface iface(QLatin1String("org.kde.networkmanagement"),
            QLatin1String("/connections"),
            QLatin1String("org.kde.networkmanagement"), QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        kError() << "KDED Module is not running!";
    }
    iface.call(QLatin1String("configure"), changedConnections);
}


// vim: sw=4 sts=4 et tw=100
