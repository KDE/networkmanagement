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
#include <solid/control/networkinterface.h>

#include "knmserviceprefs.h"
#include "connection.h"
#include "connectionpersistence.h"
#include "connectionprefs.h"
#include <tooltips.h>

#define ConnectionIdRole 1812
#define ConnectionTypeRole 1066
#define ConnectionLastUsedRole 1848

K_PLUGIN_FACTORY( ManageConnectionWidgetFactory, registerPlugin<ManageConnectionWidget>();)
K_EXPORT_PLUGIN( ManageConnectionWidgetFactory( "kcm_networkmanagement", "libknetworkmanager" ) )

ManageConnectionWidget::ManageConnectionWidget(QWidget *parent, const QVariantList &args)
: KCModule( ManageConnectionWidgetFactory::componentData(), parent, args ), mCellularMenu(0), mVpnMenu(0), mEditor(new ConnectionEditor(this))
{
    connect(mEditor, SIGNAL(connectionsChanged()), this, SLOT(restoreConnections()));

    mConnEditUi.setupUi(this);

    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
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
            SLOT(editItem(QTreeWidgetItem*)));
    connect(mConnEditUi.listWireless, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editItem(QTreeWidgetItem*)));
    connect(mConnEditUi.listCellular, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editItem(QTreeWidgetItem*)));
    connect(mConnEditUi.listVpn, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editItem(QTreeWidgetItem*)));
    connect(mConnEditUi.listPppoe, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            SLOT(editItem(QTreeWidgetItem*)));

    restoreConnections();
    if (QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.NetworkManager.KCModule"))) {
        QDBusConnection::sessionBus().registerObject(QLatin1String("/default"), this, QDBusConnection::ExportScriptableSlots);
    }
    mLastUsedTimer = new QTimer(this);
    connect(mLastUsedTimer, SIGNAL(timeout()), SLOT(updateLastUsed()));
    mLastUsedTimer->start(1000 * 60);

    setButtons(KCModule::Help | KCModule::Apply);
}

ManageConnectionWidget::~ManageConnectionWidget()
{
}

void ManageConnectionWidget::createConnection(const QString &connectionType, const QVariantList &args)
{
    mEditor->addConnection(false, Knm::Connection::typeFromString(connectionType), args);
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
    //clean up the lists
    mConnEditUi.listWired->clear();
    mConnEditUi.listWireless->clear();
    mConnEditUi.listCellular->clear();
    mConnEditUi.listVpn->clear();
    mConnEditUi.listPppoe->clear();

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
        QString type = config.readEntry("Type", QString());
        if ( name.isEmpty() || type.isEmpty() ) {
            continue;
        }
        QDateTime lastUsed = config.readEntry("LastUsed", QDateTime());
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
    // show connection settings widget for the active tab
    mEditor->addConnection(false, connectionTypeForCurrentIndex());
    emit changed();
}

void ManageConnectionWidget::editClicked()
{
    //edit might be clicked on a system connection, in which case we need a connectionid for it
    QTreeWidgetItem * item = selectedItem();
    editItem(item);
    emit changed();
}

void ManageConnectionWidget::editItem(QTreeWidgetItem * item)
{
    if (item) {
        QString connectionId = item->data(0, ConnectionIdRole).toString();
        Knm::Connection::Type type = (Knm::Connection::Type)item->data(0, ConnectionTypeRole).toUInt();
        if (connectionId.isEmpty()) {
            kDebug() << "selected item had no connectionId!";
            return;
        }

        QVariantList args;
        args << connectionId;

        mEditor->editConnection(type, args);
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
        // remove connection file
        QFile connFile(KStandardDirs::locateLocal("data",
                    Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + connectionId));
        if (!connFile.exists()) {
            kDebug() << "Connection file not found: " << connFile.fileName();
        }
        connFile.remove();

        // remove from networkmanagerrc
        KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
        prefs->config()->deleteGroup(QLatin1String("Connection_") + connectionId);

        QStringList connectionIds = prefs->connections();
        connectionIds.removeAll(connectionId);
        prefs->setConnections(connectionIds);
        prefs->writeConfig();
        mEditor->updateService();
        restoreConnections();
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
    if (index == 2) {
        if ( !mCellularMenu ) {
            mCellularMenu = new QMenu(this);
            QAction * gsmAction = new QAction(i18nc("Menu item for GSM connections", "GSM Connection"), this);
            gsmAction->setData(Knm::Connection::Gsm);
            QAction * cdmaAction = new QAction(i18nc("Menu item for CDMA connections", "CDMA Connection"), this);
            cdmaAction->setData(Knm::Connection::Cdma);

            mCellularMenu->addAction(gsmAction);
            mCellularMenu->addAction(cdmaAction);
            connect(mCellularMenu, SIGNAL(triggered(QAction*)), SLOT(connectionTypeMenuTriggered(QAction*)));
            mConnEditUi.buttonSetCellular->addButton()->setMenu(mCellularMenu);
        }
    } else if (index == 3) {
        if ( !mVpnMenu ) {
            mVpnMenu = new QMenu(this);
            // foreach vpn service, add one of these
            KPluginInfo::List vpnServices = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QLatin1String("NetworkManagement/VpnUiPlugin")));
            foreach (const KPluginInfo &pi, vpnServices) {
                QAction * vpnAction = new QAction(pi.name(), this);
                vpnAction->setData(QVariant(pi.pluginName()));
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
        mEditor->addConnection(false, (Knm::Connection::Type)action->data().toUInt());
    } else if (tabType == Knm::Connection::Vpn) {
        QVariantList vl;
        vl << action->data();
        mEditor->addConnection(false, tabType, vl);
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
        (*it)->setText(1, formatDateRelative(lastUsed));
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

