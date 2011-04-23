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

#include <QDBusInterface>
#include <QDBusReply>

#include <KDebug>
#include <KApplication>
#include <KStandardDirs>

#include "bluetooth.h"

// For now all connections are user scope. For NM-0.9 we will have to change that to system scope.
void saveConnection(Knm::Connection *con)
{
    kDebug(KDE_DEFAULT_DEBUG_AREA);
    // persist the Connection
    QString connectionFile = KStandardDirs::locateLocal("data",
        Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + con->uuid());

    Knm::ConnectionPersistence cp(
            con,
            KSharedConfig::openConfig(connectionFile),
            (Knm::ConnectionPersistence::SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode()
            );
    cp.save();

    // add to the service prefs
    QString name = con->name();
    QString type = Knm::Connection::typeAsString(con->type());
    KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
    KConfigGroup config(prefs->config(), QLatin1String("Connection_") + con->uuid());
    QStringList connectionIds = prefs->connections();
    // check if already present, we may be editing an existing Connection
    if (!connectionIds.contains(con->uuid()))
    {
        connectionIds << con->uuid();
        prefs->setConnections(connectionIds);
    }
    config.writeEntry("Name", name);
    config.writeEntry("Type", type);
    prefs->writeConfig();

    ConnectionEditor editor(0);
    editor.updateService(QStringList() << con->uuid().toString());
}

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
Bluetooth::Bluetooth(const QString bdaddr): QObject(), mBdaddr(bdaddr), mDunDevice(QString()), mobileConnectionWizard(0)
{
    connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceAdded(const QString &)),
            SLOT(modemAdded(const QString &)));
}

Bluetooth::Bluetooth(const QString bdaddr, const QString dunDevice): QObject(), mBdaddr(bdaddr), mDunDevice(dunDevice), mobileConnectionWizard(0)
{
    connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceAdded(const QString &)),
            SLOT(modemAdded(const QString &)));
}

Bluetooth::~Bluetooth()
{
}

void Bluetooth::modemAdded(const QString &udi)
{
    kDebug(KDE_DEFAULT_DEBUG_AREA);
    Solid::Control::ModemInterface *modem = Solid::Control::ModemManager::findModemInterface(udi, Solid::Control::ModemInterface::GsmCard);

    if (!modem) {
        // Try CDMA if no GSM device has been found.
        modem = Solid::Control::ModemManager::findModemInterface(udi, Solid::Control::ModemInterface::NotGsm);
    }

    // TODO: implement PANU (mDunDevice is empty with PANU)
    if (!modem || modem->device() != mDunDevice) {
        if (modem) {
            kDebug(KDE_DEFAULT_DEBUG_AREA) << "Modem" << modem->device() << " is not the one we want(" << mDunDevice << "). Quitting.";
        } else {
            kDebug(KDE_DEFAULT_DEBUG_AREA) << "Modem interface for " << udi << " not found. Quitting";
        }
        kapp->quit();
    }

    Knm::Connection::Type type;
    switch (modem->type()) {
        case Solid::Control::ModemInterface::GsmType: type = Knm::Connection::Gsm; break;
        case Solid::Control::ModemInterface::CdmaType: type = Knm::Connection::Cdma; break;
        default: type = Knm::Connection::Unknown;
    }

    if (type == Knm::Connection::Unknown) {
        kapp->quit();
    }

    if (mobileConnectionWizard) {
        return;
    }

    mobileConnectionWizard = new MobileConnectionWizard(type);
    Knm::Connection *con = 0;
    ConnectionEditor editor(0);

    if (mobileConnectionWizard->exec() == QDialog::Accepted &&
        mobileConnectionWizard->getError() == MobileProviders::Success) {
        con = editor.createConnection(true, Knm::Connection::Bluetooth, mobileConnectionWizard->args() << mBdaddr << deviceName(), false);
        saveConnection(con);
    }
    delete mobileConnectionWizard;
    kapp->quit();
}

QString Bluetooth::deviceName()
{
//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Bdaddr == " << mBdaddr;
    // find the default adapter
    QDBusInterface bluez(QLatin1String("org.bluez"), QLatin1String("/"),
                         QLatin1String("org.bluez.Manager"), QDBusConnection::systemBus());

    if (!bluez.isValid()) {
        return QString();
    }

//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Querying default adapter";

    QDBusReply<QDBusObjectPath> adapterPath = bluez.call(QLatin1String("DefaultAdapter"));

    if (!adapterPath.isValid()) {
        return QString();
    }

//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Default adapter path == " << adapterPath.value().path();

    // find the device path
    QDBusInterface adapter(QLatin1String("org.bluez"), adapterPath.value().path(),
                     QLatin1String("org.bluez.Adapter"), QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> devicePath = adapter.call(QLatin1String("FindDevice"), mBdaddr);

    if (!devicePath.isValid()) {
        return QString();
    }

//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Device path == " << devicePath.value().path();

    // get the properties
    QDBusInterface device(QLatin1String("org.bluez"), devicePath.value().path(),
                     QLatin1String("org.bluez.Device"), QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > deviceProperties = device.call(QLatin1String("GetProperties"));

    if (!deviceProperties.isValid()) {
        return QString();
    }

    QMap<QString, QVariant> properties = deviceProperties.value();
//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Device properties == " << properties;

    if (properties.contains("Name")) {
//        kDebug(KDE_DEFAULT_DEBUG_AREA) << "Name for" << mBdaddr << "is" << properties["Name"].toString();
        return properties["Name"].toString();
    }
    return QString();
}
#endif
