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

#include "connectioneditor.h"

#include <nm-setting-cdma.h>
#include <nm-setting-gsm.h>
#include <nm-setting-wireless.h>

#include <QDateTime>
#include <QDBusInterface>
#include <QFile>
#include <QMenu>
#include <QUuid>

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
#include "connectionprefs.h"
#include "wiredpreferences.h"
#include "wirelesspreferences.h"
#include "cellularpreferences.h"
#include "pppoepreferences.h"
#include "vpnpreferences.h"

#define ConnectionIdRole 1812

K_PLUGIN_FACTORY( ConnectionEditorFactory, registerPlugin<ConnectionEditor>();)
K_EXPORT_PLUGIN( ConnectionEditorFactory( "kcm_knetworkmanager" ) )

ConnectionEditor::ConnectionEditor(QWidget *parent, const QVariantList &args)
: KCModule( ConnectionEditorFactory::componentData(), parent, args ), mCellularMenu(0), mVpnMenu(0)
{
    // depending on the contents of args, either show the general purpose connection editor dialog
    // or a dialog for creating and starting a single connection. When the user clicks an
    // unconfigured wireless network in the applet, this mode is used to get them connected as
    // easily as possible.
    // Likewise if a connection does not have the right secrets NM will ask for more secrets, so the
    // service will show the dialog
    
    //QVariantList args;
    //args << "newconnection" << "type=NM_SETTING_WIRELESS_SETTING_NAME" << "interface=" << "accesspoint=";

    if ( !args.isEmpty()) {
#if 0 // WIP
        // editconnection connectionId=id
        // newconnection type=... (NM types)
        // for type == 802-11-wireless: essid, interface object path, AP object path
        kDebug() << args;
        if (args[0].toString() == "newconnection") {
            if (args.count() > 1) {
                QString rawArg = args[1].toString();
                if (rawArg.startsWith("type=")) {
                    QString type = rawArg.section('=', 1, 1);
                    // this could all be pushed down into each ConnectionEditor subclass
                    if (type == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME) && args.count() > 2) {
                        QString interfaceUni;
                        QString accessPointUni;
                        for (int i = 2; i < args.count(); ++i) {
                            rawArg = args[i].toString();
                            QString arg = rawArg.section('=', 0, 0);
                            QString value = rawArg.section('=', 1, 1);
                            if (arg == QLatin1String("interface")) {
                                interfaceUni = value;
                            }
                            if (arg == QLatin1String("accesspoint")) {
                                accessPointUni = value;
                            }
                        }
                        KDialog configDialog(this);
                        QString connectionId = QUuid::createUuid().toString();
                        QVariantList args;
                        args << connectionId;
                        WirelessPreferences * wid = new WirelessPreferences(&configDialog, args);
                        configDialog.setMainWidget(wid);
                        QVBoxLayout * lay = new QVBoxLayout(this);
                        lay->addWidget(&configDialog);
                        setLayout(lay);
                        configDialog.show();
                        configDialog.exec();
                    }
                }
            }
        }
#endif
    } else {
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
    }
}

ConnectionEditor::~ConnectionEditor()
{
}

void ConnectionEditor::restoreConnections()
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
        KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("Connection_") + connectionId);
        if (!config.isValid()) {
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
        QTreeWidgetItem * item;
        if (type == QLatin1String("Wired")) {
            item = new QTreeWidgetItem(mConnEditUi.listWired, itemContents);
            wiredItems.append(item);
        } else if (type == QLatin1String("Wireless")) {
            item = new QTreeWidgetItem(mConnEditUi.listWireless, itemContents);
            wirelessItems.append(item);
        } else if (type == QLatin1String("Cellular")) {
            item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == QLatin1String("VPN")) {
            item = new QTreeWidgetItem(mConnEditUi.listVpn, itemContents);
            vpnItems.append(item);
        } else if (type == QLatin1String("PPPoE")) {
            item = new QTreeWidgetItem(mConnEditUi.listPppoe, itemContents);
            pppoeItems.append(item);
        }
        item->setData(0, ConnectionIdRole, connectionId);
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

void ConnectionEditor::updateTabStates()
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
    bool hasVpnPlugins = !KServiceTypeTrader::self()->query(QLatin1String("KNetworkManager/VpnUiPlugin")).isEmpty();
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

void ConnectionEditor::addClicked()
{
    // show connection settings widget for the active tab
    KDialog configDialog(this);
    QString connectionId = QUuid::createUuid().toString();
    QVariantList args;
    args << connectionId;
    if ( m_nextConnectionType.isValid()) {
        args << m_nextConnectionType;
        m_nextConnectionType = QVariant();
    }
    ConnectionPreferences * cprefs = editorForCurrentIndex(&configDialog, args);

    if (!cprefs) {
        return;
    }

    configDialog.setMainWidget(cprefs);
    if ( configDialog.exec() == QDialog::Accepted ) {
        cprefs->save();
        // add to the service prefs
        QString name = cprefs->connectionName();
        QString type = cprefs->connectionType();
        if (name.isEmpty() || type.isEmpty()) {
            kDebug() << "new connection has missing name ('" << name << "') or type ('" << type << "')";
        } else {
            KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
            KConfigGroup config(prefs->config(), QLatin1String("Connection_") + connectionId);
            QStringList connectionIds = prefs->connections();
            connectionIds << connectionId;
            prefs->setConnections(connectionIds);
            config.writeEntry("Name", cprefs->connectionName());
            config.writeEntry("Type", cprefs->connectionType());
            prefs->writeConfig();
            updateService();
            restoreConnections();
        }
    }
}

void ConnectionEditor::editClicked()
{
    //edit might be clicked on a system connection, in which case we need a connectionid for it
    KDialog configDialog(this);
    QTreeWidgetItem * item = selectedItem();
    if ( !item ) {
        kDebug() << "edit clicked, but no selection!";
        return;
    }
    QString connectionId = item->data(0, ConnectionIdRole).toString();
    if (connectionId.isEmpty()) {
        kDebug() << "selected item had no connectionId!";
        return;
    }

    QVariantList args;
    args << connectionId;

    KCModule * kcm = editorForCurrentIndex(&configDialog, args);

    if (kcm) {
        configDialog.setMainWidget(kcm);
        if (configDialog.exec() == QDialog::Accepted) {
            kcm->save();
            QStringList changed;
            changed << connectionId;
            updateService(changed);
            restoreConnections();
        }
    }
}

void ConnectionEditor::deleteClicked()
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
        updateService();
        restoreConnections();
    }
}

ConnectionPreferences * ConnectionEditor::editorForCurrentIndex(QWidget * parent, const QVariantList & args) const
{
    kDebug() << args;
    int i = mConnEditUi.tabWidget->currentIndex();
    ConnectionPreferences * wid = 0;
    switch (i) {
        case 0:
            wid = new WiredPreferences(parent, args);
            break;
        case 1:
            wid = new WirelessPreferences(parent, args);
            break;
        case 2:
            wid = new CellularPreferences(parent, args);
            break;
        case 3:
            wid = new VpnPreferences(parent, args);
            break;
        case 4:
            wid = new PppoePreferences(parent, args);
            break;
        default:
            break;
    }
    return wid;
}

QTreeWidgetItem * ConnectionEditor::selectedItem() const
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

void ConnectionEditor::load()
{
    KCModule::load();
}

void ConnectionEditor::save()
{
    KCModule::save();
}

void ConnectionEditor::updateService(const QStringList & changedConnections) const
{
    QDBusInterface iface(QLatin1String("org.kde.knetworkmanagerd"),
            QLatin1String("/Configuration"), 
            QLatin1String("org.kde.knetworkmanagerd"));
    iface.call(QLatin1String("configure"), changedConnections);
}

void ConnectionEditor::tabChanged(int index)
{
    if (index == 2) {
        if ( !mCellularMenu ) {
            mCellularMenu = new QMenu(this);
            QAction * gsmAction = new QAction(i18nc("Menu item for GSM connections", "GSM Connection"), this);
            gsmAction->setData(QVariant(NM_SETTING_GSM_SETTING_NAME));
            QAction * cdmaAction = new QAction(i18nc("Menu item for CDMA connections", "CDMA Connection"), this);
            cdmaAction->setData(QVariant(NM_SETTING_CDMA_SETTING_NAME));

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

void ConnectionEditor::connectionTypeMenuTriggered(QAction* action)
{
    m_nextConnectionType = action->data();
    kDebug() << m_nextConnectionType.toString();
    addClicked();
}
#include "connectioneditor.moc"
