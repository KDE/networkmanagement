/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "wiredconnectionsetting.h"

//from nm-setting-wired.h
#define NM_SETTING_WIRED_SETTING_NAME "802-3-ethernet"
#define NM_SETTING_WIRED_PORT "port"
#define NM_SETTING_WIRED_SPEED "speed"
#define NM_SETTING_WIRED_DUPLEX "duplex"
#define NM_SETTING_WIRED_AUTO_NEGOTIATE "auto-negotiate"
#define NM_SETTING_WIRED_MAC_ADDRESS "mac-address"
#define NM_SETTING_WIRED_MTU "mtu"

WiredConnectionSetting::WiredConnectionSetting(const KConfigGroup &config, QObject *parent)
    : QObject(parent)
{
    Q_UNUSED(config)
    //take the config and initialize settings
    mtu = 1500;
}

WiredConnectionSetting::~WiredConnectionSetting()
{
}

void WiredConnectionSetting::update(const QMap<QString, QVariant> &updates)
{
    //TODO: use interators
    if (updates.keys().contains(NM_SETTING_WIRED_MAC_ADDRESS)) {
        macAddress = updates[NM_SETTING_WIRED_MAC_ADDRESS].toByteArray();
    }
    if (updates.keys().contains(NM_SETTING_WIRED_MTU)) {
        mtu = updates[NM_SETTING_WIRED_MTU].toUInt();
    }
}

QMap<QString, QVariant> WiredConnectionSetting::settingsMap() const
{
    QMap<QString, QVariant> retVal;
    retVal["name"] = QVariant(NM_SETTING_WIRED_SETTING_NAME);
    retVal[NM_SETTING_WIRED_MAC_ADDRESS] = QVariant(macAddress);
    retVal[NM_SETTING_WIRED_MTU] = QVariant(mtu);

    return retVal;
}

#include "wiredconnectionsetting.moc"
