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

#ifndef KNM_INTERNALS_SECRETSTORAGE_H
#define KNM_INTERNALS_SECRETSTORAGE_H

#include <QObject>
#include <QHash>
#include <QtGui/qwindowdefs.h> // krazy:exclude=includes (for WId)

#include <KSharedConfig>

#include "knminternals_export.h"

#include "secretsprovider.h"

namespace Knm
{
class Connection;
class Setting;
}

class ConnectionSecretsJob;
class KJob;
class SecretStoragePrivate;

class KNMINTERNALS_EXPORT SecretStorage : public SecretsProvider
{
Q_OBJECT
Q_DECLARE_PRIVATE(SecretStorage)
public:
    enum SecretStorageMode { DontStore, PlainText, Secure};
    class EnumError
    {
    public:
        enum type {NoError = 0, MissingContents, WalletDisabled, WalletNotFound, WalletOpenRefused };
    };

    SecretStorage(QObject * parent = 0);

    virtual ~SecretStorage();

    Knm::Connection * connection() const;

    // get/set the window ID used for focus stealing prevention
    static void setWalletWid( WId wid ) {
        s_walletWId = wid;
    }

    static WId walletWid() {
        return s_walletWId;
    }

    void loadSecrets(Knm::Connection*, const QString &, GetSecretsFlags);
    void saveSecrets(Knm::Connection*);
    void deleteSecrets(Knm::Connection*);
    static bool switchStorage(SecretStorageMode, SecretStorageMode);
Q_SIGNALS:
    void connectionSaved(Knm::Connection*);
protected:
    SecretStoragePrivate *d_ptr;
private Q_SLOTS:
    void walletOpenedForRead(bool);
    void walletOpenedForWrite(bool);
    void gotSecrets(KJob*);
private:
    QString walletKeyFor(const QString &,const Knm::Setting *) const;
    static QString walletKeyFor(const QString &,const QString&);
    KSharedConfig::Ptr secretsFileForUuid(const QString &);
    void askUser(Knm::Connection*, const QString &, const QStringList &);

    static QString s_walletFolderName;
    static WId s_walletWId;
};

#endif // SECRETSTORAGE_H
