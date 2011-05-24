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

#include "802-1xsecrets.h"

using namespace Knm;

Security8021xSecrets::Security8021xSecrets(Security8021xSetting * setting) : Secrets(setting), m_setting(setting)
{
}

Security8021xSecrets::Security8021xSecrets(Security8021xSecrets *secrets, Security8021xSetting * setting) : Secrets(secrets, setting), m_setting(setting)
{
}


Security8021xSecrets::~Security8021xSecrets()
{
}

QMap<QString,QString> Security8021xSecrets::secretsToMap() const
{
    QMap<QString,QString> map;
    if (m_setting->passwordflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("password"), m_setting->password());
    }
    if (m_setting->privatekeypasswordflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("private-key-password"), m_setting->privatekeypassword());
    }
    if (m_setting->phase2privatekeypasswordflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("phase2-private-key-password"), m_setting->phase2privatekeypassword());
    }
    return map;
}

void Security8021xSecrets::secretsFromMap(QMap<QString,QString> secrets) const
{
    m_setting->setPassword(secrets.value("password"));
    m_setting->setPrivatekeypassword(secrets.value("private-key-password"));
    m_setting->setPhase2privatekeypassword(secrets.value("phase2-private-key-password"));
}

void Security8021xSecrets::secretsToConfig(QMap<QString,QString> secrets, KSharedConfig::Ptr configptr) const
{
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::Security8021x));
    if (m_setting->passwordflags() & Setting::AgentOwned) {
        config->writeEntry("password", secrets.value("password"));
    }
    if (m_setting->privatekeypasswordflags() & Setting::AgentOwned) {
        config->writeEntry("private-key-password", secrets.value("private-key-password"));
    }
    if (m_setting->phase2privatekeypasswordflags() & Setting::AgentOwned) {
        config->writeEntry("phase2-private-key-password", secrets.value("phase2-private-key-password"));
    }
    delete config;
}

QMap<QString,QString> Security8021xSecrets::secretsFromConfig(KSharedConfig::Ptr configptr)
{
    QMap<QString,QString> map;
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::Security8021x));
    if (config->exists()) {
        map.insert(QLatin1String("password"), config->readEntry("password", ""));
        map.insert(QLatin1String("private-key-password"), config->readEntry("private-key-password", ""));
        map.insert(QLatin1String("phase2-private-key-password"), config->readEntry("phase2-private-key-password", ""));
    }
    delete config;
    return map;
}

QStringList Security8021xSecrets::needSecrets()
{
    QStringList list;
    if (m_setting->phase2auth() == Security8021xSetting::EnumPhase2auth::none) {
        if (m_setting->privatekeypassword().isEmpty())
                list.append("private-key-password");
    } else {
        if (m_setting->password().isEmpty())
                list.append("password");
        if (m_setting->phase2privatekeypassword().isEmpty())
            list.append("phase2-private-key-password");
    }
    return list;
}
