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
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include "bluetooth.h"

int main(int argc, char **argv)
{
    KAboutData aboutData( "networkmanagement_configshell", "kcm_networkmanagement", ki18n("Network Management"),
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
    options.add("specific-args <args>", ki18n("Space-separated connection type-specific arguments, may be either 'gsm' or 'cdma' for cellular, or 'openvpn' or 'vpnc' for vpn connections, interface and AP identifiers for wireless connections, bluetooth mac address for bluetooth pan, and bluetooth mac address and serial bluetooth identifies (i.e. rfcomm0) for bluetooth dun. Specific args are separated by commas and without spaces."));
    options.add("+mode", ki18n("Operation mode, may be either 'create' or 'edit'"), "create");
    KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.
    KApplication app;

    const KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->count() < 1)
    {
        args->usage();
        return -1;
    }

    KGlobal::locale()->insertCatalog("libknmui");
    ConnectionEditor editor(0);
    QString specifics = args->getOption("specific-args");
    QString ssid;
    QVariantList specificArgs;

    foreach (const QString& arg, specifics.split( ',' )) {
        specificArgs << QVariant(arg);
    }

    kDebug() << specificArgs;

    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    kDebug() << args;

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
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
            /* To create a bluetooth DUN connection:
	     * networkmanagement_configshell create --type bluetooth --specific-args 00:11:22:33:44:55,rfcomm0
	     *
	     * Warning: there is no space in 00:11:22:33:44:55,rfcomm0
	     *
	     * For PANU:
	     * networkmanagement_configshell create --type bluetooth --specific-args 00:11:22:33:44:55
	     */
            else if (type == QLatin1String("bluetooth")) {
                if (specificArgs.count() == 1) {
                    new Bluetooth(specificArgs[0].toString());
                    return app.exec();
		} else if (specificArgs.count() == 2) {
                    new Bluetooth(specificArgs[0].toString(), specificArgs[1].toString());
                    return app.exec();
                } else {
                    return -1;
                }
            }
#endif
            else {
                con = editor.createConnection(true, Knm::Connection::typeFromString(type), specificArgs);
            }

            if(!con)
            {
                kDebug() << Knm::Connection::typeFromString(type) << "type connection cannot be created.";
                return -1;
            }

            saveConnection(con);
        } else if (args->isSet("hiddennetwork")) {
            QString ssidOfHiddenNetwork = args->getOption("hiddennetwork");
            kDebug() << "I have been told to setup a connection to a hidden network..." << ssidOfHiddenNetwork;
            return 0;
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

