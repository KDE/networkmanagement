/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#include "secrets.h"

class SecretStoragePrivate
{
public:
    SecretStorage::SecretStorageMode storageMode;
    QList<Knm::Connection*> connectionsToWrite;
    QList<Knm::Connection*> connectionsToRead;
    QMultiHash<QString,QPair<QString,SecretsProvider::GetSecretsFlags> > settingsToRead;
};


QString SecretStorage::s_walletFolderName = QLatin1String("Network Management");

WId SecretStorage::s_walletWId = 0;

SecretStorage::SecretStorage()
    :SecretsProvider(), d_ptr(new SecretStoragePrivate())
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
            Knm::Secrets * secrets = setting->getSecretsObject();
            if (secrets) {
                QMap<QString,QString> map = secrets->secretsToMap();
                if (ptr)
                    secrets->secretsToConfig(map, ptr);
            }
        }
    } else if (d->storageMode == Secure) {
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Asynchronous );
        if (wallet) {
            connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForWrite(bool)));
            d->connectionsToWrite.append(con);
        }
    }
}

void SecretStorage::walletOpenedForWrite(bool success)
{
    Q_D(SecretStorage);
    if (success) {
        KWallet::Wallet * wallet = static_cast<KWallet::Wallet*>(sender());
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
                        if (k.startsWith(con->uuid() + ';'))
                            wallet->removeEntry(k);
                    }
                    foreach (Knm::Setting * setting, con->settings()) {
                        Knm::Secrets * secrets = setting->getSecretsObject();
                        if (secrets) {
                            QMap<QString,QString> map = secrets->secretsToMap();
                            if (!map.isEmpty()) {
                                saved = true;
                                wallet->writeMap(walletKeyFor(con->uuid(), setting), map);
                            }
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
}

void SecretStorage::walletOpenedForRead(bool success)
{
    Q_D(SecretStorage);
    bool retrievalSuccessful = true;
    if (success) {
        KWallet::Wallet * wallet = static_cast<KWallet::Wallet*>(sender());
        if (wallet->isOpen() && wallet->hasFolder(s_walletFolderName) && wallet->setFolder(s_walletFolderName)) {
            while (!d->connectionsToRead.isEmpty()) {
                Knm::Connection *con = d->connectionsToRead.takeFirst();
                QMutableHashIterator<QString, QPair<QString,GetSecretsFlags> > i(d->settingsToRead);
                while (i.hasNext()) {
                    if (i.next().key() != con->uuid())
                        continue;

                    QPair<QString,GetSecretsFlags> pair = i.value();
                    Knm::Secrets * secrets = 0;
                    bool settingsFound = false;
                    foreach (Knm::Setting * setting, con->settings()) {
                        if (setting->name() == pair.first) {
                            secrets = setting->getSecretsObject();
                            settingsFound = true;
                            if (secrets) {
                                QMap<QString,QString> map;
                                if (wallet->readMap(walletKeyFor(con->uuid(), setting), map) == 0) {
                                    secrets->secretsFromMap(map);
                                }
                                QStringList needSecretsList = secrets->needSecrets();
                                if (!needSecretsList.isEmpty() && (pair.second & AllowInteraction || pair.second & RequestNew)) {
                                    askUser(con, pair.first, needSecretsList);
                                } else {
                                    emit connectionRead(con, pair.first);
                                }
                            } else {
                                emit connectionRead(con, pair.first);
                            }
                            break;
                        }
                    }
                    if (!settingsFound) {
                        emit connectionRead(con, pair.first);
                    }
                    i.remove();
                }
            }
        } else {
            retrievalSuccessful = false;
        }
    }
    if (!retrievalSuccessful || !success) {
         while (!d->connectionsToRead.isEmpty()) {
            Knm::Connection *con = d->connectionsToRead.takeFirst();
            QMutableHashIterator<QString, QPair<QString,GetSecretsFlags> > i(d->settingsToRead);
            while (i.hasNext()) {
                if (i.next().key() != con->uuid())
                    continue;
                QPair<QString,GetSecretsFlags> pair = i.value();
                emit connectionRead(con, pair.first);
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
    if (!con->hasSecrets()) {
        return;
    }

    if (d->storageMode == PlainText) {
        KSharedConfig::Ptr ptr = secretsFileForUuid(con->uuid());
        QFile::remove(ptr->name());
    } else if (d->storageMode == Secure) {
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Synchronous );
        if( wallet && wallet->isOpen() && wallet->hasFolder( s_walletFolderName ) && wallet->setFolder( s_walletFolderName )) {
            foreach (const QString & k, wallet->entryList()) {
                if (k.startsWith(con->uuid() + ';'))
                    wallet->removeEntry(k);
            }
        }
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

    if (d->storageMode == PlainText && !(flags & RequestNew)) {
        Knm::Secrets * secrets = 0;
        foreach (Knm::Setting * setting, con->settings()) {
            if (setting->name() == name) {
                secrets = setting->getSecretsObject();
                break;
            }
        }
        KSharedConfig::Ptr ptr = secretsFileForUuid(uuid);
        if (secrets) {
            QMap<QString,QString> map;
            if (ptr)
                map = secrets->secretsFromConfig(ptr);
            secrets->secretsFromMap(map);
            QStringList needSecretsList = secrets->needSecrets();
            if (!needSecretsList.isEmpty() && (flags & AllowInteraction || flags & RequestNew)) {
                askUser(con, name, needSecretsList);
            } else {
                emit connectionRead(con, name);
            }
        } else {
            emit connectionRead(con, name);
        }
    } else if (d->storageMode == Secure && !(flags & RequestNew)) {
        kDebug() << "opening wallet...";
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
                walletWid(),KWallet::Wallet::Asynchronous);
        if (wallet) {
            connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForRead(bool)));
            d->connectionsToRead.append(con);
            QPair<QString,GetSecretsFlags> pair(name, flags);
            d->settingsToRead.insert(uuid, pair);
        } else {
            emit connectionRead(con, name);
        }
    }
}

void SecretStorage::askUser(Knm::Connection *con, const QString & name, const QStringList &secrets)
{
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
        config = KSharedConfig::openConfig(configFile, KConfig::NoGlobals);
        if (config.isNull()) {
            kDebug() << "Config not found at" << configFile;
        }
    }
    return config;
}

void SecretStorage::gotSecrets(KJob *job)
{
    ConnectionSecretsJob * csj = static_cast<ConnectionSecretsJob*>(job);
    emit connectionRead(csj->connection(), csj->settingName());
}

void SecretStorage::switchStorage(SecretStorageMode oldMode, SecretStorageMode newMode)
{
    // TODO: integrate DontStore with NM0.9 secret flags
    if (oldMode == DontStore || newMode == DontStore)
        return;

    KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
        walletWid(),KWallet::Wallet::Synchronous);
    if (!wallet)
        return;
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
            foreach (const QString &secret, secrets.keys()) {
                configGroup.writeEntry(secret, secrets.value(secret));
            }
            wallet->removeEntry(key);
        }
    }
}