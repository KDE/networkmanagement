#include <QDBusInterface>
#include <QDBusReply>

#include <KDebug>
#include <KApplication>

#include "connection.h"
#include "connectioneditor.h"
#include "connectionpersistence.h"
#include "knmserviceprefs.h"

#include "bluetooth.h"

#ifdef COMPILE_MODEM_MANAGER_SUPPORT

Bluetooth::Bluetooth(const QString bdaddr, const QString device): QObject(), mBdaddr(bdaddr), mDevice(device), mobileConnectionWizard(0)
{
    connect(Solid::Control::ModemManager::notifier(), SIGNAL(modemInterfaceAdded(const QString &)),
            SLOT(modemAdded(const QString &)));
}

Bluetooth::~Bluetooth()
{
}

void Bluetooth::modemAdded(const QString &udi)
{
    kDebug();
    Solid::Control::ModemInterface *modem = Solid::Control::ModemManager::findModemInterface(udi, Solid::Control::ModemInterface::GsmCard);

    if (!modem) {
        modem = Solid::Control::ModemManager::findModemInterface(udi, Solid::Control::ModemInterface::NotGsm);
    }

    if (!modem || modem->device() != mDevice) {
        if (modem) {
            kDebug() << "Modem" << modem->device() << " is not the one we want(" << mDevice << "). Quitting.";
        } else {
            kDebug() << "Modem interface for " << udi << " not found. Quitting";
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
        Bluetooth::saveConnection(con);
    }
    delete mobileConnectionWizard;
    kapp->quit();
}
#endif

// For now all connections are user scope. For NM-0.9 we will have to change that to system scope.
void Bluetooth::saveConnection(Knm::Connection *con)
{
    kDebug();
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

QString Bluetooth::deviceName()
{
    kDebug() << "Bdaddr == " << mBdaddr;
    // find the default adapter
    QDBusInterface bluez(QLatin1String("org.bluez"), QLatin1String("/"),
                         QLatin1String("org.bluez.Manager"), QDBusConnection::systemBus());

    if (!bluez.isValid()) {
        return QString();
    }

    kDebug() << "Querying default adapter";

    QDBusReply<QDBusObjectPath> adapterPath = bluez.call(QLatin1String("DefaultAdapter"));

    if (!adapterPath.isValid()) {
        return QString();
    }

    kDebug() << "Default adapter path == " << adapterPath.value().path();

    // find the device path
    QDBusInterface adapter(QLatin1String("org.bluez"), adapterPath.value().path(),
                     QLatin1String("org.bluez.Adapter"), QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> devicePath = adapter.call(QLatin1String("FindDevice"), mBdaddr);

    if (!devicePath.isValid()) {
        return QString();
    }

    kDebug() << "Device path == " << devicePath.value().path();

    // get the properties
    QDBusInterface device(QLatin1String("org.bluez"), devicePath.value().path(),
                     QLatin1String("org.bluez.Device"), QDBusConnection::systemBus());

    QDBusReply<QMap<QString, QVariant> > deviceProperties = device.call(QLatin1String("GetProperties"));

    if (!deviceProperties.isValid()) {
        return QString();
    }

    QMap<QString, QVariant> properties = deviceProperties.value();
    kDebug() << "Device properties == " << properties;

    if (properties.contains("Name")) {
        kDebug() << "Name for" << mBdaddr << "is" << properties["Name"].toString();
        return properties["Name"].toString();
    }
    return QString();
}
