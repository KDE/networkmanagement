/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>

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

#include "settinginterface.h"

#include <QFile>
#include <KDebug>
#include <KStandardDirs>
#include "configxml.h"
#include "secretstoragehelper.h"

class SettingInterface::Private
{
public:
    QString connectionId;
    ConfigXml * configXml;
    SecretStorageHelper * secretStorage;
};


SettingInterface::SettingInterface(const QString& connectionId) : d(new Private)
{
    d->connectionId = connectionId;
    d->secretStorage = 0;
    d->configXml = 0;
}

SettingInterface::~SettingInterface()
{
    delete d->configXml;
    delete d;
}

void SettingInterface::init()
{
    QFile schemaFile(KStandardDirs::locate("data",
            QString::fromLatin1("knetworkmanager/schemas/%1.kcfg").arg( settingName())));
    if (schemaFile.exists()) {
        d->secretStorage = new SecretStorageHelper(d->connectionId, settingName());
        QString configFile = KStandardDirs::locateLocal("data",
                QLatin1String("knetworkmanager/connections/") + d->connectionId);
        kDebug() << "Opening config file: " << configFile;
        d->configXml = new ConfigXml(KSharedConfig::openConfig(configFile), &schemaFile, true, d->secretStorage);
    }
}

QString SettingInterface::connectionId() const
{
    return d->connectionId;
}

void SettingInterface::readConfig()
{
    //default impl does nothing
}

void SettingInterface::writeConfig()
{
    //default impl does nothing
}

QVariantMap SettingInterface::secrets() const
{
    kDebug() << "DEFAULT IMPL";
    return QVariantMap();
}

ConfigXml * SettingInterface::configXml() const
{
    return d->configXml;
}

bool SettingInterface::validate() const
{
    return true;
}

// vim: sw=4 sts=4 et tw=100
