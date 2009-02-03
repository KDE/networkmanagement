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

namespace Knm
{
class Connection;
class Setting;
class SettingPersistence;

class KNM_EXPORT ConnectionPersistence : public QObject
{
Q_OBJECT
public:
    static const QString CONNECTION_PERSISTENCE_PATH;
    enum SecretStorageMode { Secure, PlainText };
    class EnumError
    {
    public:
        enum type {NoError = 0, MissingContents, WalletDisabled, WalletNotFound, WalletOpenRefused };
    };
    /**
     * Work with an existing Connection
     */
    ConnectionPersistence(Connection *, KSharedConfig::Ptr config, SecretStorageMode mode = Secure);
    /**
     * Deserialise a connection from the specified config file
     */
    ConnectionPersistence(KSharedConfig::Ptr config, SecretStorageMode mode = Secure);

    ~ConnectionPersistence();

    Connection * connection() const;

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
    /**
     * Trigger asynchronously loading this connection's secrets.
     * You should connect to the @ref gotSecrets() signal before calling this method
     * If the SecretStorageMode is PlainText, the signal will be emitted immediately!
     */
    void loadSecrets();
Q_SIGNALS:
    /**
     * Emitted when secrets have become available
     */
    void loadSecretsResult(uint);
protected Q_SLOTS:
    void walletOpenedForRead(bool);
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

} // namespace Knm

#endif // CONNECTIONPERSISTENCE_H
