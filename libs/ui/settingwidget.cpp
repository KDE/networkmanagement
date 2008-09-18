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

#include "settingwidget.h"

#include <QFile>
#include <KDebug>
#include <KStandardDirs>
#include "configxml.h"
#include "secretstoragehelper.h"

class SettingWidget::Private
{
public:
    QString connectionId;
    ConfigXml * configXml;
    SecretStorageHelper * secretStorage;
};


SettingWidget::SettingWidget(const QString& connectionId, QWidget* parent) : QWidget(parent), d(new Private)
{
    d->connectionId = connectionId;
    d->secretStorage = 0;
    d->configXml = 0;
}

SettingWidget::~SettingWidget()
{
    delete d->configXml;
    delete d;
}

void SettingWidget::init()
{
    QFile schemaFile(KStandardDirs::locate("data",
            QString::fromLatin1("knetworkmanager/schemas/%1.kcfg").arg( settingName())));
    if (schemaFile.exists()) {
        d->secretStorage = new SecretStorageHelper(d->connectionId, settingName());
        QString configFile = KStandardDirs::locateLocal("data",
                QLatin1String("knetworkmanager/connections/") + d->connectionId);
        kDebug() << "Opening config file: " << configFile;
        d->configXml = new ConfigXml(KSharedConfig::openConfig(configFile), &schemaFile, d->secretStorage);
    }
}

void SettingWidget::readConfig(KConfig *)
{
}

void SettingWidget::writeConfig(KConfig *)
{
}

ConfigXml * SettingWidget::configXml() const
{
    return d->configXml;
}

// vim: sw=4 sts=4 et tw=100
