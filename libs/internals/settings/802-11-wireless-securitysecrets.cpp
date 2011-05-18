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

#include "802-11-wireless-securitysecrets.h"

using namespace Knm;

WirelessSecuritySecrets::WirelessSecuritySecrets(WirelessSecuritySetting * setting) : Secrets(setting), m_setting(setting)
{
}

WirelessSecuritySecrets::~WirelessSecuritySecrets()
{
}

QMap<QString,QString> WirelessSecuritySecrets::secretsToMap() const
{
    QMap<QString,QString> map;
    if (m_setting->wepkeyflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("wepkey0"), m_setting->wepkey0());
        map.insert(QLatin1String("wepkey1"), m_setting->wepkey1());
        map.insert(QLatin1String("wepkey2"), m_setting->wepkey2());
        map.insert(QLatin1String("wepkey3"), m_setting->wepkey3());
    }
    if (m_setting->pskflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("psk"), m_setting->psk());
    }
    if (m_setting->leappasswordflags() & Setting::AgentOwned) {
        map.insert(QLatin1String("leappassword"), m_setting->leappassword());
    }
    return map;
}

void WirelessSecuritySecrets::secretsFromMap(QMap<QString,QString> secrets) const
{
    m_setting->setWepkey0(secrets.value("wepkey0"));
    m_setting->setWepkey1(secrets.value("wepkey1"));
    m_setting->setWepkey2(secrets.value("wepkey2"));
    m_setting->setWepkey3(secrets.value("wepkey3"));
    m_setting->setPsk(secrets.value("psk"));
    m_setting->setLeappassword(secrets.value("leappassword"));
    m_setting->setSecretsAvailable(true);
}

void WirelessSecuritySecrets::secretsToConfig(QMap<QString,QString> secrets, KSharedConfig::Ptr configptr) const
{
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::WirelessSecurity));
    if (m_setting->wepkeyflags() & Setting::AgentOwned) {
        config->writeEntry("wepkey0", secrets.value("wepkey0"));
        config->writeEntry("wepkey1", secrets.value("wepkey1"));
        config->writeEntry("wepkey2", secrets.value("wepkey2"));
        config->writeEntry("wepkey3", secrets.value("wepkey3"));
    }
    if (m_setting->pskflags() & Setting::AgentOwned) {
        config->writeEntry("psk", secrets.value("psk"));
    }
    if (m_setting->leappasswordflags() & Setting::AgentOwned) {
        config->writeEntry("leappassword", secrets.value("leappassword"));
    }
    delete config;
}

QMap<QString,QString> WirelessSecuritySecrets::secretsFromConfig(KSharedConfig::Ptr configptr)
{
    QMap<QString,QString> map;
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::WirelessSecurity));
    if (config->exists()) {
        map.insert(QLatin1String("wepkey0"), config->readEntry("wepkey0", ""));
        map.insert(QLatin1String("wepkey1"), config->readEntry("wepkey1", ""));
        map.insert(QLatin1String("wepkey2"), config->readEntry("wepkey2", ""));
        map.insert(QLatin1String("wepkey3"), config->readEntry("wepkey3", ""));
        map.insert(QLatin1String("psk"), config->readEntry("psk", ""));
        map.insert(QLatin1String("leappassword"), config->readEntry("leappassword", ""));
    }
    delete config;
    return map;
}

QStringList WirelessSecuritySecrets::needSecrets()
{
    QStringList list;
    switch (m_setting->securityType())
    {
        case WirelessSecuritySetting::EnumSecurityType::None:
            break;
        case WirelessSecuritySetting::EnumSecurityType::StaticWep:
        case WirelessSecuritySetting::EnumSecurityType::DynamicWep:
            switch (m_setting->weptxkeyindex())
            {
                case 0:
                    if (m_setting->wepkey0().isEmpty())
                        list.append("wepkey0");
                    break;
                case 1:
                    if (m_setting->wepkey1().isEmpty())
                        list.append("wepkey1");
                    break;
                case 2:
                    if (m_setting->wepkey2().isEmpty())
                        list.append("wepkey2");
                    break;
                case 3:
                    if (m_setting->wepkey3().isEmpty())
                        list.append("wepkey3");
                    break;
            }
            break;
        case WirelessSecuritySetting::EnumSecurityType::WpaPsk:
        case WirelessSecuritySetting::EnumSecurityType::Wpa2Psk:
            if (m_setting->psk().isEmpty())
                list.append("psk");
            break;
        case WirelessSecuritySetting::EnumSecurityType::Leap:
            if (m_setting->leappassword().isEmpty())
                list.append("leappassword");
            break;
    }
    return list;
}
