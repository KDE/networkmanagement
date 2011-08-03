/*
Copyright 2011 Lamarque V. Souza <lamarque@gmail.com>

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

#ifndef IMPORTNM08CONNECTIONS_H
#define IMPORTNM08CONNECTIONS_H

#include <QStringList>

#include <secretstorage.h>
#include <nmdbussettingsconnectionprovider.h>

#include "settingsnm08/settingpersistence.h"

namespace Knm
{
    class Connection;
}

class Nm08Connections: public QObject
{
Q_OBJECT
public:
    Nm08Connections(SecretStorage * secretStorage, NMDBusSettingsConnectionProvider * nmDBusConnectionProvider, QObject * parent = 0);
    ~Nm08Connections();

public Q_SLOTS:
    void importNextNm08Connection();

private Q_SLOTS:
    void gotSecrets(Knm::Connection *, const QString&, bool);

private:
    Knm::SettingPersistence * persistenceFor(Knm::Setting * setting, KSharedConfig::Ptr config);
    QHash<Knm::Setting*, Knm::SettingPersistence*> m_persistences;
    SecretStorage::SecretStorageMode m_storageMode;
    QStringList m_connectionsToImport;
    SecretStorage * m_secretStorage;
    NMDBusSettingsConnectionProvider * m_nmDBusConnectionProvider;
    QList<Knm::Connection *> m_connectionsToDelete;
    QList<Knm::Connection *> m_connectionsBeingAdded;
};

#endif // IMPORTNM08CONNECTIONS_H
