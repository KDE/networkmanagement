/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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
#include <solid/control/networkmanager.h>
#include <solid/control/networkmodeminterface.h>

#include "connectiondbus.h"
#include "knmserviceprefs.h"
#include "connection.h"
#include "connectionlist.h"
#include "connectionprefs.h"
#include <tooltips.h>

#define ConnectionIdRole Qt::UserRole + 1
#define ConnectionTypeRole Qt::UserRole + 2
#define ConnectionLastUsedRole Qt::UserRole + 3
#define ConnectionScopeRole Qt::UserRole + 4

K_PLUGIN_FACTORY( ManageConnectionWidgetFactory, registerPlugin<ManageConnectionWidget>();)
K_EXPORT_PLUGIN( ManageConnectionWidgetFactory( "kcm_networkmanagement", "libknetworkmanager" ) )

ManageConnectionWidget::ManageConnectionWidget(QWidget *parent, const QVariantList &args)
: KCModule( ManageConnectionWidgetFactory::componentData(), parent, args ), mEditConnection(0), mCellularMenu(0), mVpnMenu(0), mEditor(new ConnectionEditor(this))
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

    mConnections = new ConnectionList(this);
    mSystemSettings = new NMDBusSettingsConnectionProvider(mConnections, this);

    connect(mSystemSettings, SIGNAL(getConnectionSecretsCompleted(bool, const QString &, QVariantMapMap)), this, SLOT(editGotSecrets(bool, const QString&, QVariantMapMap)) );
    connect(mSystemSettings, SIGNAL(addConnectionCompleted(bool, const QString &)), this, SLOT(addGotConnection(bool, const QString&)) );

    connect(mSystemSettings, SIGNAL(connectionsChanged()), this, SLOT(restoreConnections()));

    connectButtonSet(mConnEditUi.buttonSetWired, mConnEditUi.listWired);
    connectButtonSet(mConnEditUi.buttonSetWireless, mConnEditUi.listWireless);
    connectButtonSet(mConnEditUi.buttonSetCellular, mConnEditUi.listCellular);
    connectButtonSet(mConnEditUi.buttonSetVpn, mConnEditUi.listVpn);
    connectButtonSet(mConnEditUi.buttonSetPppoe, mConnEditUi.listPppoe);
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(updateTabStates()));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(updateTabStates()));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(activeConnectionsChanged()),
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
    if (QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.NetworkManager.KCModule"))) {
        QDBusConnection::sessionBus().registerObject(QLatin1String("/default"), this, QDBusConnection::ExportScriptableSlots);
    }
    mLastUsedTimer = new QTimer(this);
    connect(mLastUsedTimer, SIGNAL(timeout()), SLOT(updateLastUsed()));
    mLastUsedTimer->start(1000 * 60);

    setButtons(KCModule::Help | KCModule::Apply);
    mMobileConnectionWizard = 0;
}

ManageConnectionWidget::~ManageConnectionWidget()
{
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
        mSystemSettings->addConnection(con);
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
    //KNetworkManagerServicePrefs::self()->readConfig();
    //QStringList connectionIds = KNetworkManagerServicePrefs::self()->connections();
    //


    QList<QTreeWidgetItem *> wiredItems, wirelessItems, cellularItems, vpnItems, pppoeItems;
    foreach (const QString &connectionId, mConnections->connections()) {
        // look in the corresponding group
        // read name, type, last used
        kDebug() << connectionId;
        //KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("Connection_") + connectionId);
        //if (!config.exists()) {
        //    continue;
        //}

        Knm::Connection *con = mConnections->findConnection(connectionId);

        if (!con)
            continue;

        QString name = con->name();
        QString type = con->typeAsString(con->type());

        Knm::Ipv4Setting *setting = static_cast<Knm::Ipv4Setting*>(con->setting(Knm::Setting::Ipv4));
        if (setting)
            kDebug() << "IPv4 setting method" << setting->method();

        kDebug() << "Restoring connection " <<  name << type;

        if ( name.isEmpty() || type.isEmpty() ) {
            continue;
        }
        QDateTime lastUsed = con->timestamp();
        // add an item to the editor widget for that type
        QStringList itemContents;
        itemContents << name;
        itemContents << formatDateRelative(lastUsed);

        kDebug() << type << name << lastUsed;
        QTreeWidgetItem * item = 0;
        if (type == Knm::Connection::typeAsString(Knm::Connection::Wired)) {
            item = new QTreeWidgetItem(mConnEditUi.listWired, itemContents);
            wiredItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Wireless)) {
            item = new QTreeWidgetItem(mConnEditUi.listWireless, itemContents);
            wirelessItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Gsm)) {
            item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Cdma)) {
            item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Bluetooth)) {
            item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Vpn)) {
            item = new QTreeWidgetItem(mConnEditUi.listVpn, itemContents);
            vpnItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Pppoe)) {
            item = new QTreeWidgetItem(mConnEditUi.listPppoe, itemContents);
            pppoeItems.append(item);
        }

        if (item) {
            mUuidItemHash.insert(connectionId, item);
            item->setData(0, ConnectionIdRole, connectionId);
            item->setData(0, ConnectionTypeRole, Knm::Connection::typeFromString(type));
            item->setData(0, ConnectionLastUsedRole, lastUsed);
        }
    }
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
    foreach (Solid::Control::NetworkInterfaceNm09 * iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        switch (iface->type()) {
            case Solid::Control::NetworkInterfaceNm09::Ethernet:
                hasWired = true;
                break;
            case Solid::Control::NetworkInterfaceNm09::Wifi:
                hasWireless = true;
                break;
            case Solid::Control::NetworkInterfaceNm09::Modem: {
                const Solid::Control::ModemNetworkInterfaceNm09 * nmModemIface = qobject_cast<const Solid::Control::ModemNetworkInterfaceNm09 *>(iface);
                if (nmModemIface) {
                    switch(nmModemIface->subType()) {
                        case Solid::Control::ModemNetworkInterfaceNm09::Pots:
                            hasDsl = true;
                            break;
                        case Solid::Control::ModemNetworkInterfaceNm09::GsmUmts:
                        case Solid::Control::ModemNetworkInterfaceNm09::CdmaEvdo:
                        /* TODO: case Solid::Control::ModemNetworkInterfaceNm09::Lte: */
                            hasCellular = true;
                            break;
                    }
                }
            }
            case Solid::Control::NetworkInterfaceNm09::Bluetooth:
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
                con = mEditor->createConnection(false, mMobileConnectionWizard->type(), mMobileConnectionWizard->args(), true);
            } else { // fallback to old manual editing if something wrong happened with the wizard
                kDebug() << "Got error, falling back to old method";
                con = mEditor->createConnection(false, mMobileConnectionWizard->type());
            }
        }
    } else { // show connection settings widget for the active tab
         con = mEditor->createConnection(false, connectionTypeForCurrentIndex());
    }

    if (con) {
        // TODO: Check for scope and mUserUserSettings if necessary
        /*
        if (conScope == Knm::Connection::User)
            mUserSettings->addConnection(con);
        else
        */
        mSystemSettings->addConnection(con);

        //Enable this if connections do not appears in plasma-applet
        //mEditor->updateService();

        kDebug() << "Connection pointer is set, connection will be added.";
    }
    else
        kDebug() << "Connection pointer is not set, connection will not be added!";

    emit changed();
}

void ManageConnectionWidget::editClicked()
{
    //edit might be clicked on a system connection, in which case we need a connectionid for it
    QTreeWidgetItem *item = selectedItem();

    if (item) {
        Knm::Connection *con = 0;

        QString connectionId = item->data(0, ConnectionIdRole).toString();
        //Knm::Connection::Type type = (Knm::Connection::Type)item->data(0, ConnectionTypeRole).toUInt();
        if (connectionId.isEmpty()) {
            kDebug() << "selected item had no connectionId!";
            return;
        }

        //QVariantList args;
        //args << connectionId;

        //find clicked connection from our connection list
        con = mConnections->findConnection(connectionId);
        if (!con)
        {
            kWarning() << "Clicked connection with id" << connectionId << " could not be found in connection list!";
            return;
        }

        mEditConnection = con;
        if (con->hasSecrets())
        {
            bool rep = mSystemSettings->getConnectionSecrets(con);

            if (!rep)
            {
                KMessageBox::error(this, i18n("Connection edit option failed, make sure that NetworkManager is properly running."));
                return;
            }

            kDebug() << "Get secrets " << rep;
        }
        else
        {
            kDebug() << "This connection has no secrets, good.";
            editGotSecrets(true, QString(), QVariantMapMap());
        }
        //emit changed();
    }
}

void ManageConnectionWidget::editGotSecrets(bool valid, const QString &errorMessage, QVariantMapMap secrets)
{
    if (!valid)
    {
        if (errorMessage.isEmpty())
            KMessageBox::error(this, i18n("Error"));
        else
            KMessageBox::error(this, errorMessage);
    }

    Knm::Connection *copy = new Knm::Connection(mEditConnection);
    if (!secrets.empty()) {
        ConnectionDbus dbusConverter(copy);
        dbusConverter.fromDbusSecretsMap(secrets); //update secretSettings in connection
    }

    Knm::Connection *result = mEditor->editConnection(copy); //starts editor window
    if (result)
    {
        // TODO: Check for scope and mUserUserSettings if necessary
        /*
           if (conScope == Knm::Connection::User)
           mUserSettings->updateConnection(connectionId, result);
           else
           */
        mSystemSettings->updateConnection(copy->uuid().toString(), result);

        //Enable this if connections do not updated in plasma-applet
        //mEditor->updateService();
    }
    delete copy;
}

void ManageConnectionWidget::addGotConnection(bool valid, const QString &errorMessage)
{
    if (!valid)
    {
        if (errorMessage.isEmpty())
            KMessageBox::error(this, i18n("Connection create operation failed."));
        else
            KMessageBox::error(this, errorMessage);

        return;
    }
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

        // delete it
        // remove it from our hash
        mUuidItemHash.remove(connectionId);
        // remove secrets from wallet if using encrypted storage
        //Knm::ConnectionPersistence::deleteSecrets(connectionId);
        // remove connection file
        //QFile connFile(KStandardDirs::locateLocal("data",
        //            Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + connectionId));
        //if (!connFile.exists()) {
        //    kDebug() << "Connection file not found: " << connFile.fileName();
        //}
        //connFile.remove();

        // remove from networkmanagerrc
        //KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
        //prefs->config()->deleteGroup(QLatin1String("Connection_") + connectionId);

        //QStringList connectionIds = prefs->connections();
        //connectionIds.removeAll(connectionId);
        //prefs->setConnections(connectionIds);
        //prefs->writeConfig();


        //Knm::Connection::Scope conScope = (Knm::Connection::Scope) item->data(0, ConnectionScopeRole).toUInt();

        /*
        if (conScope == Knm::Connection::User)
            mUserSettings->removeConnection(connectionId);
        else
        */
        mSystemSettings->removeConnection(connectionId);

        //Enable this if connections is not removed from plasma-applet
        //mEditor->updateService();
    }
    emit changed();
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
    if (tabType == Knm::Connection::Gsm) {
        mEditor->createConnection(false, (Knm::Connection::Type)action->data().toUInt());
    } else if (tabType == Knm::Connection::Vpn) {
        QVariantList vl;
        vl << action->data();
        Knm::Connection *con = mEditor->createConnection(false, tabType, vl);

        if (con) {
            mSystemSettings->addConnection(con);
            emit changed();
        }
    }
}


void ManageConnectionWidget::activeConnectionsChanged()
{
#if 0
    // indicate which connections are in use right now
    QStringList activeConnections = Solid::Control::NetworkManagerNm09::activeConnections();
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
