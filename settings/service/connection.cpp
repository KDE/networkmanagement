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

#include <nm-setting-wireless.h>
#include <nm-setting-wireless-security.h>

#include "connection.h"

#include <QDBusMetaType>
#include <KDebug>

#include "connectionsecretsjob.h"
#include "pbkdf2.h"

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
    kDebug() << "TODO: validate incoming settings";
    mSettingsMap = updates;
    emit Updated(mSettingsMap);
}

void Connection::Delete()
{
    kDebug();
    deleteLater();
}

QVariantMapMap Connection::GetSettings() const
{
    return mSettingsMap;
}

QVariantMapMap Connection::GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new, const QDBusMessage& message)
{
    kDebug() << setting_name << hints << request_new;
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
    KJob * secretsJob = new ConnectionSecretsJob(mId, setting_name, hints, request_new, message);
    connect(secretsJob, SIGNAL(finished(KJob*)), this, SLOT(gotSecrets(KJob*)));
    secretsJob->start();
    return QVariantMapMap();
}

void Connection::gotSecrets(KJob *job)
{
    ConnectionSecretsJob * csj = static_cast<ConnectionSecretsJob*>(job);
    if (csj->error() == ConnectionSecretsJob::NoError) {
        QVariantMap retrievedSecrets = csj->secrets();
        kDebug() << "Got secrets: " << retrievedSecrets;
        // HACK special case for WPA-PSK
        if (retrievedSecrets.contains(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK))) {
#define WPA_PMK_LEN 32
            QString psk = retrievedSecrets.value(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK)).toString();
            QString essid = mSettingsMap.value(QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)).value(QLatin1String(NM_SETTING_WIRELESS_SSID)).toString();
            kDebug() << "Hashing PSK. essid:" << essid << "psk:" << psk;
            QByteArray buffer(WPA_PMK_LEN * 2, 0);
            pbkdf2_sha1(psk.toLatin1(), essid.toLatin1(), essid.size(), 4096, (uint8_t*)buffer.data(), WPA_PMK_LEN);
            QString hexHash = buffer.toHex().left(WPA_PMK_LEN*2);
            kDebug() << "  hexadecimal key out:" << hexHash;
            retrievedSecrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK), hexHash);
        }
        //END HACK

        // update myself
        QVariantMap existingSetting = mSettingsMap.value(csj->settingName());
        QMapIterator<QString,QVariant> i(retrievedSecrets);
        while (i.hasNext()) {
            i.next();
            if (i.value().toString().isEmpty()) {
                kDebug() << "Warning: empty secret retrieved for key " << i.key();
            }
            existingSetting.insert(i.key(), i.value());
        }
        kDebug() << "Updating existing settings for " << csj->settingName() << existingSetting;
        mSettingsMap.insert(csj->settingName(), existingSetting);

        // setup reply
        QVariantMapMap replyOuterMap;
        replyOuterMap.insert(csj->settingName(), retrievedSecrets);
        kDebug() << "Final secrets map to send:" << replyOuterMap;
        QDBusMessage reply = csj->requestMessage().createReply();

        QVariant arg = QVariant::fromValue(replyOuterMap);
        reply << arg;
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::WalletDisabled ) {
        kDebug() << "ERROR: The KDE wallet is disabled";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SettingError"), "The wallet was disabled");
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::WalletNotFound ) {
        kDebug() << "ERROR: The wallet used by KNetworkManager was not found";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SettingError"), "The wallet was not found");
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::WalletOpenRefused ) {
        kDebug() << "ERROR: The user refused KNetworkManager permission to open the wallet";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SecretsRefused"), "User refused to supply secrets");
        QDBusConnection::systemBus().send(reply);
    } else if (csj->error() == ConnectionSecretsJob::UserInputCancelled ) {
        kDebug() << "ERROR: The user cancelled the get secrets dialog";
        QDBusMessage reply = csj->requestMessage().createErrorReply(QLatin1String("org.freedesktop.NetworkManager.SecretsRefused"), "User refused to supply secrets");
        QDBusConnection::systemBus().send(reply);
    }
}

QString Connection::uuid() const
{
    return mId;
}
#include "connection.moc"
