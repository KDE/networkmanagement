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
#include <QUuid>
#include <QTimer>

#include <KDebug>
#include <KStandardDirs>
#include <KLocale>
#include <KMessageBox>

#include "bluetooth.h"
#include "manageconnection.h"

Bluetooth::Bluetooth(const QString bdaddr, const QString service): QObject(), mBdaddr(bdaddr), mService(service), mobileConnectionWizard(0)
{
    mService = mService.toLower();
    if (mService == "dun") {
        connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceAdded(QString)),
                SLOT(modemAdded(QString)));
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
        KMessageBox::sorry(0, i18n("We support 'dun' and 'nap' services only."));
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
        KMessageBox::error(0, i18n("Could not contact bluetooth manager (BlueZ)."));
        kapp->quit();
        return;
    }

//    kDebug(KDE_DEFAULT_DEBUG_AREA) << "Querying default adapter";
    QDBusReply<QDBusObjectPath> adapterPath = bluez.call(QLatin1String("DefaultAdapter"));

    if (!adapterPath.isValid()) {
        KMessageBox::error(0, i18n("Default bluetooth adapter not found: %1", adapterPath.error().message()));
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
        kWarning(KDE_DEFAULT_DEBUG_AREA) << mBdaddr << " is not registered in default bluetooth adapter, it may be in another adapter.";
        kWarning(KDE_DEFAULT_DEBUG_AREA) << mBdaddr << " waiting for it to be registered in ModemManager";
	QTimer::singleShot(60000, kapp, SLOT(quit()));
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

    /*
     * Check if phone supports the requested service.
     */
    bool dun = false, nap = false;
    if (properties.contains("UUIDs")) {
        foreach (const QString &u, properties["UUIDs"].toStringList()) {
            QUuid uuid(u);
            if (uuid.data1 == 0x1103) {
                dun = true;
            } else if (uuid.data1 == 0x1116) {
                nap = true;
            }
        }
    }

    if (mService != QLatin1String("nap") && !dun) {
        KMessageBox::error(0, i18n("%1 (%2) does not support Dialup Networking (DUN).", mDeviceName, mBdaddr));
        kapp->quit();
        return;
    }

    if (mService == QLatin1String("nap") && !nap) {
        KMessageBox::error(0, i18n("%1 (%2) does not support Network Access Point (NAP).", mDeviceName, mBdaddr));
        kapp->quit();
        return;
    }

    if (mService == QLatin1String("nap")) {
        ConnectionEditor editor(0);
        Knm::Connection *con = editor.createConnection(true, Knm::Connection::Bluetooth, QVariantList() << mDeviceName << QLatin1String(NM_SETTING_BLUETOOTH_TYPE_PANU) << mBdaddr, false);
        if (con) {
            ManageConnection::saveConnection(con);
        }
        return;
    } else if (mService != QLatin1String("dun")) {
        mDunDevice = mService;
        kWarning(KDE_DEFAULT_DEBUG_AREA) << "device(" << mDunDevice << ") for" << mBdaddr << " passed as argument";
        kWarning(KDE_DEFAULT_DEBUG_AREA) << "waiting for it to be registered in ModemManager";
        return;
    }

    /*
     * Contact BlueZ to connect phone's service.
     */
    QDBusInterface serial(QLatin1String("org.bluez"), mDevicePath,
                          QLatin1String("org.bluez.Serial"), QDBusConnection::systemBus());
   
    QDBusReply<QString> reply = serial.call(QLatin1String("Connect"), mService);
    if (!reply.isValid()) {
        KMessageBox::error(0, i18n("Error activating devices's serial port: %1", reply.error().message()));
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
            KMessageBox::error(0, i18n("Device %1 is not the one we want (%2)", modem->device(), mDunDevice));
        } else {
            KMessageBox::error(0, i18n("Device for serial port %1 (%2) not found.", mDunDevice, udi));
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
            ManageConnection::saveConnection(con);
        }
    }
    delete mobileConnectionWizard;
}
