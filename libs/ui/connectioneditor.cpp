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

#include <nm-setting-cdma.h>
#include <nm-setting-gsm.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wired.h>
#include <nm-setting-wireless.h>

#include <QDBusInterface>
#include <QUuid>

#include <KDebug>
#include <KIcon>
#include <KDialog>
#include <KLocale>

#include "connectionprefs.h"
#include "wiredpreferences.h"
#include "wirelesspreferences.h"
#include "cellularpreferences.h"
#include "pppoepreferences.h"
#include "vpnpreferences.h"

//storage
#include "knmserviceprefs.h"

ConnectionEditor::ConnectionEditor(QObject * parent) : QObject(parent)
{

}

ConnectionEditor::~ConnectionEditor()
{
}

QString ConnectionEditor::addConnection(ConnectionEditor::ConnectionType connectionType, const QVariantList &otherArgs)
{
    KDialog configDialog(0);
    configDialog.setCaption(i18nc("Add connection dialog caption", "Add network connection"));
    configDialog.setWindowIcon(KIcon("networkmanager"));

    QVariantList args;
    QString connectionId = QUuid::createUuid().toString();
    args << connectionId;
    args += otherArgs;
    ConnectionPreferences * cprefs = editorForConnectionType(true, &configDialog, connectionType, args);

    if (!cprefs) {
        return QString::null;
    }

    configDialog.setMainWidget(cprefs);

    kDebug() << cprefs->needsEdits();
    if ( !cprefs->needsEdits() || configDialog.exec() == QDialog::Accepted ) {
        cprefs->save();
        // add to the service prefs
        QString name = cprefs->connectionName();
        QString type = cprefs->connectionType();
        if (name.isEmpty() || type.isEmpty()) {
            kDebug() << "new connection has missing name (" << name << ") or type (" << type << ")";
            return QString::null;
        } else {
            KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
            KConfigGroup config(prefs->config(), QLatin1String("Connection_") + connectionId);
            QStringList connectionIds = prefs->connections();
            connectionIds << connectionId;
            prefs->setConnections(connectionIds);
            config.writeEntry("Name", cprefs->connectionName());
            config.writeEntry("Type", cprefs->connectionType());
            prefs->writeConfig();
            updateService();
            emit connectionsChanged();
        }
        return connectionId;
    }
    return QString::null;
}

ConnectionEditor::ConnectionType ConnectionEditor::connectionTypeForString(const QString &type) const
{
    ConnectionEditor::ConnectionType t = ConnectionEditor::Wireless;
    if (type == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        t = ConnectionEditor::Wired;
    } else if (type == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        t = ConnectionEditor::Wireless;
    } else if (type == QLatin1String(NM_SETTING_GSM_SETTING_NAME)
            || type == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        t = ConnectionEditor::Cellular;
    } else if (type == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        t = ConnectionEditor::Vpn;
    } else if (type == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        t = ConnectionEditor::Pppoe;
    }
    return t;
}

ConnectionPreferences * ConnectionEditor::editorForConnectionType(bool setDefaults, QWidget * parent,
                                                                  ConnectionEditor::ConnectionType type,
                                                                  const QVariantList & args) const
{
    kDebug() << args;
    ConnectionPreferences * wid = 0;
    switch (type) {
        case ConnectionEditor::Wired:
            wid = new WiredPreferences(parent, args);
            break;
        case ConnectionEditor::Wireless:
            wid = new WirelessPreferences(setDefaults, parent, args);
            break;
        case ConnectionEditor::Cellular:
            wid = new CellularPreferences(parent, args);
            break;
        case ConnectionEditor::Vpn:
            wid = new VpnPreferences(parent, args);
            break;
        case ConnectionEditor::Pppoe:
            wid = new PppoePreferences(parent, args);
            break;
        default:
            break;
    }
    return wid;
}

void ConnectionEditor::updateService(const QStringList & changedConnections) const
{
    QDBusInterface iface(QLatin1String("org.kde.knetworkmanagerd"),
            QLatin1String("/Configuration"),
            QLatin1String("org.kde.knetworkmanagerd"));
    iface.call(QLatin1String("configure"), changedConnections);
}


// vim: sw=4 sts=4 et tw=100
