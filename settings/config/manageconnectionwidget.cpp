/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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
#include <nm-setting-gsm.h>

#include <QDBusConnection>
#include <QDateTime>
#include <QFile>
#include <QMenu>

#include <KCModuleProxy>
#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPluginInfo>
#include <KRandom>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>

#include "knmserviceprefs.h"
#include "connection.h"
#include "connectionprefs.h"

#define ConnectionIdRole 1812
#define ConnectionTypeRole 1066

K_PLUGIN_FACTORY( ManageConnectionWidgetFactory, registerPlugin<ManageConnectionWidget>();)
K_EXPORT_PLUGIN( ManageConnectionWidgetFactory( "kcm_knetworkmanager" ) )

ManageConnectionWidget::ManageConnectionWidget(QWidget *parent, const QVariantList &args)
: KCModule( ManageConnectionWidgetFactory::componentData(), parent, args ), mCellularMenu(0), mVpnMenu(0), mEditor(new ConnectionEditor(this))
{
    connect(mEditor, SIGNAL(connectionsChanged()), this, SLOT(restoreConnections()));

    mConnEditUi.setupUi(this);
    KNetworkManagerServicePrefs::instance(KStandardDirs::locateLocal("config",
                QLatin1String("knetworkmanagerrc")));
    connect(mConnEditUi.addConnection, SIGNAL(clicked()), SLOT(addClicked()));
    connect(mConnEditUi.editConnection, SIGNAL(clicked()), SLOT(editClicked()));
    connect(mConnEditUi.deleteConnection, SIGNAL(clicked()), SLOT(deleteClicked()));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(updateTabStates()));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(updateTabStates()));
    connect(mConnEditUi.tabWidget, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));
    restoreConnections();
    if (QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.NetworkManager.KCModule"))) {
        QDBusConnection::sessionBus().registerObject(QLatin1String("/default"), this, QDBusConnection::ExportScriptableSlots);
    }
}

ManageConnectionWidget::~ManageConnectionWidget()
{
}

void ManageConnectionWidget::createConnection(const QString &connectionType, const QVariantList &args)
{
    mEditor->addConnection(false, Knm::Connection::typeFromString(connectionType), args);
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
    foreach (QString connectionId, connectionIds) {
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
        // TODO: replace date formatting with something relative to 'now'
        itemContents << name;
        if (lastUsed.isValid()) {
            itemContents << KGlobal::locale()->formatDateTime(lastUsed, KLocale::FancyLongDate);
        } else {
            itemContents << i18nc("Label for last used time for a"
                    "network connection that has never been used", "Never");
        }
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
            item->setData(0, ConnectionIdRole, connectionId);
            item->setData(0, ConnectionTypeRole, Knm::Connection::typeFromString(type));
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
    if (KServiceTypeTrader::self()->query(QLatin1String("KNetworkManager/VpnUiPlugin")).isEmpty()) {
        mConnEditUi.tabWidget->setTabEnabled(3, false);
        mConnEditUi.tabWidget->setTabToolTip(3, i18nc("Tooltip for disabled tab when no VPN plugins are installed", "No VPN plugins were found"));
    } else {
        mConnEditUi.tabWidget->setTabEnabled(3, true);
    }
    mConnEditUi.tabWidget->setTabEnabled(4, (hasDsl || mConnEditUi.listPppoe->topLevelItemCount()));
}

void ManageConnectionWidget::addClicked()
{
    // show connection settings widget for the active tab
    mEditor->addConnection(false, connectionTypeForCurrentIndex());
}

void ManageConnectionWidget::editClicked()
{
    //edit might be clicked on a system connection, in which case we need a connectionid for it
    QTreeWidgetItem * item = selectedItem();
    if ( !item ) {
        kDebug() << "edit clicked, but no selection!";
        return;
    }
    QString connectionId = item->data(0, ConnectionIdRole).toString();
    Knm::Connection::Type type = (Knm::Connection::Type)item->data(0, ConnectionTypeRole).toUInt();
    kDebug() << connectionId << type;
    if (connectionId.isEmpty()) {
        kDebug() << "selected item had no connectionId!";
        return;
    }

    QVariantList args;
    args << connectionId;

    mEditor->editConnection(type, args);
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
    if ( KMessageBox::warningYesNo(this, i18nc("Warning message on attempting to delete a connection", "Do you really want to delete the connection '%1'?",item->data(0, Qt::DisplayRole).toString()), i18n("Confirm delete") /*, QLatin1String("ConfirmDeleteConnection")*/) == KMessageBox::Yes) {
        // delete it
        // remove connection file
        QFile connFile(KStandardDirs::locateLocal("data",
                    QLatin1String("knetworkmanager/connections/") + connectionId));
        if (!connFile.exists()) {
            kDebug() << "Connection file not found: " << connFile.fileName();
        }
        connFile.remove();

        // remove from knetworkmanagerrc
        KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
        prefs->config()->deleteGroup(QLatin1String("Connection_") + connectionId);

        QStringList connectionIds = prefs->connections();
        connectionIds.removeAll(connectionId);
        prefs->setConnections(connectionIds);
        prefs->writeConfig();
        mEditor->updateService();
        restoreConnections();
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
        }
        mConnEditUi.addConnection->setMenu(mCellularMenu);
    } else if (index == 3) {
        if ( !mVpnMenu ) {
            mVpnMenu = new QMenu(this);
            // foreach vpn service, add one of these
            KPluginInfo::List vpnServices = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QLatin1String("KNetworkManager/VpnUiPlugin")));
            foreach (KPluginInfo pi, vpnServices) {
                QAction * vpnAction = new QAction(pi.name(), this);
                vpnAction->setData(QVariant(pi.pluginName()));
                mVpnMenu->addAction(vpnAction);
            }
        }
        if (mVpnMenu->isEmpty()) {
            mConnEditUi.addConnection->setEnabled(false);
        } else {
            mConnEditUi.addConnection->setEnabled(true);
            connect(mVpnMenu, SIGNAL(triggered(QAction*)), SLOT(connectionTypeMenuTriggered(QAction*)));
            mConnEditUi.addConnection->setMenu(mVpnMenu);
        }
    } else {
        mConnEditUi.addConnection->setEnabled(true);
        mConnEditUi.addConnection->setMenu(0);
    }
}

void ManageConnectionWidget::connectionTypeMenuTriggered(QAction* action)
{
    mEditor->addConnection(false, (Knm::Connection::Type)action->data().toUInt());
}

#include "manageconnectionwidget.moc"
