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

#ifndef CONNECTIONPERSISTENCE_H
#define CONNECTIONPERSISTENCE_H

#include <QObject>
#include <QHash>
#include <QtGui/qwindowdefs.h> // krazy:exclude=includes (for WId)

#include <KSharedConfig>

#include "knm_export.h"

class Connection;
class Setting;
class SettingPersistence;

class KNM_EXPORT ConnectionPersistence : public QObject
{
Q_OBJECT
public:
    enum SecretStorageMode { Secure, PlainText };
    ConnectionPersistence(Connection *, KSharedConfig::Ptr config, SecretStorageMode mode = Secure);
    ~ConnectionPersistence();

    // get/set the window ID used for focus stealing prevention
    static void setWalletWid( WId wid ) {
        s_walletWId = wid;
    }

    static WId walletWid() {
        return s_walletWId;
    }

    void save();
    // This assumes the connection has already been constructed with the uuid, type ctor
    // and populated with all the Settings
    void load();
private:
    SettingPersistence * persistenceFor(Setting *);
    QString walletKeyFor(const Setting *) const;

    static QString s_walletFolderName;
    static WId s_walletWId;

    QHash<Setting*, SettingPersistence*> m_persistences;
    Connection * m_connection;
    KSharedConfig::Ptr m_config;
    SecretStorageMode m_storageMode;
};

#endif // CONNECTIONPERSISTENCE_H
