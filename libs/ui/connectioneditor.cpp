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
// debug only
#include <QFile>

#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KSharedConfig>
#include <KStandardDirs>
#include <KSharedConfig>
#include <KLocale>

#include "connectionprefs.h"
#include "wiredpreferences.h"
#include "wirelesspreferences.h"
#include "gsmconnectioneditor.h"
#include "cdmaconnectioneditor.h"
//#include "pppoepreferences.h"
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
    configDialog.setMainWidget(cprefs);

    if ( configDialog.exec() == QDialog::Accepted ) {
        QStringList changedConnections;
        changedConnections << cprefs->connection()->uuid();
        cprefs->save();
        persist(cprefs->connection());
        updateService(changedConnections);
        emit connectionsChanged();
    }
}

QString ConnectionEditor::addConnection(bool useDefaults, Knm::Connection::Type type, const QVariantList &otherArgs)
{
    KDialog configDialog(0);
    configDialog.setCaption(i18nc("Add connection dialog caption", "Add Network Connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));

    QVariantList args;
    QString connectionId = QUuid::createUuid().toString();
    args << connectionId;
    args += otherArgs;
    ConnectionPreferences * cprefs = editorForConnectionType(useDefaults, &configDialog, type, args);

    if (!cprefs) {
        return QString::null;
    }

    configDialog.setMainWidget(cprefs);

    if ( !cprefs->needsEdits() || configDialog.exec() == QDialog::Accepted ) {
        // update the connection from the UI and save it to a file in appdata/connections
        cprefs->save();
        // update our rcfile (Must happen after cprefs->save())
        persist(cprefs->connection());
        updateService();
        emit connectionsChanged();
    }
    return connectionId;
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
            wid = new WiredPreferences(parent, args);
            break;
        case Knm::Connection::Wireless:
            wid = new WirelessPreferences(setDefaults, parent, args);
            break;
        case Knm::Connection::Cdma:
            wid = new CdmaConnectionEditor(parent, args);
            break;
        case Knm::Connection::Gsm:
            wid = new GsmConnectionEditor(parent, args);
            break;
        case Knm::Connection::Vpn:
            wid = new VpnPreferences(parent, args);
            break;
        case Knm::Connection::Pppoe:
            //wid = new PppoePreferences(parent, args);
            break;
        default:
            break;
    }
    return wid;
}

void ConnectionEditor::updateService(const QStringList & changedConnections) const
{
    kDebug() << changedConnections;
    QDBusInterface iface(QLatin1String("org.kde.kded"),
            QLatin1String("/modules/knetworkmanager"),
            QLatin1String("org.kde.knetworkmanagerd"), QDBusConnection::sessionBus());
    if (!iface.isValid()) {
        kError() << "KDED Module is not running!";
    }
    iface.call(QLatin1String("configure"), changedConnections);
}


// vim: sw=4 sts=4 et tw=100
