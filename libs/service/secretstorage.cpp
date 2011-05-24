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
#include <QMutableHashIterator>

#include "paths.h"

#include "ui/connectionsecretsjob.h"

#include "knmserviceprefs.h"
#include "connection.h"
#include "setting.h"

#include "secrets.h"

QString SecretStorage::s_walletFolderName = QLatin1String("Network Management");

WId SecretStorage::s_walletWId = 0;

SecretStorage::SecretStorage()
    :SecretsProvider()
{
    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);
    KNetworkManagerServicePrefs::self()->config()->reparseConfiguration();
    m_storageMode = (SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();
}

SecretStorage::~SecretStorage()
{
}

void SecretStorage::saveSecrets(Knm::Connection *con)
{
    KNetworkManagerServicePrefs::self()->config()->reparseConfiguration();
    m_storageMode = (SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();

    if (m_storageMode == PlainText) {
        KSharedConfig::Ptr ptr = secretsFileForUuid(con->uuid());
        foreach (Knm::Setting * setting, con->settings()) {
            Knm::Secrets * secrets = setting->getSecretsObject();
            if (secrets) {
                QMap<QString,QString> map = secrets->secretsToMap();
                if (ptr)
                    secrets->secretsToConfig(map, ptr);
            }
        }
    } else if (m_storageMode == Secure) {
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Asynchronous );
        if (wallet) {
            connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForWrite(bool)));
            m_connectionsToWrite.append(con);
        }
    }
}

void SecretStorage::walletOpenedForWrite(bool success)
{
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
                while (!m_connectionsToWrite.isEmpty()) {
                    Knm::Connection *con = m_connectionsToWrite.takeFirst();
                    bool saved = false;
                    foreach (Knm::Setting * setting, con->settings()) {
                        Knm::Secrets * secrets = setting->getSecretsObject();
                        if (secrets) {
                            QMap<QString,QString> map = secrets->secretsToMap();
                            // Do not save pin.
                            map.take("pin");
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
    bool retrievalSuccessful = true;
    if (success) {
        KWallet::Wallet * wallet = static_cast<KWallet::Wallet*>(sender());
        if (wallet->isOpen() && wallet->hasFolder(s_walletFolderName) && wallet->setFolder(s_walletFolderName)) {
            bool readyForWalletRead = false;
            if ( wallet->setFolder( s_walletFolderName ) ) {
                readyForWalletRead = true;
            }
            if (readyForWalletRead) {
                while (!m_connectionsToRead.isEmpty()) {
                    Knm::Connection *con = m_connectionsToRead.takeFirst();
                    QMutableHashIterator<QString, QPair<QString,GetSecretsFlags> > i(m_settingsToRead);
                    while (i.hasNext() && i.next().key() == con->uuid()) {
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
                        if (!settingsFound)
                            emit connectionRead(con, pair.first);
                        m_settingsToRead.remove(i.key(), i.value());
                    }
                }
            } else {
               retrievalSuccessful = false;
            }
        }
    }
    if (!retrievalSuccessful || !success) {
         while (!m_connectionsToRead.isEmpty()) {
            Knm::Connection *con = m_connectionsToRead.takeFirst();
            QMultiHash<QString, QPair<QString,GetSecretsFlags> >::iterator i = m_settingsToRead.find(con->uuid());
            while (i != m_settingsToRead.end() && i.key() == con->uuid()) {
                QPair<QString,GetSecretsFlags> pair = i.value();
                emit connectionRead(con, pair.first);
                m_settingsToRead.remove(i.key(), i.value());
            }
         }
    }
}

void SecretStorage::deleteSecrets(Knm::Connection *con)
{
    if (m_storageMode == PlainText) {
        KSharedConfig::Ptr ptr = secretsFileForUuid(con->uuid());
        QFile::remove(ptr->name());
    } else if (m_storageMode == Secure) {
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

QString SecretStorage::walletKeyFor(const QString &uuid, const QString &name) const
{
    return uuid + ';' + name;
}

void SecretStorage::loadSecrets(Knm::Connection *con, const QString &name, GetSecretsFlags flags)
{
    KNetworkManagerServicePrefs::self()->config()->reparseConfiguration();
    m_storageMode = (SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode();

    QString uuid = con->uuid();

    if (m_storageMode == PlainText && !(flags & RequestNew)) {
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
    } else if (m_storageMode == Secure && !(flags & RequestNew)) {
        kDebug() << "opening wallet...";
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
                walletWid(),KWallet::Wallet::Asynchronous);
        if (wallet) {
            connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForRead(bool)));
            m_connectionsToRead.append(con);
            QPair<QString,GetSecretsFlags> pair(name, flags);
            m_settingsToRead.insert(uuid, pair);
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
