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

#ifndef WIRELESS_CONNECTION_H
#define WIRELESS_CONNECTION_H

#include <QObject>
#include <QVariant>
#include <QMap>
#include <QString>

#include <KConfigGroup>

class WirelessConnectionSetting : public QObject
{
    Q_OBJECT

    public:
        WirelessConnectionSetting(const KConfigGroup &config, QObject *parent=0);
        ~WirelessConnectionSetting();

        QMap<QString, QVariant> settingsMap() const;
        QMap<QString, QVariant> secretsKeyMap() const;
        QMap<QString, QVariant> secretsMap() const;
        void update(const QMap<QString, QVariant> &updates);

    private:
        //settings
        QString ssid;
        QString mode;
        //QString band;
        //not implemented yet: quint32 channel;
        //not implemented yet: QByteArray bssid;
        //not implemented yet: quint32 rate;
        //not implemented yet: quint32 tx_power;
        //not implemented yet: QByteArray mac_address;
        //not implemented yet: quint32 mtu;
        //not implemented yet: QStringList seen_bssids;
        QString security;

        //secrets
        QString key_mgmt;
        quint32 wep_tx_keyidx;
        QString auth_alg;
        QStringList proto;
        QStringList pairwise;
        QStringList group;
        //not implemented yet: QString leap_username;
        QString wep_key0;
        QString wep_key1;
        QString wep_key2;
        QString wep_key3;
        QString psk;
        //not implemented yet: QString leap_password;
};

#endif
