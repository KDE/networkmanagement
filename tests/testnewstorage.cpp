/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "connection.h"
#include "connectiondbus.h"
#include "connectionpersistence.h"
#include "settings/802-11-wireless.h"

static const char description[] =
    I18N_NOOP("Test harness for new statically generated connection storage layer");

static const char version[] = "v0.1";

int main( int argc, char** argv )
{
    KAboutData about("TestNewStorage", 0, ki18n("testnewstorage"), version, ki18n(description), KAboutData::License_GPL, ki18n("(C) 2009 Will Stephenson"), KLocalizedString(), 0, "wstephenson@kde.org");
    about.addAuthor( ki18n("Will Stephenson"), KLocalizedString(), "wstephenson@kde.org" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    Connection foo(QUuid::createUuid(), Connection::Wireless);
    WirelessSetting * ws = new WirelessSetting();
    ws->setSsid("stevello");
    ws->setMode(WirelessSetting::EnumMode::infrastructure);
    foo.addSetting(ws);

    ConnectionPersistence bar(&foo, KSharedConfig::openConfig("./testnewstoragerc"));
    bar.load();

    ConnectionDbus baz(&foo);
    kDebug() << baz.toDbusMap();
    return app.exec();
}


