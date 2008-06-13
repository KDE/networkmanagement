/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

//QT specific includes
#include <QList>
#include <QtDBus/QtDBus>

//kde specific includes
#include <KUniqueApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KAboutData>
#include <KLocale>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>

#include "networksettings.h"

#include <NetworkManager.h>

int main(int args, char **argv)
{
    KAboutData aboutData(QByteArray("SettingsDaemonTest"), 0, ki18n("SettingsDaemonTest"),
                         KDE_VERSION_STRING, ki18n("test"), KAboutData::License_GPL,
                         ki18n("test"));
    KCmdLineArgs::init(args, argv, &aboutData);
    KApplication app;

    KConfig config("/etc/networkconfig");
    KConfigGroup configGroup(&config, "Profiles");
    kDebug() << "Available profiles . . . ";
    foreach (const QString &profile, configGroup.groupList()) {
        kDebug() << profile;
        }

    //load wired
    NetworkSettings nmSettings(configGroup);
    if (nmSettings.isValid()) {
        if (!nmSettings.loadProfile("Wired")) {
            kDebug() << "Profile could not be loaded.";
        } else {
            kDebug() << "Profile loaded.";
        }
    } else {
        return 1;
    }
    return app.exec();
}
