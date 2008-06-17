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

#include "settings/configxml.h"

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
    ConfigXml config("/tmp/testconfigxmlrc", &file);
    QStringList keys;
    foreach (KConfigSkeletonItem * item, config.items()) {
        kDebug() << item->key() << item->property();
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
    KConfigSkeletonItem * item = config.findItem(QLatin1String("802-3-ethernet"), "mac-address");
    item->setProperty(QByteArray("1234"));
    item = config.findItem(QLatin1String("802-3-ethernet"), "mtu");
    item->setProperty(1500);
    config.writeConfig();
//#endif
    return app.exec();
}


