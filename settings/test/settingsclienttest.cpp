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
#include <KDebug>

#include "client.h"

#include <NetworkManager.h>

int main(int args, char **argv)
{
KAboutData aboutData(QByteArray("SettingsClientTest"), 0, ki18n("SettingsClientTest"),
                         KDE_VERSION_STRING, ki18n("test"), KAboutData::License_GPL,
                         ki18n("test"));
    KCmdLineArgs::init(args, argv, &aboutData);
    KApplication app;

    Client settings;
    QStringList connections = settings.connections();
    kDebug() << "Current connections are: " << connections;
    if (!connections.isEmpty()) {
        kDebug() << "Getting Settings . . . ";
        QVariantMapMap map = settings.settings(connections[0]);
        foreach (const QString &key1, map.keys()) {
            foreach(const QString &key2, map[key1].keys()) {
                kDebug() << "map[" << key1 << "][" << key2 << "] = " << map[key1][key2];
            }
        }
    }
    return app.exec();
}
