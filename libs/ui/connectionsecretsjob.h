/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONNECTIONSECRETSJOB_H
#define CONNECTIONSECRETSJOB_H

#include <QDBusMessage>
#include <QStringList>

#include <KJob>

#include "connectionpersistence.h"

#include "knm_export.h"

class KDialog;
class SettingWidget;

/**
 * A job that encapsulates looking up a specified set of Secrets from (secure) storage, ask the user
 * if not found or if requestNew is set, set the new secrets back on the Connection object, and 
 * write the connection to disk
 */
class KNM_EXPORT ConnectionSecretsJob : public KJob
{
Q_OBJECT
public:
    class EnumError : public Knm::ConnectionPersistence::EnumError
    {
    public:
        enum type { UserInputCancelled = Knm::ConnectionPersistence::EnumError::WalletOpenRefused + 1};

    };
    ConnectionSecretsJob(Knm::Connection * connection,
            const QString &settingName,
            const QStringList &secrets,
            bool requestNew,
            const QDBusMessage& request);
    ~ConnectionSecretsJob();
    void start();
    Knm::Connection * connection() const;
    QString settingName() const;
    QVariantMap secrets() const;
    QDBusMessage requestMessage() const;

public Q_SLOTS:
    void doWork();
    void dialogAccepted();
    void dialogRejected();

protected Q_SLOTS:
    void gotPersistedSecrets(uint);

private:
    void doAskUser();
    QString keyForEntry(const QString & entry) const;

    Knm::Connection * m_connection;
    Knm::ConnectionPersistence * m_connectionPersistence;

    QString mSettingName;
    QVariantMap mSecrets;
    bool mRequestNew;
    QDBusMessage mRequest;
    KDialog * m_askUserDialog;
    SettingWidget * m_settingWidget;
};

#endif // CONNECTIONSECRETSJOB_H
