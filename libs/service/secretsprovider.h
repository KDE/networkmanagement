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

#ifndef SECRETSPROVIDER_H
#define SECRETSPROVIDER_H

#include "knm_export.h"

#include <QObject>
#include <QString>

namespace Knm
{
    class Connection;
} // namespace Knm

/**
 * Interface of an object that can provide secrets
 * Implementations that ignore an operation should provide an empty implementation of the pure
 * virtual methods
 */
class KNM_EXPORT SecretsProvider : public QObject
{
    Q_OBJECT
    public:
        Q_FLAGS(GetSecretsFlags)
        enum GetSecretsFlag { None = 0, AllowInteraction = 0x01, RequestNew = 0x02};
        Q_DECLARE_FLAGS(GetSecretsFlags, GetSecretsFlag)

        SecretsProvider(QObject * parent);
        virtual ~SecretsProvider();
        virtual void loadSecrets(Knm::Connection *, const QString &, GetSecretsFlags) = 0;
        virtual void saveSecrets(Knm::Connection *) = 0;
        virtual void deleteSecrets(Knm::Connection *) = 0;
    Q_SIGNALS:
        void connectionSaved(Knm::Connection *);
        void connectionRead(Knm::Connection *con, const QString& name, bool failed, bool needsSaving);
};

#endif // SECRETSPROVIDER_H
