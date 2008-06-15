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

#include "wirelessconnectionsetting.h"

#include <KDebug>

//from nm-setting-wireless.h
#define NM_SETTING_WIRELESS_SETTING_NAME "802-11-wireless"
#define NM_SETTING_WIRELESS_SSID        "ssid"
#define NM_SETTING_WIRELESS_MODE        "mode"
#define NM_SETTING_WIRELESS_BAND        "band"
#define NM_SETTING_WIRELESS_CHANNEL     "channel"
#define NM_SETTING_WIRELESS_BSSID       "bssid"
#define NM_SETTING_WIRELESS_RATE        "rate"
#define NM_SETTING_WIRELESS_TX_POWER    "tx-power"
#define NM_SETTING_WIRELESS_MAC_ADDRESS "mac-address"
#define NM_SETTING_WIRELESS_MTU         "mtu"
#define NM_SETTING_WIRELESS_SEEN_BSSIDS "seen-bssids"
#define NM_SETTING_WIRELESS_SEC         "security"

//from nm-setting-wireless.h
#define NM_SETTING_WIRELESS_SECURITY_SETTING_NAME "802-11-wireless-security"
#define NM_SETTING_WIRELESS_SECURITY_KEY_MGMT "key-mgmt"
#define NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX "wep-tx-keyidx"
#define NM_SETTING_WIRELESS_SECURITY_AUTH_ALG "auth-alg"
#define NM_SETTING_WIRELESS_SECURITY_PROTO "proto"
#define NM_SETTING_WIRELESS_SECURITY_PAIRWISE "pairwise"
#define NM_SETTING_WIRELESS_SECURITY_GROUP "group"
#define NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME "leap-username"
#define NM_SETTING_WIRELESS_SECURITY_WEP_KEY0 "wep-key0"
#define NM_SETTING_WIRELESS_SECURITY_WEP_KEY1 "wep-key1"
#define NM_SETTING_WIRELESS_SECURITY_WEP_KEY2 "wep-key2"
#define NM_SETTING_WIRELESS_SECURITY_WEP_KEY3 "wep-key3"
#define NM_SETTING_WIRELESS_SECURITY_PSK "psk"
#define NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD "leap-password"

WirelessConnectionSetting::WirelessConnectionSetting(const KConfigGroup &config, QObject *parent)
    : QObject(parent)
{
    //take the config and initialize settings
    ssid = config.readEntry("ESSID", QString());
    switch (config.readEntry("WirelessMode", 0)) {
        case 0: //Auto
            break;
        case 1: //802.11a
            mode = "a";
            break;
        case 2: //802.11bg
            mode = "bg";
        case 3: //802.11n
            kDebug() << "802.11n is not yet supported.  Switching to auto-mode.";
            break;
        default:
            kDebug() << "Unrecognized mode.";
            break;
    }

    //encryption settings
    KConfigGroup encConfig(&config, "Encryption");
    security = NM_SETTING_WIRELESS_SECURITY_SETTING_NAME;
    QString authType = encConfig.readEntry("AuthenticationType", QString());
    if (authType == "WEP") {
        key_mgmt = "None"; //8021x is not supported yet
        auth_alg = (encConfig.readEntry("WEPAuthentication", QString()) == "Open") ? "OPEN" : "SHARED";//LEAP is not supported yet
        wep_tx_keyidx = encConfig.readEntry("WEPKey", 1) -1;
        wep_key0 = encConfig.readEntry("WEPStaticKey1", QString());
        wep_key1 = encConfig.readEntry("WEPStaticKey2", QString());
        wep_key2 = encConfig.readEntry("WEPStaticKey3", QString());
        wep_key3 = encConfig.readEntry("WEPStaticKey4", QString());

        //clear the previous encryption types
        //WEP40, WEP104, etc?
        switch (encConfig.readEntry("WEPType", 0)) {
            case 0: //WEP-64
                group << "WEP-40";
                break;
            case 1: //WEP-128
                group << "WEP-104";
                break;
            default:
                break;
        }
    } else if (authType == "WPA") {
        kDebug() << "WPA is not supported.";
    }
}

WirelessConnectionSetting::~WirelessConnectionSetting()
{
}

void WirelessConnectionSetting::update(const QMap<QString, QVariant> &updates)
{
    Q_UNUSED(updates)
    //TODO: use interators
    /*if (updates.keys().contains(NM_SETTING_WIRED_MAC_ADDRESS)) {
        macAddress = updates[NM_SETTING_WIRED_MAC_ADDRESS].toByteArray();
    }
    if (updates.keys().contains(NM_SETTING_WIRED_MTU)) {
        mtu = updates[NM_SETTING_WIRED_MTU].toUInt();
    }*/
}

QMap<QString, QVariant> WirelessConnectionSetting::settingsMap() const
{
    QMap<QString, QVariant> retVal;
    retVal["name"] = QVariant(NM_SETTING_WIRELESS_SETTING_NAME);
    retVal[NM_SETTING_WIRELESS_SSID] = QVariant(ssid);
    retVal[NM_SETTING_WIRELESS_MODE] = QVariant(mode);
    //retVal[NM_SETTING_WIRELESS_BAND] = QVariant(band);
    //retVal[NM_SETTING_WIRELESS_CHANNEL] = QVariant(channel);
    //retVal[NM_SETTING_WIRELESS_BSSID] = QVariant(bssid);
    //retVal[NM_SETTING_WIRELESS_RATE] = QVariant(rate);
    //retVal[NM_SETTING_WIRELESS_TX_POWER] = QVariant(tx_power);
    //retVal[NM_SETTING_WIRELESS_MAC_ADDRESS] = QVariant(mac_address);
    //retVal[NM_SETTING_WIRELESS_MTU] = QVariant(mtu);
    //retVal[NM_SETTING_WIRELESS_SEEN_BSSIDS] = QVariant(seen_bssids);
    retVal[NM_SETTING_WIRELESS_SEC] = QVariant(security);

    return retVal;
}

QMap<QString, QVariant> WirelessConnectionSetting::secretsKeyMap() const
{
    QMap<QString, QVariant> retVal;
    retVal["name"] = QVariant(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
    retVal[NM_SETTING_WIRELESS_SECURITY_KEY_MGMT] = QVariant(key_mgmt);
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_TX_KEYIDX] = QVariant(wep_tx_keyidx);
    retVal[NM_SETTING_WIRELESS_SECURITY_AUTH_ALG] = QVariant(auth_alg);
    if (!proto.isEmpty()) retVal[NM_SETTING_WIRELESS_SECURITY_PROTO] = QVariant(proto);
    if (!pairwise.isEmpty()) retVal[NM_SETTING_WIRELESS_SECURITY_PAIRWISE] = QVariant(pairwise);
    if (!group.isEmpty()) retVal[NM_SETTING_WIRELESS_SECURITY_GROUP] = QVariant(group);
    //retVal[NM_SETTING_WIRELESS_SECURITY_LEAP_USERNAME] = QString();
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY0] = QString();
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY1] = QString();
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY2] = QString();
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY3] = QString();
    retVal[NM_SETTING_WIRELESS_SECURITY_PSK] = QString();
    //retVal[NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD] = QString();

    return retVal;
}

QMap<QString, QVariant> WirelessConnectionSetting::secretsMap() const
{
    QMap<QString, QVariant> retVal;
    retVal["name"] = QVariant(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY0] = QVariant(wep_key0);
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY1] = QVariant(wep_key1);
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY2] = QVariant(wep_key2);
    retVal[NM_SETTING_WIRELESS_SECURITY_WEP_KEY3] = QVariant(wep_key3);
    retVal[NM_SETTING_WIRELESS_SECURITY_PSK] = QVariant(psk);
    //retVal[NM_SETTING_WIRELESS_SECURITY_LEAP_PASSWORD] = QVariant(leap_password);

    return retVal;
}

#include "wirelessconnectionsetting.moc"
