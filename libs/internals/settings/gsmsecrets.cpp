/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#include "gsmsecrets.h"

using namespace Knm;

GsmSecrets::GsmSecrets(GsmSetting * setting) : Secrets(setting), m_setting(setting)
{
}

GsmSecrets::GsmSecrets(GsmSecrets * secrets, GsmSetting * setting) : Secrets(secrets, setting), m_setting(setting)
{
}

GsmSecrets::~GsmSecrets()
{
}

QMap<QString,QString> GsmSecrets::secretsToMap() const
{
    QMap<QString,QString> map;
    if (m_setting->passwordflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("password"), m_setting->password());
    }
    if (m_setting->pinflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("pin"), m_setting->pin());
    }
    return map;
}

void GsmSecrets::secretsFromMap(QMap<QString,QString> secrets) const
{
    m_setting->setPassword(secrets.value("password"));
    m_setting->setPassword(secrets.value("pin"));
}

void GsmSecrets::secretsToConfig(QMap<QString,QString> secrets, KSharedConfig::Ptr configptr) const
{
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::Gsm));
    if (m_setting->passwordflags() & Setting::AgentOwned) {
        config->writeEntry("password", secrets.value("password"));
    }
    if (m_setting->pinflags() & Setting::AgentOwned) {
        config->writeEntry("pin", secrets.value("pin"));
    }
    delete config;
}

QMap<QString,QString> GsmSecrets::secretsFromConfig(KSharedConfig::Ptr configptr)
{
    QMap<QString,QString> map;
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::Gsm));
    if (config->exists()) {
        map.insert(QLatin1String("password"), config->readEntry("password", ""));
        map.insert(QLatin1String("pin"), config->readEntry("pin", ""));
    }
    delete config;
    return map;
}

QStringList GsmSecrets::needSecrets()
{
    QStringList list;
    if (m_setting->password().isEmpty())
         list.append("password");
    if (m_setting->pin().isEmpty())
        list.append("pin");
    return list;
}
