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

#ifndef KNM_INTERNALS_SETTINGPERSISTENCE_H
#define KNM_INTERNALS_SETTINGPERSISTENCE_H

#include "setting.h"

namespace Knm
{

class SettingPersistence
{
public:
    enum SecretStorageMode { DontStore, PlainText, Secure};

    SettingPersistence(Setting *, KSharedConfig::Ptr, SettingPersistence::SecretStorageMode mode = SettingPersistence::Secure);
    virtual ~SettingPersistence();

    virtual void load() = 0;
    static QByteArray macaddressFromString( const QString & s);

protected:
    Setting *m_setting;
    KConfigGroup * m_config;
    SettingPersistence::SecretStorageMode m_storageMode;
};

} // namespace Knm

#endif // SETTINGPERSISTENCE_H
