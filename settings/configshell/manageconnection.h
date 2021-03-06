/*
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

#ifndef CONFIGSHELL_MANAGE_CONNECTION_H
#define CONFIGSHELL_MANAGE_CONNECTION_H

#include <QDBusInterface>

#include <KApplication>

#include "nmdbussettingsconnectionprovider.h"
#include "connectioneditor.h"

class ManageConnection: public QObject
{
Q_OBJECT
public:
    ManageConnection(Knm::Connection *con);
    ~ManageConnection();

    static void saveConnection(Knm::Connection *con);
private Q_SLOTS:
    void addConnectionCompleted(bool valid, const QString &errorMessage);
    void updateConnectionCompleted();
    void activatableAdded(const QVariantMap &properties);
private:
    NMDBusSettingsConnectionProvider * mSystemSettings;
    QDBusInterface m_manager;
};
#endif
