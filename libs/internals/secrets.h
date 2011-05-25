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

#ifndef KNM_INTERNALS_SECRETS_H
#define KNM_INTERNALS_SECRETS_H

#include "knminternals_export.h"

#include <QObject>
#include <QMap>

#include <KSharedConfig>

class QStringList;

namespace Knm
{
class Setting;

class KNMINTERNALS_EXPORT Secrets : public QObject
{
public:
    enum SecretStorageMode { DontStore, PlainText, Secure};

    Secrets(Setting *);
    Secrets(Secrets *, Setting *);
    virtual ~Secrets();
    virtual QMap<QString,QString> secretsToMap() const;
    virtual void secretsFromMap(QMap<QString,QString>) const;
    virtual void secretsToConfig(QMap<QString,QString>, KSharedConfig::Ptr) const;
    virtual QMap<QString,QString> secretsFromConfig(KSharedConfig::Ptr);
    virtual QStringList needSecrets();
protected:
    Setting *m_setting;
};

} // namespace Knm

#endif // SECRETS_H
