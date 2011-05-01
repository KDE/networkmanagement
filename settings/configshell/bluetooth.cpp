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

#include <nm-setting-bluetooth.h>

#include <QDBusInterface>
#include <QDBusReply>

#include <KDebug>
#include <KApplication>
#include <KStandardDirs>

#include "bluetooth.h"

// For now all connections are user scope. For NM-0.9 we will have to change that to system scope.
void saveConnection(Knm::Connection *con)
{
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
#include <QTimer>

Bluetooth::Bluetooth(const QString bdaddr, const QString service): QObject(), mBdaddr(bdaddr), mService(service), mobileConnectionWizard(0)
{
    mService = mService.toLower();
    if (mService == "dun") {
        connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceAdded(const QString &)),
                SLOT(modemAdded(const QString &)));
    }
    QTimer::singleShot(0, this, SLOT(init()));
}

Bluetooth::~Bluetooth()
{
}

void Bluetooth::init()
{
    QRegExp rx("dun|rfcomm?|nap");

    if (rx.indexIn(mService) < 0) {
        kDebug(KDE_DEFAULT_DEBUG_AREA) << "Error: we only support 'dun' and 'nap' services.";
        kapp->quit();
        return;
    }
//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Bdaddr == " << mBdaddr;

    /*
     * Find default bluetooth adapter registered in BlueZ.
     */

    QDBusInterface bluez(QLatin1String("org.bluez"), QLatin1String("/"),
                         QLatin1String("org.bluez.Manager"), QDBusConnection::systemBus());

    if (!bluez.isValid()) {
        kDebug(KDE_DEFAULT_DEBUG_AREA) << "Error: could not contact BlueZ.";
        kapp->quit();
        return;
    }

//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Querying default adapter";
    QDBusReply<QDBusObjectPath> adapterPath = bluez.call(QLatin1String("DefaultAdapter"));

    if (!adapterPath.isValid()) {
        kDebug(KDE_DEFAULT_DEBUG_AREA) << "Error: default bluetooth adapter not found. Quiting.";
        kapp->quit();
        return;
    }

//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Default adapter path is " << adapterPath.value().path();

    /*
     * Find device path registered in BlueZ.
     */

    QDBusInterface adapter(QLatin1String("org.bluez"), adapterPath.value().path(),
                           QLatin1String("org.bluez.Adapter"), QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> devicePath = adapter.call(QLatin1String("FindDevice"), mBdaddr);

    if (!devicePath.isValid()) {
        kDebug(KDE_DEFAULT_DEBUG_AREA) << mBdaddr << " is not registered in default bluetooth adapter, it may be in another adapter.";
        kDebug(KDE_DEFAULT_DEBUG_AREA) << mBdaddr << " waiting for it to be registered in ModemManager";
        return;
    }

    mDevicePath = devicePath.value().path();
    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Device path for " << mBdaddr << " is " << mDevicePath;

    /*
     * Find name registered in BlueZ.
     */

    // get device properties
    QDBusInterface device(QLatin1String("org.bluez"), mDevicePath,
                          QLatin1String("org.bluez.Device"), QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > deviceProperties = device.call(QLatin1String("GetProperties"));

    if (!deviceProperties.isValid()) {
        return;
    }

    QMap<QString, QVariant> properties = deviceProperties.value();
//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Device properties == " << properties;

    if (properties.contains("Name")) {
        kDebug(KDE_DEFAULT_DEBUG_AREA) << "Name for" << mBdaddr << "is" << properties["Name"].toString();
        mDeviceName = properties["Name"].toString();
    }

    if (mService == "nap") {
        ConnectionEditor editor(0);
        Knm::Connection *con = editor.createConnection(true, Knm::Connection::Bluetooth, QVariantList() << mDeviceName << QLatin1String(NM_SETTING_BLUETOOTH_TYPE_PANU) << mBdaddr, false);
        if (con) {
            saveConnection(con);
        }
        kapp->quit();
        return;
    } else if (mService != QLatin1String("dun")) {
        mDunDevice = mService;
        kDebug(KDE_DEFAULT_DEBUG_AREA) << "device(" << mDunDevice << ") for" << mBdaddr << " passed as argument";
        kDebug(KDE_DEFAULT_DEBUG_AREA) << "waiting for it to be registered in ModemManager";
        return;
    }

    /*
     * Contact BlueZ to connect phone's service.
     */
    QDBusInterface serial(QLatin1String("org.bluez"), mDevicePath,
                          QLatin1String("org.bluez.Serial"), QDBusConnection::systemBus());
   
    QDBusReply<QString> reply = serial.call(QLatin1String("Connect"), mService);
    if (!reply.isValid()) {
        kDebug(KDE_DEFAULT_DEBUG_AREA) << "Error: org.bluez.Serial.Connect did not work. Quiting.";
        kapp->quit();
        return;
    }

    mDunDevice = reply.value();
}

void Bluetooth::modemAdded(const QString &udi)
{
    kDebug(KDE_DEFAULT_DEBUG_AREA);
    Solid::Control::ModemInterface *modem = Solid::Control::ModemManager::findModemInterface(udi, Solid::Control::ModemInterface::GsmCard);

    if (!modem) {
        // Try CDMA if no GSM device has been found.
        modem = Solid::Control::ModemManager::findModemInterface(udi, Solid::Control::ModemInterface::NotGsm);
    }

    QStringList temp = mDunDevice.split("/");
    if (temp.count() == 3) {
        mDunDevice = temp[2];
    }

    if (!modem || modem->device() != mDunDevice) {
        if (modem) {
            kDebug(KDE_DEFAULT_DEBUG_AREA) << "Modem" << modem->device() << " is not the one we want(" << mDunDevice << "). Quitting.";
        } else {
            kDebug(KDE_DEFAULT_DEBUG_AREA) << "Modem interface for " << udi << " not found. Quitting";
        }
        kapp->quit();
        return;
    }

    Knm::Connection::Type type;
    switch (modem->type()) {
        case Solid::Control::ModemInterface::GsmType: type = Knm::Connection::Gsm; break;
        case Solid::Control::ModemInterface::CdmaType: type = Knm::Connection::Cdma; break;
        default: type = Knm::Connection::Unknown;
    }

    if (type == Knm::Connection::Unknown) {
        kapp->quit();
        return;
    }

    if (mobileConnectionWizard) {
        return;
    }

    mobileConnectionWizard = new MobileConnectionWizard(type);
    Knm::Connection *con = 0;
    ConnectionEditor editor(0);

    if (mobileConnectionWizard->exec() == QDialog::Accepted &&
        mobileConnectionWizard->getError() == MobileProviders::Success) {
        con = editor.createConnection(true, Knm::Connection::Bluetooth, QVariantList() << mDeviceName << QLatin1String(NM_SETTING_BLUETOOTH_TYPE_DUN) << mBdaddr << mobileConnectionWizard->args(), false);
        if (con) {
            saveConnection(con);
        }
    }
    delete mobileConnectionWizard;
    kapp->quit();
}

#endif
