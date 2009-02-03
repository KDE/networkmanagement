/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>
    Copyright (C) 2008 Will Stephenson <wstephenson@kde.org>

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

#include "busconnection.h"

#include <QDBusMetaType>
#include <KDebug>

#include "connectionsecretsjob.h"

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
    delete m_connection;
    emit Removed();
}

void BusConnection::Update(QVariantMapMap updates)
{
    kDebug() << "TODO: validate incoming settings";
    kDebug() << "TODO: implement fromDbusMap for all settings!";
    kDebug() << "TODO: replace existing connection with one specified in updates";
    Knm::ConnectionDbus cd(m_connection);
    cd.fromDbusMap(updates);
    emit Updated(cd.toDbusMap());
}


void BusConnection::updateInternal(Knm::Connection * connection)
{
    if (m_job) {
        m_job->kill(KJob::Quietly);
    }
    delete m_connection;
    m_connection = connection;
    Knm::ConnectionDbus cd(m_connection);
    QVariantMapMap map = cd.toDbusMap();
    kDebug() << "emitting Updated" << map;
    emit Updated(cd.toDbusMap());
}

void BusConnection::Delete()
{
    kDebug();
    deleteLater();
}

QVariantMapMap BusConnection::GetSettings() const
{
    Knm::ConnectionDbus cd(m_connection);
    QVariantMapMap map = cd.toDbusMap();
    return map;
}

QVariantMapMap BusConnection::GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new, const QDBusMessage& message)
{
    kDebug() << m_connection->uuid() << setting_name << hints << request_new;
    if (!request_new && !m_connection->hasSecrets()) {
        Knm::ConnectionDbus cd(m_connection);
        return cd.toDbusSecretsMap();
    }
    message.setDelayedReply(true);
    KJob * secretsJob = new ConnectionSecretsJob(m_connection, setting_name, hints, request_new, message);
    connect(secretsJob, SIGNAL(finished(KJob*)), this, SLOT(gotSecrets(KJob*)));
    secretsJob->start();

    return QVariantMapMap();
}

void BusConnection::gotSecrets(KJob *job)
{
    ConnectionSecretsJob * csj = static_cast<ConnectionSecretsJob*>(job);
    if (csj->error() == ConnectionSecretsJob::NoError) {
        Knm::ConnectionDbus db(m_connection);
        QVariantMapMap secrets = db.toDbusSecretsMap();

        QDBusMessage reply = csj->requestMessage().createReply();

        QVariant arg = QVariant::fromValue(secrets);
        reply << arg;
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::EnumError::WalletDisabled ) {
        kDebug() << "ERROR: The KDE wallet is disabled";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SettingError"), "The wallet was disabled");
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::EnumError::WalletNotFound ) {
        kDebug() << "ERROR: The wallet used by KNetworkManager was not found";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SettingError"), "The wallet was not found");
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::EnumError::WalletOpenRefused ) {
        kDebug() << "ERROR: The user refused KNetworkManager permission to open the wallet";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SecretsRefused"), "User refused to supply secrets");
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::EnumError::UserInputCancelled ) {
        kDebug() << "ERROR: The user cancelled the get secrets dialog";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SecretsRefused"), "User refused to supply secrets");
        QDBusConnection::systemBus().send(reply);
    }
}

QString BusConnection::uuid() const
{
    return m_connection->uuid();
}

#include "busconnection.moc"
