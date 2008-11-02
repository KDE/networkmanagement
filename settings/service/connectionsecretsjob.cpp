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

#include "connectionsecretsjob.h"

#include <QTimer>

#include <KDebug>

#include <kwallet.h>

#include "datamappings.h"

ConnectionSecretsJob::ConnectionSecretsJob(const QString & connectionId, const QString &settingName, const QStringList& secrets, bool requestNew,  QDBusMessage& reply)
    : mConnectionId(connectionId), mSettingName(settingName), mRequestNew(requestNew), mReply(reply)
{
    // record the secrets that we are looking for
    foreach (QString secretKey, secrets) {
        mSecrets.insert(secretKey, QVariant());
    }
}

ConnectionSecretsJob::~ConnectionSecretsJob()
{
}

void ConnectionSecretsJob::start()
{
    QTimer::singleShot(0, this, SLOT(doWork()));
}

void ConnectionSecretsJob::doWork()
{
    if (mRequestNew) {
        kDebug() << "FIXME: implement doAskUser()";
    }
    if (false/*mRequestNew*/) {
        doAskUser();
    } else {
        // do wallet lookup
        if (KWallet::Wallet::isEnabled()) {
            kDebug() << "opening wallet...";
            KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0/*WId*/, KWallet::Wallet::Asynchronous);
            if (wallet) {
                connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForRead(bool)));
            } else {
                setError(WalletNotFound);
                emitResult();
            }
        } else {
            setError(WalletDisabled);
            emitResult();
        }
    }
}

void ConnectionSecretsJob::doAskUser()
{
    // popup a dialog showing the appropriate UI for the type of connection
    kDebug();
}

void ConnectionSecretsJob::walletOpenedForRead(bool success)
{
    if (success) {
        // get the requested secrets, set our secrets, and emit result
        KWallet::Wallet * wallet = static_cast<KWallet::Wallet*>(sender());
        if (wallet->isOpen() && wallet->hasFolder("NetworkManager") && wallet->setFolder("NetworkManager")) {
            if (mSecrets.isEmpty()) {
                kDebug() << "Reading all entries for connection";
                QMap<QString,QString> entries;
                QString key = mConnectionId + ';' + mSettingName + ";*";
                if (wallet->readPasswordList(key, entries) == 0) {
                    kDebug() << "Got password list: " << entries;
                    DataMappings dm;
                    QMapIterator<QString,QString> i(entries);
                    while (i.hasNext()) {
                        i.next();
                        // the part that NM has asked for is the final part of the key used in
                        // kwallet
                        QString key = i.key().section(';', 2, 2);
                        mSecrets.insert(dm.convertKey(key), dm.convertValue(key, QString(i.value())));
                    }
                    emitResult();
                } else {
                    kDebug() << "Wallet::readEntryList for :" << key << " failed";
                }
            } else {
                kDebug() << "Reading requested entries from wallet: "<< mSecrets.keys();
                bool missingSecret = false;
                foreach (QString key, mSecrets.keys()) {
                    kDebug() << "Requesting password from wallet: " << key;
                    QString secret;
                    if (wallet->readPassword(keyForEntry(key), secret) == 0 ) {
                        kDebug() << "Got: " << key << " : " << secret;
                        mSecrets.insert(key, secret);
                    } else {
                        missingSecret = true;
                    }
                }
                if (missingSecret) {
                    kDebug() << "FIXME: missing secrets - implement doAskUser()";
                    //doAskUser();
                    emitResult();
                } else {
                    emitResult();
                }
            }
        }
    } else {
        setError(WalletOpenRefused);
        emitResult();
    }
}

void ConnectionSecretsJob::walletOpenedForWrite(bool success)
{

}

void ConnectionSecretsJob::dialogAccepted()
{

}

void ConnectionSecretsJob::dialogCancelled()
{

}
QString ConnectionSecretsJob::settingName() const
{
    return mSettingName;
}

QVariantMap ConnectionSecretsJob::secrets() const
{
    return mSecrets;
}

QDBusMessage ConnectionSecretsJob::message() const
{
    return mReply;
}

QString ConnectionSecretsJob::keyForEntry(const QString & entry) const
{
    return mConnectionId + ';' + mSettingName + ';' + entry;
}


#include "connectionsecretsjob.moc"
// vim: sw=4 sts=4 et tw=100
