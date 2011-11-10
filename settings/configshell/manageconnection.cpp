/*
Copyright 2011 Lamarque Souza <lamarque@kde.org>

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

#include <QTimer>
#include <QDBusReply>

#include <KDebug>
#include <KLocale>
#include <KMessageBox>

#include "manageconnection.h"
#include "settings/bluetooth.h"

ManageConnection::ManageConnection(Knm::Connection *con): m_manager("org.kde.networkmanagement",
                                                                    "/org/kde/networkmanagement",
                                                                    "org.kde.networkmanagement",
                                                                    QDBusConnection::sessionBus())
{
    bool addConnection = true;
    mSystemSettings = new NMDBusSettingsConnectionProvider(0, 0);
    Knm::BluetoothSetting * btSetting = static_cast<Knm::BluetoothSetting *>(con->setting(Knm::Setting::Bluetooth));

    if (btSetting) {
        foreach(const QString & connectionId, mSystemSettings->connectionList()->connections()) {
            Knm::Connection * c = mSystemSettings->connectionList()->findConnection(connectionId);
            Knm::BluetoothSetting * setting = static_cast<Knm::BluetoothSetting *>(c->setting(Knm::Setting::Bluetooth));

            if (setting && setting->bdaddr() == btSetting->bdaddr()) {
                kDebug() << "Updating existing bluetooth connection instead of creating one";
                connect(mSystemSettings, SIGNAL(connectionsChanged()), SLOT(updateConnectionCompleted()));
                mSystemSettings->updateConnection(c->uuid(), con);
                addConnection = false;

                // In case the connectionChanged signal never arrives.
                QTimer::singleShot(60000, this, SLOT(updateConnectionCompleted()));
                break;
            }
        }
    } else {
        kError(KDE_DEFAULT_DEBUG_AREA) << "bluetooth setting not found";
    }

    if (addConnection) {
        connect(mSystemSettings, SIGNAL(addConnectionCompleted(bool, const QString &)), SLOT(addConnectionCompleted(bool, const QString &)));
        connect(&m_manager, SIGNAL(ActivatableAdded(QString, uint, int)), this, SLOT(activatableAdded(QString, uint, int)));
        mSystemSettings->addConnection(con);
    }
}

ManageConnection::~ManageConnection()
{
    if (mSystemSettings) {
        delete mSystemSettings;
    }
}

void ManageConnection::saveConnection(Knm::Connection *con)
{
    kDebug() << "Saving connection" << con->name();
    ManageConnection *ptr = new ManageConnection(con);
}

void ManageConnection::addConnectionCompleted(bool valid, const QString &errorMessage)
{
    kDebug(KDE_DEFAULT_DEBUG_AREA);
    if (!valid) {
        if (errorMessage.isEmpty())
            KMessageBox::error(0, i18n("Connection create operation failed."));
        else
            KMessageBox::error(0, errorMessage);

        deleteLater();
        kapp->quit();
    }
}

void ManageConnection::activatableAdded(QString path, uint type, int index)
{
    Q_UNUSED(type)
    Q_UNUSED(index)
    QDBusInterface activatable("org.kde.networkmanagement",
                               path,
                               "org.kde.networkmanagement.Activatable",
                               QDBusConnection::sessionBus());

    if (!activatable.isValid()) {
        goto OUT;
    }

    // Activate the connection. This step is required to make hidden wifi networks work.
    activatable.call("activate");

OUT:
    deleteLater();
    kapp->quit();
}

void ManageConnection::updateConnectionCompleted()
{
    deleteLater();
    kapp->quit();
}
