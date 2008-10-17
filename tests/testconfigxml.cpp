/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public 
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <QtCore>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KDebug>
#include <KDialog>
#include <KLocale>

#include "configxml.h"
#include "secretstoragehelper.h"

static const char description[] =
    I18N_NOOP("Test harness for extended ConfigXml class");

static const char version[] = "v0.1";

int main( int argc, char** argv )
{
    KAboutData about("TestConfigXml", 0, ki18n("testconfigxml"), version, ki18n(description), KAboutData::License_GPL, ki18n("(C) 2008 Will Stephenson"), KLocalizedString(), 0, "wstephenson@kde.org");
    about.addAuthor( ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    QFile file("settings/802-3-ethernet.kcfg");
    if (!file.exists()) {
        kDebug() << "settings/802-3-ethernet.kcfg not found!";
        return 0;
    }
    // all settings will be in one file so we'll have to pass in a kconfiggroup
    // the secrets need to be stored by connection and setting name
    // could use the kwallet map type - map name=connection-settinggroup 
    // BUT a map is to be stored in the wallet as an entire map, so the mapped settings
    // would have to be collated during writeConfig and written at the end
    // otherwise connection-settinggroup-settingname with single entries.
    // How to obtain the required context to produce these keys?
    // ideal solution: pass a reference to a 'wallet location' into the kconfigskeletonitem as with
    // writeConfig().
    // WHAT IF we could pass it a KConfig* that is actually a KWallet::Wallet?
    // This would require changes in KCoreConfigSkeleton to manage both the config and the wallet
    // Pass a SecretStorageHelper in to ConfigXml ctor with Id and group being managed.  Then pass
    // this down to all ItemSecrets for use when reading/writing secrets.
    ConfigXml config("/tmp/testconfigxmlrc", &file, true, new SecretStorageHelper(QLatin1String("testconfigxml"), QLatin1String("802-3-ethernet")));
    QStringList keys;
    kDebug() << "dumping config contents";
    foreach (KConfigSkeletonItem * item, config.items()) {
        kDebug() << item << " - " << item->key() << item->property();
    }
#if 0
    KConfigSkeletonItem * item = config.findItem(QLatin1String("802-3-ethernet"), "port");
    item->setProperty("ETH");
    item = config.findItem(QLatin1String("802-3-ethernet"), "speed");
    item->setProperty(1000);
    item = config.findItem(QLatin1String("802-3-ethernet"), "duplex");
    item->setProperty(true);
    item = config.findItem(QLatin1String("802-3-ethernet"), "auto-negotiate");
    item->setProperty(true);
#endif
    kDebug() << "trying to change mtu and macaddress";
    KConfigSkeletonItem * item = config.findItem(QLatin1String("802-3-ethernet"), "macaddress");
    if (item) {
        item->setProperty(QByteArray("1234"));
        item = config.findItem(QLatin1String("802-3-ethernet"), "mtu");
        item->setProperty(1500);
        item = config.findItem(QLatin1String("802-3-ethernet"), "dummysecret");
        item->setProperty("hi i'm a secret");
        config.writeConfig();
    } else {
        kDebug() << "  macaddress not found!";
        foreach (KConfigSkeletonItem * item, config.items() ) {
            kDebug() << item->key() << item->property();
        }
    }
//#endif
    return app.exec();
}


