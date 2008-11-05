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

class KConfigDialog;
class SettingWidget;

class ConnectionSecretsJob : public KJob
{
Q_OBJECT
public:
    enum ErrorCode { NoError = 0, WalletDisabled, WalletNotFound, WalletOpenRefused, UserInputCancelled };
    ConnectionSecretsJob(const QString &connectionId, const QString &settingName, const QStringList &secrets, bool requestNew, const QDBusMessage& request);
    ~ConnectionSecretsJob();
    void start();
    QString settingName() const;
    QVariantMap secrets() const;
    QDBusMessage requestMessage() const;


public Q_SLOTS:
    void doWork();
    void dialogAccepted();
    void dialogRejected();
    void walletOpenedForRead(bool success);
    void walletOpenedForWrite(bool success);
private:
    void doAskUser();
    QString keyForEntry(const QString & entry) const;
    QString mConnectionId;
    QString mSettingName;
    QVariantMap mSecrets;
    bool mRequestNew;
    QDBusMessage mRequest;
    KConfigDialog * m_askUserDialog;
    SettingWidget * m_settingWidget;
};

#endif // CONNECTIONSECRETSJOB_H
