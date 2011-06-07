/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2011 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>

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

#include "manageconnectionwidget.h"

#include <unistd.h>

#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-gsm.h>
#include <NetworkManager.h>

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDateTime>
#include <QFile>
#include <QMenu>
#include <QTimer>
#include <QEvent>
#include <QKeyEvent>

#include <KCModuleProxy>
#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPluginInfo>
#include <KPushButton>
#include <KRandom>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KToolInvocation>
#include <kfiledialog.h>
#include <KUser>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include "knmserviceprefs.h"
#include "connection.h"
#include "connectionlist.h"
#include "connectionpersistence.h"
#include "connectionprefs.h"
#include "vpnuiplugin.h"
#include "settings/vpn.h"

#include <tooltips.h>

#define ConnectionIdRole Qt::UserRole + 1
#define ConnectionTypeRole Qt::UserRole + 2
#define ConnectionLastUsedRole Qt::UserRole + 3
#define ConnectionScopeRole Qt::UserRole + 4

K_PLUGIN_FACTORY( ManageConnectionWidgetFactory, registerPlugin<ManageConnectionWidget>();)
K_EXPORT_PLUGIN( ManageConnectionWidgetFactory( "kcm_networkmanagement", "libknetworkmanager" ) )

ManageConnectionWidget::ManageConnectionWidget(QWidget *parent, const QVariantList &args)
: KCModule( ManageConnectionWidgetFactory::componentData(), parent, args ), mCellularMenu(0), mVpnMenu(0), mEditor(new ConnectionEditor(this)), mEditConnection(0)
{
    connect(mEditor, SIGNAL(connectionsChanged()), this, SLOT(restoreConnections()));

    mConnEditUi.setupUi(this);
    mConnEditUi.listWired->setSortingEnabled(true);
    mConnEditUi.listWired->sortByColumn(0, Qt::AscendingOrder);
    mConnEditUi.listWireless->setSortingEnabled(true);
    mConnEditUi.listWireless->sortByColumn(0, Qt::AscendingOrder);
    mConnEditUi.listCellular->setSortingEnabled(true);
    mConnEditUi.listCellular->sortByColumn(0, Qt::AscendingOrder);
    mConnEditUi.listVpn->setSortingEnabled(true);
    mConnEditUi.listVpn->sortByColumn(0, Qt::AscendingOrder);
    mConnEditUi.listPppoe->setSortingEnabled(true);
    mConnEditUi.listPppoe->sortByColumn(0, Qt::AscendingOrder);

    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    mConnections = new ConnectionList(this);
    mSystemSettings = new NMDBusSettingsConnectionProvider(mConnections, NMDBusSettingsService::SERVICE_SYSTEM_SETTINGS, this);

    connect(mSystemSettings, SIGNAL(getConnectionSecretsCompleted(bool, const QString &)), this, SLOT(editGotSecrets(bool, const QString&)) );
    connect(mSystemSettings, SIGNAL(addConnectionCompleted(bool, const QString &)), this, SLOT(addGotConnection(bool, const QString&)) );
    connect(mSystemSettings, SIGNAL(connectionsChanged()), this, SLOT(restoreConnections()));

    connectButtonSet(mConnEditUi.buttonSetWired, mConnEditUi.listWired);
    connectButtonSet(mConnEditUi.buttonSetWireless, mConnEditUi.listWireless);
    connectButtonSet(mConnEditUi.buttonSetCellular, mConnEditUi.listCellular);
    connectButtonSet(mConnEditUi.buttonSetVpn, mConnEditUi.listVpn);
    connectButtonSet(mConnEditUi.buttonSetPppoe, mConnEditUi.listPppoe);
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(updateTabStates()));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(updateTabStates()));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(activeConnectionsChanged()),
            SLOT(activeConnectionsChanged()));
    connect(mConnEditUi.tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));

    // handle doubleclicks
    connect(mConnEditUi.listWired, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editClicked()));
    connect(mConnEditUi.listWireless, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editClicked()));
    connect(mConnEditUi.listCellular, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editClicked()));
    connect(mConnEditUi.listVpn, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editClicked()));
    connect(mConnEditUi.listPppoe, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editClicked()));

    restoreConnections();

    if (!Solid::Control::NetworkManager::isWirelessEnabled()) {
        mConnEditUi.tabWidget->setCurrentIndex(0);
    }

    if (QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.NetworkManager.KCModule"))) {
        bool result = QDBusConnection::sessionBus().registerObject(QLatin1String("/default"), this, QDBusConnection::ExportScriptableSlots);
        if (result)
            kDebug() << "/default interface succesfully registered.";
        else
            kWarning() << "/default interface could not be registered.";
    } else
        kWarning() << "org.kde.NetworkManager.KCModule interface cannot be registered!";

    mLastUsedTimer = new QTimer(this);
    connect(mLastUsedTimer, SIGNAL(timeout()), SLOT(updateLastUsed()));
    mLastUsedTimer->start(1000 * 60);

    setButtons(KCModule::NoAdditionalButton);
    mMobileConnectionWizard = 0;
}

ManageConnectionWidget::~ManageConnectionWidget()
{
    QDBusConnection::sessionBus().unregisterService(QLatin1String("org.kde.NetworkManager.KCModule"));
    //HACK: don't destroy until NMDBusSettingsConnectionProvider returns
    //(prevents crashes when closing kcmshell too fast after adding/editing
    //a connection, as this is then deleted from another thread)
    usleep(100000);
    delete mSystemSettings;
    delete mConnections;
    delete mEditor;
}

void ManageConnectionWidget::createConnection(const QString &connectionType, const QVariantList &args)
{
    Knm::Connection * con = mEditor->createConnection(false, Knm::Connection::typeFromString(connectionType), args);

    kDebug() << "con is " << con;

    if (con) {
        kDebug() << "Connection pointer is set, connection will be added.";

        if (con->scope() == Knm::Connection::User)
            saveConnection(con);
        else
            mSystemSettings->addConnection(con);

        updateServiceAndUi(con);
    }
}

QString ManageConnectionWidget::formatDateRelative(const QDateTime & lastUsed)
{
    QString lastUsedText;
    if (lastUsed.isValid()) {
        QDateTime now = QDateTime::currentDateTime();
        if (lastUsed.daysTo(now) == 0 ) {
            int secondsAgo = lastUsed.secsTo(now);
            if (secondsAgo < (60 * 60 )) { // less than an hour ago
                int minutesAgo = secondsAgo / 60;
                lastUsedText = i18ncp(
                        "Label for last used time for a network connection used in the last hour, as the number of minutes since usage",
                        "One minute ago",
                        "%1 minutes ago",
                        minutesAgo);
            } else {
                int hoursAgo = secondsAgo / (60 * 60);
                lastUsedText = i18ncp(
                        "Label for last used time for a network connection used in the last day, as the number of hours since usage",
                        "One hour ago",
                        "%1 hours ago",
                        hoursAgo);
            }
        } else if (lastUsed.daysTo(now) == 1) {
            lastUsedText = i18nc("Label for last used time for a network connection used the previous day", "Yesterday");
        } else {
            lastUsedText = KGlobal::locale()->formatDate(lastUsed.date(), KLocale::ShortDate);
        }
    } else {
        lastUsedText =  i18nc("Label for last used time for a "
                "network connection that has never been used", "Never");
    }
    return lastUsedText;
}

void ManageConnectionWidget::restoreUserConnections()
{
    // if settings are accessed from plasma applet settings, KNetworkManagerServicePrefs reads the configuration once
    // and always displays the same connection list, even if connections are updated from systemsettings. Line below,
    // always makes the list up-to-date by reading configuration from disk.
    KNetworkManagerServicePrefs::self()->readConfig();
    QStringList connectionIds = KNetworkManagerServicePrefs::self()->connections();
    QList<QTreeWidgetItem *> wiredItems, wirelessItems, cellularItems, vpnItems, pppoeItems;
    foreach (const QString &connectionId, connectionIds) {
        // look in the corresponding group
        // read name, type, last used
        kDebug() << connectionId;
        KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("Connection_") + connectionId);
        if (!config.exists()) {
            continue;
        }
        QString name = config.readEntry("Name", QString());
        QString typeString = config.readEntry("Type", QString());
        if ( name.isEmpty() || typeString.isEmpty() ) {
            continue;
        }
        QDateTime lastUsed = config.readEntry("LastUsed", QDateTime());
        Knm::Connection::Type type = Knm::Connection::typeFromString(typeString);
        // add an item to the editor widget for that type
        QStringList itemContents;
        itemContents << name;
        itemContents << Knm::Connection::scopeAsLocalizedString(Knm::Connection::User);
        itemContents << formatDateRelative(lastUsed);

        kDebug() << typeString << name << lastUsed;
        QTreeWidgetItem * item = 0;

        switch(type) {
            case Knm::Connection::Wired:
                item = new QTreeWidgetItem(mConnEditUi.listWired, itemContents);
                wiredItems.append(item);
                break;
            case Knm::Connection::Wireless:
                item = new QTreeWidgetItem(mConnEditUi.listWireless, itemContents);
                wirelessItems.append(item);
                break;
            case Knm::Connection::Gsm:
                item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
                cellularItems.append(item);
                break;
            case Knm::Connection::Cdma:
                item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
                cellularItems.append(item);
                break;
            case Knm::Connection::Bluetooth:
                item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
                cellularItems.append(item);
                break;
            case Knm::Connection::Vpn:
                item = new QTreeWidgetItem(mConnEditUi.listVpn, itemContents);
                vpnItems.append(item);
                break;
            case Knm::Connection::Pppoe:
                item = new QTreeWidgetItem(mConnEditUi.listPppoe, itemContents);
                pppoeItems.append(item);
                break;
            default:
                break;
        }

        if (item) {
            mUuidItemHash.insert(connectionId, item);
            item->setData(0, ConnectionIdRole, connectionId);
            item->setData(0, ConnectionTypeRole, type);
            item->setData(0, ConnectionLastUsedRole, lastUsed);
            item->setData(0, ConnectionScopeRole, Knm::Connection::User);
        }
    }
    mConnEditUi.listWired->insertTopLevelItems(0, wiredItems);
    mConnEditUi.listWireless->insertTopLevelItems(0, wirelessItems);
    mConnEditUi.listCellular->insertTopLevelItems(0, cellularItems);
    mConnEditUi.listVpn->insertTopLevelItems(0, vpnItems);
    mConnEditUi.listPppoe->insertTopLevelItems(0, pppoeItems);
}

void ManageConnectionWidget::restoreConnections()
{
    // clean up the lists
    mConnEditUi.listWired->clear();
    mConnEditUi.listWireless->clear();
    mConnEditUi.listCellular->clear();
    mConnEditUi.listVpn->clear();
    mConnEditUi.listPppoe->clear();

    // if settings are accessed from plasma applet settings, KNetworkManagerServicePrefs reads the configuration once
    // and always displays the same connection list, even if connections are updated from systemsettings. Line below,
    // always makes the list up-to-date by reading configuration from disk.
    KNetworkManagerServicePrefs::self()->readConfig();
    QStringList connectionIds = KNetworkManagerServicePrefs::self()->connections();


    QList<QTreeWidgetItem *> wiredItems, wirelessItems, cellularItems, vpnItems, pppoeItems;
    foreach (const QString &connectionId, mConnections->connections()) {
        // look in the corresponding group
        // read name, type, last used
        kDebug() << connectionId;
        // KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("Connection_") + connectionId);
        // if (!config.exists()) {
        //     continue;
        // }

        Knm::Connection *con = mConnections->findConnection(connectionId);

        if (!con) {
            //kDebug() << "Connection" << con->name() << "not found";
            continue;
        }
        if (con->scope() == Knm::Connection::User) {
            //kDebug() << "Connection" << con->name() << "is user scope";
            continue;
        }

        QString name = con->name();
        QString typeString = con->typeAsString(con->type());

        Knm::Ipv4Setting *setting = static_cast<Knm::Ipv4Setting*>(con->setting(Knm::Setting::Ipv4));
        if (setting)
            kDebug() << "IPv4 setting method" << setting->method();

        kDebug() << "Restoring connection " <<  name << typeString;

        if ( name.isEmpty() || typeString.isEmpty() ) {
            continue;
        }

        Knm::Connection::Type type = Knm::Connection::typeFromString(typeString);
        QDateTime lastUsed = con->timestamp();
        // add an item to the editor widget for that type
        QStringList itemContents;
        itemContents << name;
        itemContents << Knm::Connection::scopeAsLocalizedString(Knm::Connection::System);
        itemContents << formatDateRelative(lastUsed);

        kDebug() << typeString << name << lastUsed;
        QTreeWidgetItem * item = 0;

        switch (type) {
            case Knm::Connection::Wired:
                item = new QTreeWidgetItem(mConnEditUi.listWired, itemContents);
                wiredItems.append(item);
                break;
            case Knm::Connection::Wireless:
                item = new QTreeWidgetItem(mConnEditUi.listWireless, itemContents);
                wirelessItems.append(item);
                break;
            case Knm::Connection::Gsm:
                item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
                cellularItems.append(item);
                break;
            case Knm::Connection::Cdma:
                item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
                cellularItems.append(item);
                break;
            case Knm::Connection::Bluetooth:
                item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
                cellularItems.append(item);
                break;
            case Knm::Connection::Vpn:
                item = new QTreeWidgetItem(mConnEditUi.listVpn, itemContents);
                vpnItems.append(item);
                break;
            case Knm::Connection::Pppoe:
                item = new QTreeWidgetItem(mConnEditUi.listPppoe, itemContents);
                pppoeItems.append(item);
                break;
            default:
                break;
        }

        if (item) {
            mUuidItemHash.insert(connectionId, item);
            item->setData(0, ConnectionIdRole, connectionId);
            item->setData(0, ConnectionTypeRole, type);
            item->setData(0, ConnectionLastUsedRole, lastUsed);
            item->setData(0, ConnectionScopeRole, con->scope());
        }
    }

    restoreUserConnections();

    mConnEditUi.listWired->insertTopLevelItems(0, wiredItems);
    mConnEditUi.listWired->resizeColumnToContents(0);
    mConnEditUi.listWireless->insertTopLevelItems(0, wirelessItems);
    mConnEditUi.listWireless->resizeColumnToContents(0);
    mConnEditUi.listCellular->insertTopLevelItems(0, cellularItems);
    mConnEditUi.listCellular->resizeColumnToContents(0);
    mConnEditUi.listVpn->insertTopLevelItems(0, vpnItems);
    mConnEditUi.listVpn->resizeColumnToContents(0);
    mConnEditUi.listPppoe->insertTopLevelItems(0, pppoeItems);
    mConnEditUi.listPppoe->resizeColumnToContents(0);

    // check which tabs should be enabled depending on the existing hardware
    updateTabStates();
}

void ManageConnectionWidget::updateTabStates()
{
    bool hasWired = false, hasWireless = false, hasCellular = false, hasDsl = false;
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        switch (iface->type()) {
            case Solid::Control::NetworkInterface::Ieee8023:
                hasWired = true;
                break;
            case Solid::Control::NetworkInterface::Ieee80211:
                hasWireless = true;
                break;
            case Solid::Control::NetworkInterface::Serial:
                hasDsl = true;
                break;
            case Solid::Control::NetworkInterface::Gsm:
            case Solid::Control::NetworkInterface::Cdma:
#ifdef NM_0_8
            case Solid::Control::NetworkInterface::Bluetooth:
#endif
                hasCellular = true;
                break;
            default:
                break;
        }
    }

    mConnEditUi.tabWidget->setTabEnabled(0, (hasWired || mConnEditUi.listWired->topLevelItemCount()));
    mConnEditUi.tabWidget->setTabEnabled(1, (hasWireless || mConnEditUi.listWireless->topLevelItemCount()));
    mConnEditUi.tabWidget->setTabEnabled(2, (hasCellular || mConnEditUi.listCellular->topLevelItemCount()));
    if (KServiceTypeTrader::self()->query(QLatin1String("NetworkManagement/VpnUiPlugin")).isEmpty()) {
        //mConnEditUi.tabWidget->setTabEnabled(3, false);
        mConnEditUi.tabWidget->setTabToolTip(3, i18nc("Tooltip for disabled tab when no VPN plugins are installed", "No VPN plugins were found"));
    } else {
        mConnEditUi.tabWidget->setTabEnabled(3, true);
    }
    mConnEditUi.tabWidget->setTabEnabled(4, (hasWired || hasDsl || mConnEditUi.listPppoe->topLevelItemCount()));
}

void ManageConnectionWidget::addClicked()
{
    Knm::Connection *con = NULL;

    kDebug() << "Add clicked, currentIndex is " << connectionTypeForCurrentIndex();

    if (connectionTypeForCurrentIndex() == Knm::Connection::Gsm) {
        kDebug() << "GSM tab selected, connection wizard will be shown.";

        delete mMobileConnectionWizard;
        mMobileConnectionWizard = new MobileConnectionWizard();

        if (mMobileConnectionWizard->exec() == QDialog::Accepted) {
            if (mMobileConnectionWizard->getError() == MobileProviders::Success) {
                kDebug() << "Got no error, creating connection";

                // Use false as the last parameter to make users see the Gsm Connection details for last check,
                // some additionals may be changed here, i.e. pin code
                con = mEditor->createConnection(false, mMobileConnectionWizard->type(), mMobileConnectionWizard->args(), false);
            } else { // fallback to old manual editing if something wrong happened with the wizard
                kDebug() << "Got error, falling back to old method";
                con = mEditor->createConnection(false, mMobileConnectionWizard->type());
            }
        }
    } else { // show connection settings widget for the active tab
         con = mEditor->createConnection(false, connectionTypeForCurrentIndex());
    }

    if (con) {
        kDebug() << "Connection pointer is set, connection will be added.";

        if (con->scope() == Knm::Connection::User)
            saveConnection(con);
        else
            mSystemSettings->addConnection(con);

        updateServiceAndUi(con);
    }
    else
        kDebug() << "Connection pointer is not set, connection will not be added!";

}

void ManageConnectionWidget::importClicked()
{
    //Get the file from which connection is to be imported
    QString impFile = KFileDialog::getOpenFileName(KUser().homeDir(),"",this,i18nc("File chooser dialog title for importing VPN","Import VPN connection settings"));
    if (impFile.isEmpty())
        return;

    //Try to import the connection with each VPN plugin found
    Knm::Connection * con = 0;
    QString pluginError;
    KPluginInfo::List vpnServices = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QLatin1String("NetworkManagement/VpnUiPlugin")));
    foreach (const KPluginInfo &pi, vpnServices) {
        QString serviceType = pi.service()->property("X-NetworkManager-Services", QVariant::String).toString();
        VpnUiPlugin * vpnUi = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &pluginError );
        if (pluginError.isEmpty()) {
            QVariantList conArgs = vpnUi->importConnectionSettings(impFile);

            if (!conArgs.isEmpty()) {
                conArgs.insert(0, serviceType);        //VPN service
                con = mEditor->createConnection(false, Knm::Connection::Vpn, conArgs);
            }
            if (con) {
                kDebug() << "VPN Connection pointer is set, connection will be added.";

                if (con->scope() == Knm::Connection::User)
                    saveConnection(con);
                else
                    mSystemSettings->addConnection(con);

                updateServiceAndUi(con);
                delete vpnUi;
                break;
            }
        }
        delete vpnUi;
    }
    if (!con) {
        kDebug() << "VPN import failed";
        KMessageBox::error(this, i18n("Could not import VPN connection settings"), i18n("Error"), KMessageBox::Notify) ;
    }

}

void ManageConnectionWidget::exportClicked()
{
    QTreeWidgetItem * item = selectedItem();
    Knm::Connection * con = 0;
    QString connectionId = item->data(0, ConnectionIdRole).toString();
    Knm::Connection::Type type = (Knm::Connection::Type)item->data(0, ConnectionTypeRole).toUInt();
    if (connectionId.isEmpty()) {
        kDebug() << "selected item had no connectionId!";
        return;
    }

    Knm::Connection::Scope scope = (Knm::Connection::Scope)item->data(0, ConnectionScopeRole).toUInt();
    if (scope == Knm::Connection::User) {
        con = new Knm::Connection(QUuid(connectionId), type);
        loadConnection(con);
    } else {
        //find clicked connection from our connection list
        // FIXME: we should create a copy here like above instead of using the original.
        con = mConnections->findConnection(connectionId);
    }
    if (!con) {
        kWarning() << "Clicked connection with id" << connectionId << " could not be found in connection list!";
        return;
    }

    QString serviceType = static_cast<Knm::VpnSetting*>(con->setting(Knm::Setting::Vpn))->serviceType();
    QString pluginError;
    VpnUiPlugin * vpnUi = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &pluginError );
    if (pluginError.isEmpty()) {
        QString expFile = KFileDialog::getSaveFileName(KUser().homeDir().append("/" + vpnUi->suggestedFileName(con)),"",this,i18nc("File chooser dialog title for exporting VPN","Export VPN"));
        if (expFile.isEmpty()) {
            delete vpnUi;
            return;
        }

        vpnUi->exportConnectionSettings(con, expFile);

        KMessageBox::information(this, i18n("VPN connection successfully exported"), i18n("Success"), i18n("Do not show again"), KMessageBox::Notify);
    } else {
        KMessageBox::error(this, i18n("Could not export VPN connection settings"), i18n("Error"), KMessageBox::Notify);
    }

    delete vpnUi;
    return;
}

void ManageConnectionWidget::loadConnection(Knm::Connection *con)
{
    // restore the Connection if possible
    QString connectionFile(KStandardDirs::locateLocal("data",
                Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + QString(con->uuid())));
    connectionPersistence = new Knm::ConnectionPersistence(con, KSharedConfig::openConfig(connectionFile),
            (Knm::ConnectionPersistence::SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode());
    connectionPersistence->load();
}

void ManageConnectionWidget::saveConnection(Knm::Connection *con)
{
    // persist the Connection
    QString connectionFile = KStandardDirs::locateLocal("data",
        Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + QString(con->uuid()));

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
    KConfigGroup config(prefs->config(), QLatin1String("Connection_") + QString(con->uuid()));
    QStringList connectionIds = prefs->connections();

    // check if already present, we may be editing an existing Connection
    if (!connectionIds.contains(con->uuid())) {
        connectionIds << con->uuid();
        prefs->setConnections(connectionIds);
    }
    config.writeEntry("Name", name);
    config.writeEntry("Type", type);
    prefs->writeConfig();
}

void ManageConnectionWidget::gotSecrets(uint result)
{
    delete connectionPersistence;
    connectionPersistence = 0;
    editGotSecrets(result == Knm::ConnectionPersistence::EnumError::NoError, QLatin1String(""));
}

void ManageConnectionWidget::editClicked()
{
    //edit might be clicked on a system connection, in which case we need a connectionid for it
    QTreeWidgetItem *item = selectedItem();

    if (item) {
        Knm::Connection *con = NULL;

        QString connectionId = item->data(0, ConnectionIdRole).toString();
        Knm::Connection::Type type = (Knm::Connection::Type)item->data(0, ConnectionTypeRole).toUInt();
        if (connectionId.isEmpty()) {
            kDebug() << "selected item had no connectionId!";
            return;
        }

        Knm::Connection::Scope scope = (Knm::Connection::Scope)item->data(0, ConnectionScopeRole).toUInt();
        if (scope == Knm::Connection::User) {
            con = new Knm::Connection(QUuid(connectionId), type);
            loadConnection(con);

        } else
            //find clicked connection from our connection list
            // FIXME: we should create a copy here like above instead of using the original.
            con = mConnections->findConnection(connectionId);

        if (!con) {
            kWarning() << "Clicked connection with id" << connectionId << " could not be found in connection list!";
            return;
        }

        mEditConnection = con;
        oldScope = con->scope();
        if (con->hasSecrets()) {
            if (scope == Knm::Connection::System) {
                bool rep = mSystemSettings->getConnectionSecrets(con);

                if (!rep) {
                    KMessageBox::error(this, i18n("Connection edit option failed, make sure that NetworkManager is properly running."));
                    return;
                }

                kDebug() << "Get secrets " << rep;
            } else {
                connect(connectionPersistence, SIGNAL(loadSecretsResult(uint)), SLOT(gotSecrets(uint)));
                connectionPersistence->loadSecrets();
            }
        } else {
            kDebug() << "This connection has no secrets, good.";
            editGotSecrets(true, QString());
            if (scope == Knm::Connection::User) {
                delete connectionPersistence;
                connectionPersistence = 0;
            }
        }
    }
}

void ManageConnectionWidget::editGotSecrets(bool valid, const QString &errorMessage)
{
    if (!valid) {
        if (errorMessage.isEmpty())
            KMessageBox::error(this, i18n("Error"));
        else
            KMessageBox::error(this, errorMessage);

        return;
    }

    Knm::Connection *con = mEditConnection;
    if (!con)
        return;

    con = mEditor->editConnection(con); //starts editor window
    if (con) {
        if (oldScope == con->scope()) {
            if (con->scope() == Knm::Connection::User)
                saveConnection(con);
            else
                mSystemSettings->updateConnection(con->uuid().toString(), con);
        } else {
            if (con->scope() == Knm::Connection::User) {
                if (deleteConnection(con->uuid().toString(), Knm::Connection::System, con->type())) {
                    saveConnection(con);
                } else {
                    // FIXME: when changing one system connection to user scope con is a pointer to
                    // the connection, when changing from user scope to system, con is just a copy.
                    // Since we could not restore all the original settings restore at least
                    // the scope.
                    con->setScope(Knm::Connection::System);
                    restoreConnections();
                    mEditConnection = NULL;
                    return;
                }
            } else {
                if (mSystemSettings->addConnection(con)) {
                    deleteConnection(con->uuid().toString(), Knm::Connection::User, con->type());
                    mConnections->replaceConnection(con);
                } else {
                    restoreConnections();
                    mEditConnection = NULL;
                    return;
                }
            }
        }

        updateServiceAndUi(con);
    }
    mEditConnection = NULL;
}

void ManageConnectionWidget::addGotConnection(bool valid, const QString &errorMessage)
{
    if (!valid) {
        if (errorMessage.isEmpty())
            KMessageBox::error(this, i18n("Connection create operation failed."));
        else
            KMessageBox::error(this, errorMessage);

        return;
    }
}

bool ManageConnectionWidget::deleteConnection(QString id, Knm::Connection::Scope scope, Knm::Connection::Type type)
{
    // delete it
    if (scope == Knm::Connection::System)
        mSystemSettings->removeConnection(id);
    else {
        // remove secrets from wallet if using encrypted storage
        Knm::ConnectionPersistence::deleteSecrets(id);

        // delete everything related, like certificates
        QFile connFile(KStandardDirs::locateLocal("data",
                    Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + id));
        if (!connFile.exists()) {
        kDebug() << "Connection file not found: " << connFile.fileName();
        }

        Knm::Connection *con = new Knm::Connection(QUuid(id), type);
        connectionPersistence = new Knm::ConnectionPersistence(con, KSharedConfig::openConfig(connFile.fileName()),
            (Knm::ConnectionPersistence::SecretStorageMode)KNetworkManagerServicePrefs::self()->secretStorageMode());
        connectionPersistence->load();
        con->removeCertificates();
        delete(connectionPersistence);
        delete(con);

        // remove connection file
        connFile.remove();

        // remove from networkmanagerrc
        KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
        prefs->config()->deleteGroup(QLatin1String("Connection_") + id);

        QStringList connectionIds = prefs->connections();
        connectionIds.removeAll(id);
        prefs->setConnections(connectionIds);
        prefs->writeConfig();
    }

    // remove it from our hash
    mUuidItemHash.remove(id);

    updateServiceAndUi(id, scope);
    return true;
}

void ManageConnectionWidget::deleteClicked()
{
    QTreeWidgetItem * item = selectedItem();
    if ( !item ) {
        kDebug() << "delete clicked, but no selection!";
        return;
    }
    QString connectionId = item->data(0, ConnectionIdRole).toString();
    if (connectionId.isEmpty()) {
        kDebug() << "item to be deleted had no connectionId!";
        return;
    }
    KMessageBox::Options options;
    options |= KMessageBox::Dangerous;
    if ( KMessageBox::warningContinueCancel(this,
        i18nc("Warning message on attempting to delete a connection", "Do you really want to delete the connection '%1'?",item->data(0, Qt::DisplayRole).toString()),
        i18n("Confirm Delete"),
        KStandardGuiItem::del())
        == KMessageBox::Continue) {

        Knm::Connection::Scope conScope = (Knm::Connection::Scope) item->data(0, ConnectionScopeRole).toUInt();
        Knm::Connection::Type type = (Knm::Connection::Type)item->data(0, ConnectionTypeRole).toUInt();
        deleteConnection(connectionId, conScope, type);
    }
}

Knm::Connection::Type ManageConnectionWidget::connectionTypeForCurrentIndex() const
{
    Knm::Connection::Type t = Knm::Connection::Wireless;
    int i = mConnEditUi.tabWidget->currentIndex();
    switch (i) {
        case 0:
            t = Knm::Connection::Wired;
            break;
        case 1:
            t = Knm::Connection::Wireless;
            break;
        case 2:
            // HACK - tab 2 always reports Gsm despite containing both Gsm and Cdma
            t = Knm::Connection::Gsm;
            break;
        case 3:
            t = Knm::Connection::Vpn;
            break;
        case 4:
            t = Knm::Connection::Pppoe;
            break;
        default:
            break;
    }
    return t;
}

QTreeWidgetItem * ManageConnectionWidget::selectedItem() const
{
    kDebug();
    QTreeWidgetItem * item = 0;
    QTreeWidget * list = 0;
    if ( mConnEditUi.tabWidget->currentWidget() == mConnEditUi.tabWired ) {
        list = mConnEditUi.listWired;
    } else if ( mConnEditUi.tabWidget->currentWidget() == mConnEditUi.tabWireless ) {
        list = mConnEditUi.listWireless;
    } else if ( mConnEditUi.tabWidget->currentWidget() == mConnEditUi.tabCellular ) {
        list = mConnEditUi.listCellular;
    } else if ( mConnEditUi.tabWidget->currentWidget() == mConnEditUi.tabVpn ) {
        list = mConnEditUi.listVpn;
    } else if ( mConnEditUi.tabWidget->currentWidget() == mConnEditUi.tabPppoe ) {
        list = mConnEditUi.listPppoe;
    }
    if (list) {
        QList<QTreeWidgetItem*> selected = list->selectedItems();
        if (selected.count() == 1) {
            item = selected.first();
        }
    }
    return item;
}

void ManageConnectionWidget::load()
{
    KCModule::load();
}

void ManageConnectionWidget::save()
{
    KNetworkManagerServicePrefs::self()->writeConfig();
    KCModule::save();
}

void ManageConnectionWidget::tabChanged(int index)
{
    if (index == 3) {
        if ( !mVpnMenu ) {
            mVpnMenu = new QMenu(this);
            // foreach vpn service, add one of these
            KPluginInfo::List vpnServices = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QLatin1String("NetworkManagement/VpnUiPlugin")));
            foreach (const KPluginInfo &pi, vpnServices) {
                QString serviceType = pi.service()->property("X-NetworkManager-Services", QVariant::String).toString();
                QAction * vpnAction = new QAction(pi.name(), this);
                vpnAction->setData(serviceType);
                mVpnMenu->addAction(vpnAction);
            }
            connect(mVpnMenu, SIGNAL(triggered(QAction*)), SLOT(connectionTypeMenuTriggered(QAction*)));
            mConnEditUi.buttonSetVpn->addButton()->setMenu(mVpnMenu);
        }
        mConnEditUi.buttonSetVpn->addButton()->setEnabled(!mVpnMenu->isEmpty());
        mConnEditUi.buttonSetVpn->importButton()->setEnabled(!mVpnMenu->isEmpty());
        mConnEditUi.buttonSetVpn->importButton()->setVisible(!mVpnMenu->isEmpty());
        mConnEditUi.buttonSetVpn->exportButton()->setVisible(!mVpnMenu->isEmpty());
        connect(mConnEditUi.buttonSetVpn->importButton(),SIGNAL(clicked()),SLOT(importClicked()));
        connect(mConnEditUi.buttonSetVpn->exportButton(),SIGNAL(clicked()),SLOT(exportClicked()));
    }
}

void ManageConnectionWidget::connectionTypeMenuTriggered(QAction* action)
{
    // HACK - tab 2 always reports GSM, tab 3 always reports VPN.
    // NM uses plugins to handle different VPN types but has hardcoded different mobile broadband
    // types.  However we don't want to blow up the UI so we merge GSM and VPN into one tab.
    // Because of the inconsistent handling of sub-types, we need a hack here to figure out what to
    // pass to the editor widget.

    // If it is a cellular type, check the data() on the action for the real type
    // If it is a VPN type, keep Vpn, but use the data() on the action for the plugin
    Knm::Connection::Type tabType = connectionTypeForCurrentIndex();
    Knm::Connection *con = NULL;
    if (tabType == Knm::Connection::Gsm) {
        con = mEditor->createConnection(false, (Knm::Connection::Type)action->data().toUInt());
    } else if (tabType == Knm::Connection::Vpn) {
        QVariantList vl;
        vl << action->data();
        con = mEditor->createConnection(false, tabType, vl);
    }
    if (con) {
        kDebug() << "Connection pointer is set, connection will be added.";

        if (con->scope() == Knm::Connection::User)
            saveConnection(con);
        else
            mSystemSettings->addConnection(con);

        updateServiceAndUi(con);
    }
}

void ManageConnectionWidget::activeConnectionsChanged()
{
#if 0
    // indicate which connections are in use right now
    QStringList activeConnections = Solid::Control::NetworkManager::activeConnections();
    foreach (QString conn, activeConnections) {
        OrgFreedesktopNetworkManagerConnectionActiveInterface candidate(NM_DBUS_SERVICE,
                                                                        conn, QDBusConnection::systemBus(), 0);
        // do we own the connection?
        if (candidate.serviceName() == NM_DBUS_SERVICE_USER_SETTINGS) {
            // get its UUID from our service
            QDBusObjectPath connectionPath = candidate.connection();
            OrgFreedesktopNetworkManagerSettingsConnectionInterface connection(NM_DBUS_SERVICE_USER_SETTINGS, connectionPath.path(), QDBusConnection::systemBus());
            if (connection.isValid()) {
                QVariantMapMap settings = connection.GetSettings();
                QDBusError lastError = connection.lastError();
                if (lastError.isValid()) {
                    kDebug() << "Could not get settings for " << connectionPath.path();
                }
                QString connKey = QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME);
                if (settings.contains(connKey))
                {
                    QVariantMap connectionSetting = settings.value(connKey);
                    QString uuidKey = QLatin1String(NM_SETTING_CONNECTION_UUID);
                    QString typeKey = QLatin1String(NM_SETTING_CONNECTION_TYPE);
                    if (!connectionSetting.contains(uuidKey)) {
                        kDebug() << "Settings does not contain UUID!";
                    }
                    if (!connectionSetting.contains(typeKey)) {
                        kDebug() << "Settings does not contain UUID!";
                    }
                    QString uuid = connectionSetting.value(uuidKey).toString();
                    QString type = connectionSetting.value(typeKey).toString();
                    kDebug() << "Connection at " << connectionPath.path() << " has uuid '" << uuid << "' and type '" << type;
                    QTreeWidgetItem * item = mUuidItemHash.value(uuid);
                    if (item) {
                        kDebug() << "Setting last used text to Now";
                        item->setText(1, i18nc("Text for connection list entry that is currently in used", "Now"));
                    }
                } else {
                    kDebug() << "No" << QLatin1String(NM_SETTING_CONNECTION_SETTING_NAME) << "in settings from" << connectionPath.path() << ", keys: " << settings.keys();
                }
            } else {
                kDebug() << "Connection '" << connectionPath.path() << "' is not valid!";
            }
        }
    }
#endif
}

void ManageConnectionWidget::updateLastUsed()
{
    updateLastUsed(mConnEditUi.listWired);
    updateLastUsed(mConnEditUi.listWireless);
    updateLastUsed(mConnEditUi.listCellular);
    updateLastUsed(mConnEditUi.listVpn);
    updateLastUsed(mConnEditUi.listPppoe);

}

void ManageConnectionWidget::updateLastUsed(QTreeWidget * list)
{
    QTreeWidgetItemIterator it(list);
    while (*it) {
        QDateTime lastUsed = (*it)->data(0, ConnectionLastUsedRole).toDateTime();
        (*it)->setText(2, formatDateRelative(lastUsed));
        ++it;
    }
}

void ManageConnectionWidget::connectButtonSet(AddEditDeleteButtonSet* buttonSet, QTreeWidget* tree)
{
    buttonSet->setTree(tree);
    connect(buttonSet->addButton(), SIGNAL(clicked()), SLOT(addClicked()));
    connect(buttonSet->editButton(), SIGNAL(clicked()), SLOT(editClicked()));
    connect(buttonSet->deleteButton(), SIGNAL(clicked()), SLOT(deleteClicked()));
}

bool ManageConnectionWidget::event(QEvent *ev)
{
    if (ev->type() == QEvent::KeyPress) {
        int key = static_cast<QKeyEvent*>(ev)->key();

        if (key == Qt::Key_Delete) {
            deleteClicked();
            return true;
        }
    }

    return KCModule::event(ev);
}

void ManageConnectionWidget::updateServiceAndUi(Knm::Connection *con)
{
    updateServiceAndUi(con->uuid().toString(), con->scope());
}

void ManageConnectionWidget::updateServiceAndUi(QString id, Knm::Connection::Scope scope)
{
    // System connections will be updated anyschronously when NM
    // signals update
    if (scope == Knm::Connection::User) {
        // Inform kded module about changed connections
        QStringList l;
        l << id;
        mEditor->updateService(l);
    }

    // Update UI
    restoreConnections();
}
