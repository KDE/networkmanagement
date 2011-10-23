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

#include <kaboutdata.h>
#include <KUniqueApplication>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include "bluetooth.h"
#include "manageconnection.h"

int main(int argc, char **argv)
{
    KAboutData aboutData( "networkmanagement_configshell", "libknetworkmanager", ki18n("Network Management"),
                          0,
                          ki18n("Create network connections standalone"),
                          KAboutData::License_GPL,
                          ki18n("(c) 2008 Will Stephenson") );

    aboutData.addAuthor(ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("connection <connection-id>", ki18n("Connection ID to edit"));
    options.add("hiddennetwork <ssid>", ki18n("Connect to a hidden wireless network"));
    options.add("type <type>", ki18n("Connection type to create, must be one of '802-3-ethernet', '802-11-wireless', 'pppoe', 'vpn', 'cellular', 'bluetooth'"));
    options.add("specific-args <args>", ki18n("Space-separated connection type-specific arguments, may be either 'gsm' or 'cdma' for cellular connections,\n'openvpn' or 'vpnc' for vpn connections,\ninterface and AP identifiers for wireless connections,\nbluetooth mac address and service ('dun' or 'nap') for bluetooth connections.\n\nYou can also pass the serial device (i.e. 'rfcomm0') instead of service for dun bluetooth connections,\nin that case this program will block waiting for that device to be registered in ModemManager."));
    options.add("+mode", ki18n("Operation mode, may be either 'create' or 'edit'"), "create");
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KUniqueApplication app;

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

    foreach (const QString& arg, specifics.split( ' ' )) {
        specificArgs << QVariant(arg);
    }

    kDebug(KDE_DEFAULT_DEBUG_AREA) << specificArgs;
    kDebug(KDE_DEFAULT_DEBUG_AREA) << args;
    QTextStream qout(stdout, QIODevice::WriteOnly);

    if (args->arg(0) == QLatin1String("create")) {
        if (args->isSet("type")) {
            const QString type = args->getOption("type");
            Knm::Connection *con = 0;
            if (type == QLatin1String("cellular")) {
                MobileConnectionWizard *mobileConnectionWizard = new MobileConnectionWizard();

                if (mobileConnectionWizard->exec() == QDialog::Accepted &&
                    mobileConnectionWizard->getError() == MobileProviders::Success) {
                    con = editor.createConnection(true, mobileConnectionWizard->type(), mobileConnectionWizard->args(), false);
                }
                delete mobileConnectionWizard;
            }
            /* To create a bluetooth DUN connection:
             * networkmanagement_configshell create --type bluetooth --specific-args "00:11:22:33:44:55 dun"
             *     or
             * networkmanagement_configshell create --type bluetooth --specific-args "00:11:22:33:44:55 rfcomm0"
             *
             * in the latter case networkmanagement_configshell will block waiting for the device rfcomm0 to be
             * registered in ModemManager.
             */
            else if (type == QLatin1String("bluetooth")) {
                if (specificArgs.count() == 2) {
                    new Bluetooth(specificArgs[0].toString(), specificArgs[1].toString());
                    return app.exec();
                } else {
                    qout << i18n("Expected two specific args, found %1: %2", specificArgs.count(), specifics) << "\n"; 
                    return -1;
                }
            } else {
                con = editor.createConnection(true, Knm::Connection::typeFromString(type), specificArgs);
            }

            if (!con) {
                kDebug(KDE_DEFAULT_DEBUG_AREA) << Knm::Connection::typeFromString(type) << "type connection cannot be created.";
                return -1;
            }

            /* TODO: test if connection already exists and in case affirmative do not add it. */
            ManageConnection::saveConnection(con);
            return app.exec();
        } else {
            args->usage();
            return -1;
        }
    } else {
        if (args->isSet("connection")) {
            QString connectionId = args->getOption("connection");
            kDebug(KDE_DEFAULT_DEBUG_AREA) << "Editing connections is not yet implemented";
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

