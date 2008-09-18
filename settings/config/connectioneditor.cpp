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

#include <QDateTime>

#include <KCModuleProxy>
#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>
#include <KRandom>
#include <KStandardDirs>

#include "knmserviceprefs.h"
#include "connectionprefs.h"
#include "wiredpreferences.h"
#include "wirelesspreferences.h"
#include "cellularpreferences.h"
#include "pppoepreferences.h"

#define ConnectionIdRole 1812

K_PLUGIN_FACTORY( ConnectionEditorFactory, registerPlugin<ConnectionEditor>();)
K_EXPORT_PLUGIN( ConnectionEditorFactory( "kcm_knetworkmanager" ) )

ConnectionEditor::ConnectionEditor(QWidget *parent, const QVariantList &args)
: KCModule( ConnectionEditorFactory::componentData(), parent, args )
{
    mConnEditUi.setupUi(this);
    mConnEditUi.tabWidget->setTabEnabled(3, false);
    KNetworkManagerServicePrefs::instance(KStandardDirs::locate("config",
                QLatin1String("knetworkmanagerrc")));
    connect(mConnEditUi.addConnection, SIGNAL(clicked()), SLOT(addClicked()));
    connect(mConnEditUi.editConnection, SIGNAL(clicked()), SLOT(editClicked()));
    connect(mConnEditUi.deleteConnection, SIGNAL(clicked()), SLOT(deleteClicked()));
    restoreConnections();
}

ConnectionEditor::~ConnectionEditor()
{
}

void ConnectionEditor::restoreConnections()
{
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
}

void ConnectionEditor::addClicked()
{
    // show connection settings widget for the active tab
    KDialog configDialog(this);
    QString connectionId = KRandom::randomString(10);
    QVariantList args;
    args << connectionId;
    ConnectionPreferences * cprefs = editorForCurrentIndex(&configDialog, args);

    if (!cprefs) {
        return;
    }

    configDialog.setMainWidget(cprefs);
    if ( configDialog.exec() == QDialog::Accepted ) {
        cprefs->save();
        // add to the service prefs
        KNetworkManagerServicePrefs * prefs = KNetworkManagerServicePrefs::self();
        QStringList connectionIds = prefs->connections();
        connectionIds << connectionId;
        prefs->setConnections(connectionIds);
        KConfigGroup config(prefs->config(), QLatin1String("Connection_") + connectionId);
        config.writeEntry("Name", cprefs->connectionName());
        config.writeEntry("Type", cprefs->connectionType());
        prefs->writeConfig();
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
            kDebug() << "VPN connections are not yet supported";
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

#include "connectioneditor.moc"
