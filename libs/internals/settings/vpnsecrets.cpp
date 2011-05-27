/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#include "vpnsecrets.h"
#include "vpn.h"

using namespace Knm;

VpnSecrets::VpnSecrets(VpnSetting * setting) : Secrets(setting), m_setting(setting)
{
}

VpnSecrets::VpnSecrets(VpnSecrets * secrets, VpnSetting * setting) : Secrets(secrets, setting), m_setting(setting)
{
}

VpnSecrets::~VpnSecrets()
{
}

QVariantMap VpnSecrets::variantMapFromStringList(const QStringList & list)
{
    QVariantMap secretsMap;
    if (list.count() % 2 == 0) {
        for ( int i = 0; i < list.count(); i += 2 ) {
            secretsMap.insert( list[i], list[i+1] );
        }
    }
    return secretsMap;
}

QStringList VpnSecrets::variantMapToStringList(const QVariantMap & map)
{
    QStringList rawSecrets;
    QMapIterator<QString,QVariant> i(map);
    while (i.hasNext()) {
        i.next();
        rawSecrets << i.key() << i.value().toString();
    }
    return rawSecrets;
}

QStringMap VpnSecrets::stringMapFromStringList(const QStringList & list)
{
    QStringMap map;
    if (list.count() % 2 == 0) {
        for ( int i = 0; i < list.count(); i += 2 ) {
            map.insert( list[i], list[i+1] );
        }
    }
    return map;
}

QStringList VpnSecrets::stringMapToStringList(const QStringMap & map)
{
    QStringList rawSecrets;
    QStringMapIterator i(map);
    while (i.hasNext()) {
        i.next();
        rawSecrets << i.key() << i.value();
    }
    return rawSecrets;
}

QMap<QString,QString> VpnSecrets::secretsToMap() const
{
    QMap<QString,QString> map;
    map.insert(QLatin1String("VpnSecrets"), variantMapToStringList(secretsToSave(m_setting->secretsStorageType(), m_setting->vpnSecrets())).join(QLatin1String("%SEP%")));
    return map;
}

void VpnSecrets::secretsFromMap(QMap<QString,QString> secrets) const
{
    //  if (m_storageMode == ConnectionSecrets::Secure) {
    m_setting->setVpnSecrets(stringMapFromStringList(secrets.value("VpnSecrets").split("%SEP%")));
    m_setting->setSecretsAvailable(true);
    //  }
}

void VpnSecrets::secretsToConfig(QMap<QString,QString> secrets, KSharedConfig::Ptr configptr) const
{
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::Vpn));
    // VPN does not have secret flag, assuming it is always AgentOwned.
    config->writeEntry("VpnSecrets", secrets.value("VpnSecrets"));
    delete config;
}

QMap<QString,QString> VpnSecrets::secretsFromConfig(KSharedConfig::Ptr configptr)
{
    QMap<QString,QString> map;
    KConfigGroup * config = new KConfigGroup(configptr, Setting::typeAsString(Setting::Vpn));
    if (config->exists()) {
        map.insert(QLatin1String("VpnSecrets"), config->readEntry("VpnSecrets", ""));
    }
    delete config;
    return map;
}

QStringList VpnSecrets::needSecrets()
{
    // TODO: implement.
    return QStringList();
}

QVariantMap VpnSecrets::secretsToSave(const QStringMap & type, const QStringMap & secrets)
{
    QVariantMap toSave;
    QMapIterator<QString,QString> i(secrets);
    while (i.hasNext()) {
        i.next();
        if (type[i.key()].isNull() || type[i.key()] == QLatin1String(NM_VPN_PW_TYPE_SAVE))
            toSave.insert( i.key(), i.value() );
    }
    return toSave;
}
