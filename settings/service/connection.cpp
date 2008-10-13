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

#include "connection.h"

#include <QDBusMetaType>
#include <KDebug>

#include "connectionsecretsjob.h"

typedef QMap<QString,QVariantMap> QVariantMapMap;

Connection::Connection(const QString & id, const QVariantMapMap & settingsMap, QObject *parent)
    : QObject(parent), mId(id), mSettingsMap(settingsMap), mHasSecrets(false)
{
    qDBusRegisterMetaType<QVariantMapMap>();
}

Connection::~Connection()
{
    emit Removed();
}


bool Connection::hasSecrets() const
{
    return mHasSecrets;
}

void Connection::Update(QVariantMapMap updates)
{
    kDebug();
#if 0
    foreach (const QString &key1, changedParameters.keys()) {
        foreach (const QString &key2, changedParameters[key1].keys()) {
            connectionMap[key1][key2] = changedParameters[key1][key2];
        }
        if (wired) {
            wired->update(updates[NM_SETTING_WIRED_SETTING_NAME]);
        }
    }
    /*if (updates.exists(NM_SETTING_WIRELESS_SETTING_NAME)) {
    }*/
#endif
}

void Connection::Delete()
{
    kDebug();
    deleteLater();
}

QVariantMapMap Connection::GetSettings() const
{
    kDebug();
    return mSettingsMap;
}

QVariantMapMap Connection::GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new, const QDBusMessage& message)
{
    kDebug();
    if (!request_new && hasSecrets()) {
        QVariantMapMap replyOuterMap;
        QVariantMap replyInnerMap;
        if (mSettingsMap.contains(setting_name)) {
            QVariantMap settingGroup = mSettingsMap.value(setting_name);
            foreach (QString setting, hints) {
                replyInnerMap.insert(setting, settingGroup.value(setting));
            }
            replyOuterMap.insert(setting_name, replyInnerMap);
            return replyOuterMap;
        }
    }
    message.setDelayedReply(true);
    QDBusMessage reply = message.createReply();
    QDBusConnection::systemBus().send(reply);
    KJob * secretsJob = new ConnectionSecretsJob(mId, setting_name, hints, request_new, reply);
    connect(secretsJob, SIGNAL(finished(KJob*)), this, SLOT(gotSecrets(KJob*)));
    secretsJob->start();
    return QVariantMapMap();
}

void Connection::gotSecrets(KJob *job)
{
    kDebug();
    ConnectionSecretsJob * csj = static_cast<ConnectionSecretsJob*>(job);
    if (csj->error() == ConnectionSecretsJob::NoError) {
        QVariantMap retrievedSecrets = csj->secrets();
        // update myself
        QVariantMap existingSetting = mSettingsMap.value(csj->settingName());
        QMapIterator<QString,QVariant> i(retrievedSecrets);
        while (i.hasNext()) {
            i.next();
            existingSetting.insert(i.key(), i.value());
        }
        kDebug() << "Updating existing settings for " << csj->settingName() << existingSetting;
        mSettingsMap.insert(csj->settingName(), existingSetting);

        // setup reply
        QVariantMapMap replyOuterMap;
        replyOuterMap.insert(csj->settingName(), retrievedSecrets);
        QDBusMessage reply = csj->message();

        QVariant arg = QVariant::fromValue(replyOuterMap);
        //reply << arg;
        reply << QStringList();
        kDebug() << "Meta type id for QVariantMapMap: " << qMetaTypeId<QVariantMapMap>();
        kDebug() << "Meta type id for arg: " << arg.userType();
        kDebug() << "Meta type id for unregistered type: " << qMetaTypeId<QStringList>();
        kDebug() << "Sending reply: " << reply.arguments() << " count: " << reply.arguments().count() << " with signature: " << reply.signature();
        QDBusConnection::systemBus().send(reply);
    }
}

#include "connection.moc"
