/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef SETTINGDBUS_H
#define SETTINGDBUS_H

#include <QVariant>

#include "setting.h"

namespace Knm
{
    class Setting;
} // namespace Knm

class SettingDbus
{
public:
    SettingDbus(Knm::Setting *);
    virtual ~SettingDbus();

    virtual void fromMap(const QVariantMap &) = 0;
    virtual QVariantMap toMap() = 0;
    virtual QVariantMap toSecretsMap() = 0;

    // static methods to convert between binary and hex string MAC representation
    static QByteArray macHex2Bin(const QByteArray &hexMac);
    static QByteArray macBin2Hex(const QByteArray &binMac);
protected:
    static void insertIfTrue(QVariantMap& map, const char * key, bool value);
    static void insertIfFalse(QVariantMap& map, const char * key, bool value);
    static void insertIfNonZero(QVariantMap& map, const char * key, uint value);
    static void insertIfNonEmpty(QVariantMap& map, const char * key, const QString & value);
    Knm::Setting *m_setting;
};

#endif // SETTINGDBUS_H
