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

#ifndef SETTINGS_KCONFIGTOSERVICE_H
#define SETTINGS_KCONFIGTOSERVICE_H

#include <QObject>
#include <QMap>
#include <QPair>
#include <QString>
#include <QVariant>

#include <KSharedConfig>

#include "marshalarguments.h"
#include <QtGui/qwindowdefs.h> // krazy:exclude=includes (for WId)

/**
 * This class extracts stored connections and puts them on the settings service
 * 1) get the names of all the connections from the main config file
 * 2) open each connection's file and create 1 or more ConfigXml for it
 * for each group, create a QVariantMap containing its settings and store them in the master
 * connection map (maybe exclude any secrets)
 * check key for key name conversion
 *   check value for any value conversions needed
 * Secrets?
 * Update connection?  Get a signal on new or updated connections, read file and update
 * NetworkSettings
 * Remove connection?  This can be done in NetworkSettings
 */
class DataMappings;
class NetworkSettings;

class KConfigToService : public QObject
{
Q_OBJECT
public:
    KConfigToService(NetworkSettings * service, bool active);
    ~KConfigToService();
    void init();
    void addOrUpdate(const QString & id);
    QList<QPair<const QString&, const QString&> > keyMappings() const;
    void configure(const QStringList & changedConnections);
    void start(WId wid);
    void stop();
public Q_SLOTS:
    void connectionActivated(const QString &uuid);
private:
    // map from a) keys that have been munged to be legal variable names
    // to b) actual networkmanager parameter keys
    QString convertKey(const QString &) const;
    // utility method to do last minute value conversions, if required
    QVariant convertValue(const QString& key, const QVariant& value) const;
    // restore the given connection from storage to a map
    QVariantMapMap restoreConnection(const QString & connectionId);
    // deserialize a single settings group
    QVariantMap handleGroup(const QString & name);
private:
    NetworkSettings * m_service;
    QMap<QString, QString> m_connectionIdToObjectPath;
    KSharedConfigPtr m_config;
    QString m_currentConnectionType;
    bool m_error;
    DataMappings * m_dataMappings;
    bool m_init;
};

#endif // SETTINGS_KCONFIGTOSERVICE_H
