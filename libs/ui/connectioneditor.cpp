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
#include "knmserviceprefs.h"

ConnectionEditor::ConnectionEditor(QObject * parent) : QObject(parent)
{

}

ConnectionEditor::~ConnectionEditor()
{
}

Knm::Connection *ConnectionEditor::editConnection(Knm::Connection::Type type, const QVariantList &args)
{
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    KDialog configDialog(parentWidget);
    configDialog.setCaption(i18nc("Edit connection dialog caption", "Edit Network Connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));
    configDialog.setWindowModality(Qt::WindowModal);

    ConnectionPreferences * cprefs = editorForConnectionType(false, &configDialog, type, args);
    connect(cprefs, SIGNAL(valid(bool)), &configDialog, SLOT(enableButtonOk(bool)));
    configDialog.setMainWidget(cprefs);

    cprefs->load();
    cprefs->validate();

    if ( cprefs && configDialog.exec() == QDialog::Accepted ) {
        cprefs->save();
        return cprefs->connection();
    }
    return 0;
}

Knm::Connection *ConnectionEditor::editConnection(Knm::Connection *con)
{
    Knm::Connection *cprefsCon = NULL;
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    KDialog configDialog(parentWidget);
    configDialog.setCaption(i18nc("Edit connection dialog caption", "Edit Network Connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));
    configDialog.setWindowModality(Qt::WindowModal);

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

    return 0;
}


Knm::Connection *ConnectionEditor::createConnection(bool useDefaults, Knm::Connection::Type type, const QVariantList &otherArgs, const bool autoAccept)
{
    Knm::Connection *cprefsCon = NULL;
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    KDialog configDialog(parentWidget);
    configDialog.setCaption(i18nc("Add connection dialog caption", "Add Network Connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));
    configDialog.setWindowModality(Qt::WindowModal);

    QVariantList args;

    QString connectionId = QUuid::createUuid().toString();

    args << connectionId;
    args += otherArgs;
    ConnectionPreferences * cprefs = editorForConnectionType(useDefaults, &configDialog, type, args);

    if (!cprefs) {
        return 0;
    }

    connect(cprefs, SIGNAL(valid(bool)), &configDialog, SLOT(enableButtonOk(bool)));
    cprefs->load();
    cprefs->validate();

    configDialog.setMainWidget(cprefs);

    if ( autoAccept || configDialog.exec() == QDialog::Accepted ) {
        // update the connection from the UI
        cprefs->save();

        cprefsCon = cprefs->connection();

        kDebug() << "Add dialog accepted: Connection name: " << cprefsCon->name() << "type: " << cprefsCon->typeAsString(cprefsCon->type()) << "uuid: " << cprefsCon->uuid().toString() << "iconname: "<< cprefsCon->iconName();

        foreach(Knm::Setting *set, cprefsCon->settings())
            kDebug() << "    Setting name: " << set->name();

        return cprefs->connection();
    }
    return 0;
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

// vim: sw=4 sts=4 et tw=100
