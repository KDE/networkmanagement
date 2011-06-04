/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#include <KDebug>

#include "manageconnection.h"

ManageConnection::ManageConnection(Knm::Connection *con): mSystemSettings(0)
{
    NMDBusSettingsConnectionProvider * mSystemSettings = new NMDBusSettingsConnectionProvider(0, 0);
    connect(mSystemSettings, SIGNAL(addConnectionCompleted(bool, const QString &)), SLOT(addConnectionCompleted(bool, const QString &)));
    mSystemSettings->addConnection(con);
    ConnectionEditor editor(0);
}

ManageConnection::~ManageConnection()
{
    if (mSystemSettings) {
        delete mSystemSettings;
    }
}

void ManageConnection::saveConnection(Knm::Connection *con)
{
    ManageConnection *ptr = new ManageConnection(con);
}

void ManageConnection::addConnectionCompleted(bool valid, const QString &errorMessage)
{
    kDebug(KDE_DEFAULT_DEBUG_AREA);
    deleteLater();
    kapp->quit();
}
