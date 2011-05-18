/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
Copyright (C) 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "busconnection.h"

#include <QDBusMetaType>
#include <KDebug>

//#include "connectionsecretsjob.h"

#include "connection.h"
#include "connectiondbus.h"

typedef QMap<QString,QVariantMap> QVariantMapMap;

BusConnection::BusConnection(Knm::Connection * connection, QObject *parent)
    : QObject(parent), m_connection(connection), m_job(0)
{
    qDBusRegisterMetaType<QVariantMapMap>();
    qDBusRegisterMetaType<QList<uint> >();
    qDBusRegisterMetaType<QStringMap>();
}

BusConnection::~BusConnection()
{
    emit Removed();
}

Knm::Connection * BusConnection::connection() const
{
    return m_connection;
}

void BusConnection::Update(QVariantMapMap updates)
{
    kDebug() << "TODO: validate incoming settings";
    kDebug() << "TODO: implement fromDbusMap for all settings!";
    kDebug() << "TODO: replace existing connection with one specified in updates";
    ConnectionDbus cd(m_connection);
    cd.fromDbusMap(updates);
    emit Updated(cd.toDbusMap());
}


void BusConnection::updateInternal(Knm::Connection * connection)
{
    /*if (m_job) {
        // the user updated the connection using the KCM, _while_ a CSJ
        // for another GetSecrets was running
        ConnectionSecretsJob * newJob = new ConnectionSecretsJob(connection, m_job->settingName(), m_job->secrets().keys(), false, m_job->requestMessage());
        m_job->kill(KJob::Quietly);

        m_job = newJob;
        connect(m_job, SIGNAL(finished(KJob*)), this, SLOT(gotSecrets(KJob*)));
        m_job->start();
    }*/
    m_connection = connection;
    ConnectionDbus cd(m_connection);
    QVariantMapMap map = cd.toDbusMap();
    kDebug() << "emitting Updated" << map;
    emit Updated(cd.toDbusMap());
}

void BusConnection::Delete()
{
    kDebug();
    /*if (m_job) {
        m_job->kill(KJob::Quietly);
    }*/
    deleteLater();
}

QVariantMapMap BusConnection::GetSettings() const
{
    ConnectionDbus cd(m_connection);
    QVariantMapMap map = cd.toDbusMap();
    return map;
}

QVariantMapMap BusConnection::GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new, const QDBusMessage& message)
{
    /*if (m_job) {
        kDebug() << "existing job, ignoring";
        return QVariantMapMap();
    } else {
        kDebug() << m_connection->uuid() << setting_name << hints << request_new;
        if (!request_new && !m_connection->hasSecrets()) {
            ConnectionDbus cd(m_connection);
            return cd.toDbusSecretsMap();
        }
        message.setDelayedReply(true);
        m_job = new ConnectionSecretsJob(m_connection, setting_name, hints, request_new, message);
        connect(m_job, SIGNAL(finished(KJob*)), this, SLOT(gotSecrets(KJob*)));
        m_job->start();

        return QVariantMapMap();
    } */
    return QVariantMapMap();
}

void BusConnection::gotSecrets(KJob *job)
{
 /*   ConnectionSecretsJob * csj = static_cast<ConnectionSecretsJob*>(job);
    if (csj == m_job) {
        if (m_job->error() == ConnectionSecretsJob::NoError) {
            ConnectionDbus db(m_connection);
            QVariantMapMap secrets = db.toDbusSecretsMap();

            QDBusMessage reply = m_job->requestMessage().createReply();

            QVariant arg = QVariant::fromValue(secrets);
            reply << arg;
            QDBusConnection::systemBus().send(reply);
        } else if (m_job->error() == ConnectionSecretsJob::EnumError::WalletDisabled ) {
            kDebug() << "ERROR: The KDE wallet is disabled";
            QDBusMessage reply = m_job->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SettingError"), "The wallet was disabled");
            QDBusConnection::systemBus().send(reply);
        } else if (m_job->error() == ConnectionSecretsJob::EnumError::WalletNotFound ) {
            kDebug() << "ERROR: The wallet used by KDE Network Management was not found";
            QDBusMessage reply = m_job->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SettingError"), "The wallet was not found");
            QDBusConnection::systemBus().send(reply);
        } else if (m_job->error() == ConnectionSecretsJob::EnumError::WalletOpenRefused ) {
            kDebug() << "ERROR: The user refused KDE Network Management (plasma) permission to open the wallet";
            QDBusMessage reply = m_job->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SecretsRefused"), "User refused to supply secrets");
            QDBusConnection::systemBus().send(reply);
        } else if (m_job->error() == ConnectionSecretsJob::EnumError::UserInputCancelled ) {
            kDebug() << "ERROR: The user cancelled the get secrets dialog";
            QDBusMessage reply = m_job->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SecretsRefused"), "User refused to supply secrets");
            QDBusConnection::systemBus().send(reply);
        }
        m_job = 0;
    }*/
}

QString BusConnection::uuid() const
{
    return m_connection->uuid();
}

