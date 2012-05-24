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

#include "ctype.h"

#include "settingdbus.h"

using namespace Knm;

SettingDbus::SettingDbus(Setting * setting)
   : m_setting(setting)
{

}

SettingDbus::~SettingDbus()
{

}

// convert MAC from hex string to binary
// e.g. 00:11:22:dd:ee:ff -> 6-byte array
QByteArray SettingDbus::macHex2Bin(const QByteArray &hexMac)
{
    const int MAC_STR_LEN = 17;
    QByteArray ba = hexMac;

    if (ba.isEmpty()) {
        return ba;
    }

    // Check the MAC first and correct it.
    // Although fromHex() ignores invalid characters, it scans the array from
    // the end; so add missing zeroes to have even number of characters.
    for (int i = 0; i < MAC_STR_LEN; i++) {
        char ch = i < ba.size() ? ba.at(i) : ':';
        int mod = i%3;
        if (mod != 2) {
            if (ch == ':') ba.insert(i-mod, "0");
            else if (!isxdigit(ch)) ba[i] = '0';
        } else {
            if (ch != ':') ba.insert(i, ":");
        }
    }
    ba.resize(MAC_STR_LEN);

    return QByteArray::fromHex(ba);
}

// convert binary MAC to hex string (human readable)
// e.g. 6-byte array -> 00:11:22:dd:ee:ff
QByteArray SettingDbus::macBin2Hex(const QByteArray &binMac)
{
    const int MAC_STR_LEN = 17;
    QByteArray ba = binMac;

    if (ba.isEmpty()) {
        return ba;
    }

    ba = ba.toHex().toUpper();
    ba.insert(2, ':');
    ba.insert(5, ':');
    ba.insert(8, ':');
    ba.insert(11, ':');
    ba.insert(14, ':');
    ba.resize(MAC_STR_LEN);

    return ba;
}

void SettingDbus::insertIfTrue(QVariantMap& map, const char * key, bool setting)
{
    if (setting) {
        map.insert(QLatin1String(key), true);
    }
}

void SettingDbus::insertIfFalse(QVariantMap& map, const char * key, bool setting)
{
    if (!setting) {
        map.insert(QLatin1String(key), false);
    }
}

void SettingDbus::insertIfNonZero(QVariantMap& map, const char * key, uint setting)
{
    if (setting != 0) {
        map.insert(QLatin1String(key), setting);
    }
}

void SettingDbus::insertIfNonEmpty(QVariantMap& map, const char * key, const QString & setting)
{
    if (!setting.isEmpty()) {
        map.insert(QLatin1String(key), setting);
    }
}


// vim: sw=4 sts=4 et tw=100
