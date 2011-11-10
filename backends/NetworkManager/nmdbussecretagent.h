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

#ifndef NMDBUSSECRETAGENT_H
#define NMDBUSSECRETAGENT_H

#include "nm-agent-manager.h"
#include "nm-secret-agentadaptor.h"
#include "knm_export.h"

#include <QObject>
#include <QDBusContext>
#include <QDBusServiceWatcher>

namespace Knm
{
    class Connection;
    class Secrets;
    class Setting;
}

class SecretsProvider;
class NMDBusSecretAgentPrivate;

class KNM_EXPORT NMDBusSecretAgent : public QObject, protected QDBusContext
{
Q_OBJECT
Q_DECLARE_PRIVATE(NMDBusSecretAgent)
public:
    NMDBusSecretAgent(QObject * parent = 0);
    virtual ~NMDBusSecretAgent();
    void registerSecretsProvider(SecretsProvider *);
public Q_SLOTS:
    QVariantMapMap GetSecrets(const QVariantMapMap&, const QDBusObjectPath&, const QString&, const QStringList&, uint);
    void SaveSecrets(const QVariantMapMap&, const QDBusObjectPath&);
    void DeleteSecrets(const QVariantMapMap &, const QDBusObjectPath &);
    void CancelGetSecrets(const QDBusObjectPath &, const QString &);
protected:
    NMDBusSecretAgentPrivate *d_ptr;
private:
    void loadSecrets(Knm::Secrets*);

private Q_SLOTS:
    void secretsReady(Knm::Connection *, const QString &, bool, bool);
    void deleteSavedConnection(Knm::Connection *);
    void registerAgent();
};

#endif // NMDBUSSECRETAGENT_H
