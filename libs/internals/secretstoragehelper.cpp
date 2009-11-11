/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#include "secretstoragehelper.h"

#include <KDebug>
#include <kwallet.h>

QString SecretStorageHelper::s_walletFolderName = QLatin1String("NetworkManager");
WId SecretStorageHelper::s_walletWId = 0;

SecretStorageHelper::SecretStorageHelper( const QString &connectionName, const QString &settingGroup) : m_connectionName(connectionName), m_settingGroup(settingGroup)
{
}

QString SecretStorageHelper::keyForEntry(const QString & entry) const
{
    return m_connectionName + ';' + m_settingGroup + ';' + entry;
}

void SecretStorageHelper::readSecret(const QString &key, QString &secret )
{
    kDebug() << key << secret;
    KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Synchronous );
    if( wallet && wallet->isOpen() ) {
        if( !wallet->hasFolder( s_walletFolderName ) )
            wallet->createFolder( s_walletFolderName );

        if ( wallet->setFolder( s_walletFolderName ) ) {
            if (wallet->readPassword(keyForEntry(key), secret) == 0 ) {
                kDebug() << "Got secret '" << keyForEntry(key) << "' : '" << secret << "' in folder " << s_walletFolderName;
                return;
            }
        }
    }
    kDebug() << "Failed to get secret '" << keyForEntry(key) << "' in folder " << s_walletFolderName;
    secret.clear();
}

void SecretStorageHelper::writeSecret(const QString &key, const QString &secret )
{
    kDebug() << key << secret;
    KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Synchronous );
    if( wallet && wallet->isOpen() ) {
        if( !wallet->hasFolder( s_walletFolderName ) )
            wallet->createFolder( s_walletFolderName );

        if ( wallet->setFolder( s_walletFolderName ) ) {
            wallet->writePassword(keyForEntry(key), secret);
        }
    }
}

// vim: sw=4 sts=4 et tw=100
