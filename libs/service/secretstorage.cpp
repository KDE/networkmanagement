/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

#include "secretstorage.h"

#include <KConfigGroup>
#include <kwallet.h>
#include <kstandarddirs.h>
#include <KDebug>

#include <QFile>
#include <QDir>
#include <QMutableHashIterator>

#include "paths.h"

#include "ui/connectionsecretsjob.h"

#include "knmserviceprefs.h"
#include "connection.h"
#include "setting.h"

class SecretStoragePrivate
{
public:
    SecretStorage::SecretStorageMode storageMode;
    QList<Knm::Connection*> connectionsToWrite;
    QList<Knm::Connection*> connectionsToRead;
    QMultiHash<QString,QPair<QString,SecretsProvider::GetSecretsFlags> > settingsToRead;
    QList<QString> userAskRequestsInProgress;
};


QString SecretStorage::s_walletFolderName = QLatin1String("Network Management");

WId SecretStorage::s_walletWId = 0;

SecretStorage::SecretStorage(QObject * parent)
    :SecretsProvider(parent), d_ptr(new SecretStoragePrivate())
{
    Q_D(SecretStorage);
    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);
    KNetworkManagerServicePrefs::self()->readConfig();
    d->storageMode = (SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();
}

SecretStorage::~SecretStorage()
{
    Q_D(SecretStorage);
    delete d;
}

void SecretStorage::saveSecrets(Knm::Connection *con)
{
    Q_D(SecretStorage);
    KNetworkManagerServicePrefs::self()->readConfig();
    d->storageMode = (SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();

    if (d->storageMode == PlainText) {
        KSharedConfig::Ptr ptr = secretsFileForUuid(con->uuid());
        foreach (Knm::Setting * setting, con->settings()) {
            QMap<QString, QString> secrets = setting->secretsToMap();
            KConfigGroup config(ptr, Knm::Setting::typeAsString(setting->type()));
            config.deleteGroup();
            if (!secrets.isEmpty()) {
                foreach (const QString &secret, secrets.keys()) {
                    config.writeEntry(secret, secrets.value(secret));
                }
            }
        }
    } else if (d->storageMode == Secure) {
        if (!KWallet::Wallet::isEnabled()) {
            kWarning() << "KWallet is disabled, please enable it or change Plasma NM to use 'In file' storage. Secrets not saved.";
            return;
        }

        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Asynchronous );
        Q_ASSERT(wallet);

        if (!wallet) {
            kWarning() << "Error opening kwallet. Secrets not saved.";
            return;
        }

        connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForWrite(bool)));
        d->connectionsToWrite.append(con);
    }
}

void SecretStorage::walletOpenedForWrite(bool success)
{
    Q_D(SecretStorage);
    KWallet::Wallet * wallet = qobject_cast<KWallet::Wallet*>(sender());
    Q_ASSERT(wallet);

    if (success) {
        if (wallet->isOpen()) {
            bool readyForWalletWrite = false;
            if( !wallet->hasFolder( s_walletFolderName ) )
                wallet->createFolder( s_walletFolderName );
            if ( wallet->setFolder( s_walletFolderName ) ) {
                readyForWalletWrite = true;
            }
            if (readyForWalletWrite) {
                while (!d->connectionsToWrite.isEmpty()) {
                    Knm::Connection *con = d->connectionsToWrite.takeFirst();
                    bool saved = false;
                    foreach (const QString & k, wallet->entryList()) {
                        if (k.startsWith(con->uuid() + ';')) {
                            kDebug() << "Removing entry " << k << ")";
                            wallet->removeEntry(k);
                        }
                    }
                    foreach (Knm::Setting * setting, con->settings()) {
                        QMap<QString,QString> map = setting->secretsToMap();
                        if (!map.isEmpty()) {
                            saved = true;
                            wallet->writeMap(walletKeyFor(con->uuid(), setting), map);
                            kDebug() << "Writing entry " << walletKeyFor(con->uuid(), setting);
                        }
                    }
                    if (!saved) {
                        kWarning() << "No secret has been written to the kwallet.";
                    }
                    emit connectionSaved(con);
                }
            }
        }
    }
    wallet->deleteLater();
}

void SecretStorage::walletOpenedForRead(bool success)
{
    Q_D(SecretStorage);
    kDebug();
    bool retrievalSuccessful = true;
    KWallet::Wallet * wallet = static_cast<KWallet::Wallet*>(sender());
    Q_ASSERT(wallet);

    if (success) {
        if (wallet->isOpen() && wallet->hasFolder(s_walletFolderName) && wallet->setFolder(s_walletFolderName)) {
            while (!d->connectionsToRead.isEmpty()) {
                Knm::Connection *con = d->connectionsToRead.takeFirst();
                QMutableHashIterator<QString, QPair<QString,GetSecretsFlags> > i(d->settingsToRead);
                while (i.hasNext()) {
                    if (i.next().key() != con->uuid())
                        continue;

                    QPair<QString,GetSecretsFlags> pair = i.value();
                    bool settingsFound = false;
                    foreach (Knm::Setting * setting, con->settings()) {
                        if (!setting) {
                            kWarning() << "Setting for " << con->uuid() << " is null. That should not happen.";
                            continue;
                        }
                        if (setting->name() == pair.first) {
                            settingsFound = true;
                            QMap<QString,QString> map;
                            if (wallet->readMap(walletKeyFor(con->uuid(), setting), map) == 0) {
                                setting->secretsFromMap(map);
                            }
                            QStringList needSecretsList = setting->needSecrets(pair.second & RequestNew);
                            kDebug() << "Needed secrets" << needSecretsList;
                            if ((pair.second & RequestNew) || (!needSecretsList.isEmpty() && (pair.second & AllowInteraction))) {
                                askUser(con, pair.first, needSecretsList);
                            } else {
                                emit connectionRead(con, pair.first, false, false);
                            }
                            break;
                        }
                    }
                    if (!settingsFound) {
                        emit connectionRead(con, pair.first, true, false);
                    }
                    i.remove();
                }
            }
        } else {
            retrievalSuccessful = false;
        }
    }

    if (wallet) {
        wallet->deleteLater();
    }

    if (!retrievalSuccessful || !success) {
         while (!d->connectionsToRead.isEmpty()) {
            Knm::Connection *con = d->connectionsToRead.takeFirst();
            QMutableHashIterator<QString, QPair<QString,GetSecretsFlags> > i(d->settingsToRead);
            while (i.hasNext()) {
                if (i.next().key() != con->uuid())
                    continue;
                QPair<QString,GetSecretsFlags> pair = i.value();
                emit connectionRead(con, pair.first, true, false);
                i.remove();
            }
         }
    }
}

void SecretStorage::deleteSecrets(Knm::Connection *con)
{
    Q_D(SecretStorage);
    KNetworkManagerServicePrefs::self()->readConfig();
    d->storageMode = (SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();
    if (!con->hasPersistentSecrets()) {
        return;
    }

    if (d->storageMode == PlainText) {
        KSharedConfig::Ptr ptr = secretsFileForUuid(con->uuid());
        QFile::remove(ptr->name());
    } else if (d->storageMode == Secure) {
        if (!KWallet::Wallet::isEnabled()) {
            kWarning() << "KWallet is disabled, please enable it. Secrets not deleted.";
            return;
        }

        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Synchronous );
        Q_ASSERT(wallet);

        if (!wallet) {
            kWarning() << "Error opening kwallet. Secrets not deleted.";
            return;
        }

        if( wallet->isOpen() && wallet->hasFolder( s_walletFolderName ) && wallet->setFolder( s_walletFolderName )) {
            foreach (const QString & k, wallet->entryList()) {
                if (k.startsWith(con->uuid() + ';'))
                    wallet->removeEntry(k);
            }
        }
        wallet->deleteLater();
    }
}

QString SecretStorage::walletKeyFor(const QString &uuid, const Knm::Setting * setting) const
{
    return uuid + ';' + setting->name();
}

QString SecretStorage::walletKeyFor(const QString &uuid, const QString &name)
{
    return uuid + ';' + name;
}

void SecretStorage::loadSecrets(Knm::Connection *con, const QString &name, GetSecretsFlags flags)
{
    Q_D(SecretStorage);
    KNetworkManagerServicePrefs::self()->readConfig();
    d->storageMode = (SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();

    QString uuid = con->uuid();

    if (d->storageMode == PlainText) {
        KSharedConfig::Ptr ptr = secretsFileForUuid(uuid);
        KConfigGroup config(ptr, name);
        QMap<QString,QString> map = config.entryMap();
        Knm::Setting *setting = con->setting(Knm::Setting::typeFromString(name));
        setting->secretsFromMap(map);
        QStringList needSecretsList = setting->needSecrets(flags & RequestNew);
        if ((flags & RequestNew) || (!needSecretsList.isEmpty() && (flags & AllowInteraction))) {
            askUser(con, name, needSecretsList);
        } else {
            emit connectionRead(con, name, false, false);
        }
    } else if (d->storageMode == Secure) {
        if (!KWallet::Wallet::isEnabled()) {
            kWarning() << "KWallet is disabled, please enable it. Secrets not loaded.";
            return;
        }

        kDebug() << "opening wallet...";
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
                walletWid(),KWallet::Wallet::Asynchronous);
        Q_ASSERT(wallet);

        // remove stale request to prevent crashes.
        // https://bugs.kde.org/show_bug.cgi?id=283105
        QMutableListIterator<Knm::Connection *> conIter(d->connectionsToRead);
        while (conIter.hasNext()) {
            Knm::Connection * c = conIter.next();
            if (c->uuid() == con->uuid()) {
                QMutableHashIterator<QString, QPair<QString,GetSecretsFlags> > i(d->settingsToRead);
                bool deleteConnection = false;
                while (i.hasNext()) {
                    i.next();
                    if (i.key() != c->uuid() || i.value().first != name)
                        continue;
                    QPair<QString,GetSecretsFlags> pair = i.value();
                    i.remove();
                    deleteConnection = true;
                    kDebug() << "Removing stale request" << c->uuid() << pair.first;
                }
                // the same connection may contain secrets in different settings,
                // delete the connection only if there is no more secrets to read from it.
                if (deleteConnection && !d->settingsToRead.contains(con->uuid())) {
                    delete c;
                    conIter.remove();
                }
            }
        }

        d->connectionsToRead.append(con);
        QPair<QString,GetSecretsFlags> pair(name, flags);
        d->settingsToRead.insert(uuid, pair);

        if (wallet) {
            connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForRead(bool)));
        } else {
            kWarning() << "Error opening kwallet. Secrets not loaded.";

            // emit connectionRead() signal to indicate operation has failed.
            walletOpenedForRead(false);
        }
    }
}

void SecretStorage::askUser(Knm::Connection *con, const QString & name, const QStringList &secrets)
{
    Q_D(SecretStorage);
    if (d->userAskRequestsInProgress.contains(con->uuid().toString())) {
        return;
    }
    d->userAskRequestsInProgress.append(con->uuid().toString());
    ConnectionSecretsJob *job = new ConnectionSecretsJob(con, name, secrets);
    connect(job, SIGNAL(finished(KJob*)), this, SLOT(gotSecrets(KJob*)));
    job->start();
}

KSharedConfig::Ptr SecretStorage::secretsFileForUuid(const QString & uuid)
{
    KSharedConfig::Ptr config;
    if (!uuid.isEmpty()) {
        QString configFile;
        configFile = KStandardDirs::locateLocal("data", Knm::SECRETS_PERSISTENCE_PATH + uuid);

        kDebug() << "configFile:" << configFile;
        config = KSharedConfig::openConfig(configFile, KConfig::SimpleConfig);
        if (config.isNull()) {
            kDebug() << "Config not found at" << configFile;
        }
    }
    return config;
}

void SecretStorage::gotSecrets(KJob *job)
{
    Q_D(SecretStorage);
    ConnectionSecretsJob * csj = static_cast<ConnectionSecretsJob*>(job);
    bool failed = true;
    if (csj->error() == ConnectionSecretsJob::EnumError::NoError) {
        failed = false;
    }
    Knm::Connection * con = csj->connection();

    if (con) {
        d->userAskRequestsInProgress.removeAll(con->uuid().toString());
    }
    emit connectionRead(con, csj->settingName(), failed, true);
}

bool SecretStorage::switchStorage(SecretStorageMode oldMode, SecretStorageMode newMode)
{
    // TODO: integrate DontStore with NM0.9 secret flags
    if (oldMode == DontStore || newMode == DontStore) {
        return true;
    }

    if (!KWallet::Wallet::isEnabled()) {
        kWarning() << "KWallet is disabled, please enable it. Storage mode not changed.";
        return false;
    }

    KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
        walletWid(),KWallet::Wallet::Synchronous);
    Q_ASSERT(wallet);

    if (!wallet) {
        kWarning() << "Error opening kwallet.";
        return false;
    }

    if( !wallet->hasFolder( s_walletFolderName ) )
        wallet->createFolder( s_walletFolderName );
    wallet->setFolder( s_walletFolderName );
    QString secretsDirectory = KStandardDirs::locateLocal("data", Knm::SECRETS_PERSISTENCE_PATH);

    if (oldMode == PlainText && newMode == Secure) {
        QDir dir(secretsDirectory);
        foreach (const QString &file, dir.entryList()) {
            KSharedConfig::Ptr config = KSharedConfig::openConfig(secretsDirectory + file, KConfig::SimpleConfig);
            foreach (const QString &group, config->groupList()) {
                KConfigGroup configGroup(config, group);
                wallet->writeMap(walletKeyFor(file, group), configGroup.entryMap());
            }
            QFile::remove(secretsDirectory + file);
        }
    } else if (oldMode == Secure && newMode == PlainText) {
        foreach (const QString &key, wallet->entryList()) {
            QStringList parts = key.split(";");
            KSharedConfig::Ptr config = KSharedConfig::openConfig(secretsDirectory + parts[0], KConfig::SimpleConfig);
            KConfigGroup configGroup(config, parts[1]);
            QMap<QString, QString> secrets;
            wallet->readMap(key, secrets);
            QMap<QString, QString>::ConstIterator it = secrets.constBegin();
            for ( ; it != secrets.constEnd(); ++it) {
                configGroup.writeEntry(it.key(), it.value());
            }
            wallet->removeEntry(key);
        }
    }
    wallet->deleteLater();
    return true;
}
