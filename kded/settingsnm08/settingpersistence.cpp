/*
Copyright 2011 Lamarque V. Souza <lamarque@kde.org>

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

#include "settingpersistence.h"

#include <KConfigGroup>

using namespace Knm;

SettingPersistence::SettingPersistence(Setting * setting, KSharedConfig::Ptr config, SettingPersistence::SecretStorageMode mode)
   : m_setting(setting), m_config(new KConfigGroup(config, setting->name())), m_storageMode(mode)
{

}

SettingPersistence::~SettingPersistence()
{
    delete m_config;
}

QByteArray SettingPersistence::macaddressFromString( const QString & s)
{
    QStringList macStringList = s.split(':');
    QByteArray ba;
    if (!s.isEmpty())
    {
        ba.resize(6);
        int i = 0;

        foreach (const QString &macPart, macStringList)
            ba[i++] = macPart.toUInt(0, 16);
    }
    return ba;
}

// vim: sw=4 sts=4 et tw=100
