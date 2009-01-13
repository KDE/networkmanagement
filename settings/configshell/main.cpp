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

#include <QDBusInterface>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>

#include <kicon.h>

#include "connectioneditor.h"
#include "knmserviceprefs.h"

int main(int argc, char **argv)
{
    KAboutData aboutData( "knetworkmanager_configshell", 0, ki18n("KNetworkManager Config Shell"),
                          0,
                          ki18n("Create network connections standalone"),
                          KAboutData::License_GPL,
                          ki18n("(c) 2008 Will Stephenson") );

    aboutData.addAuthor(ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("connection <connection-id>", ki18n("Connection ID to edit"));
    options.add("type <type>", ki18n("Connection type to create, must be one of '802-3-ethernet', '802-11-wireless', 'pppoe', 'vpn', 'cellular'"));
    options.add("specific-args <args>", ki18n("Space-separated connection type-specific arguments, may be either 'gsm' or 'cdma' for cellular, or 'openvpn' or 'vpnc' for vpn connections, and interface and AP identifiers for wireless connections"));
    options.add("+mode", ki18n("Operation mode, may be either 'create' or 'edit'"), "create");
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KApplication app;

    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count() < 1)
    {
        args->usage();
        return -1;
    }

    ConnectionEditor editor(0);
    QString specifics = args->getOption("specific-args");
    QString ssid;
    QVariantList specificArgs;

    if ( specifics.length() )
    {
        if ( specifics[0] == '\'' )
        {
            int index = 1;
            while ( true )
            {
                index = specifics.indexOf( '\'', index );
                if ( index == -1 )
                    break;
                if ( specifics[index-1] != '\\' )
                    break;
            }
            ssid = specifics.mid(1, index - 1 );
            specifics = specifics.mid( index + 1 ).trimmed();
        } else {
            ssid = specifics.left( specifics.indexOf( ' ' ) );
            specifics = specifics.mid( specifics.indexOf( ' ' ) );
        }
    }

    QStringList argsList = specifics.split(' ');

    specificArgs << ssid;

    foreach (QString arg, specifics.split( ' ' )) {
        specificArgs << QVariant(arg);
    }
    kDebug() << specificArgs;

    KNetworkManagerServicePrefs::instance("knetworkmanagerrc");

    if (args->arg(0) == QLatin1String("create")) {
        if (args->isSet("type")) {
            QString type = args->getOption("type");
            QString cid = editor.addConnection(true, editor.connectionTypeForString(args->getOption("type")), specificArgs);
            QDBusInterface ref( "org.kde.kded", "/modules/knetworkmanager",
                                "org.kde.knetworkmanagerd", QDBusConnection::sessionBus() );
            QStringList ids;
            ids << cid;
            ref.call( QLatin1String( "configure" ), ids );
            kDebug() << ref.isValid() << ref.lastError().message() << ref.lastError().name();
        } else {
            args->usage();
            return -1;
        }
    } else {
        if (args->isSet("connection")) {
            QString connectionId = args->getOption("connection");
            kDebug() << "Editing connections is not yet implemented";
            return 0;
            // do edit
        } else {
            args->usage();
            return -1;
        }
    }

    return 0;
}
// vim: sw=4 et sts=4

