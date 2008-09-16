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
#include <KPluginFactory>
#include <KStandardDirs>

#include "knmserviceprefs.h"

K_PLUGIN_FACTORY( ConnectionEditorFactory, registerPlugin<ConnectionEditor>();)
K_EXPORT_PLUGIN( ConnectionEditorFactory( "kcm_knetworkmanager" ) )

ConnectionEditor::ConnectionEditor(QWidget *parent, const QVariantList &args)
: KCModule( ConnectionEditorFactory::componentData(), parent, args )
{
    QWidget * contents = new QWidget(this);
    mConnEditUi.setupUi(contents);
    KNetworkManagerServicePrefs::instance(KStandardDirs::locate("config",
                QLatin1String("knetworkmanagerrc")));
    connect(mConnEditUi.addWired, SIGNAL(clicked()), SLOT(addWiredClicked()));
    connect(mConnEditUi.addWireless, SIGNAL(clicked()), SLOT(addWirelessClicked()));
    connect(mConnEditUi.addCellular, SIGNAL(clicked()), SLOT(addCellularClicked()));
    connect(mConnEditUi.addVpn, SIGNAL(clicked()), SLOT(addVpnClicked()));
    connect(mConnEditUi.addPppoe, SIGNAL(clicked()), SLOT(addPppoeClicked()));
    connect(mConnEditUi.editWired, SIGNAL(clicked()), SLOT(editWiredClicked()));
    connect(mConnEditUi.editWireless, SIGNAL(clicked()), SLOT(editWirelessClicked()));
    connect(mConnEditUi.editCellular, SIGNAL(clicked()), SLOT(editCellularClicked()));
    connect(mConnEditUi.editVpn, SIGNAL(clicked()), SLOT(editVpnClicked()));
    connect(mConnEditUi.editPppoe, SIGNAL(clicked()), SLOT(editPppoeClicked()));
    connect(mConnEditUi.deleteWired, SIGNAL(clicked()), SLOT(deleteWiredClicked()));
    connect(mConnEditUi.deleteWireless, SIGNAL(clicked()), SLOT(deleteWirelessClicked()));
    connect(mConnEditUi.deleteCellular, SIGNAL(clicked()), SLOT(deleteCellularClicked()));
    connect(mConnEditUi.deleteVpn, SIGNAL(clicked()), SLOT(deleteVpnClicked()));
    connect(mConnEditUi.deletePppoe, SIGNAL(clicked()), SLOT(deletePppoeClicked()));
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
        if (type == QLatin1String("Wired")) {
            QTreeWidgetItem * item = new QTreeWidgetItem(mConnEditUi.listWired, itemContents);
            wiredItems.append(item);
        } else if (type == QLatin1String("Wireless")) {
            QTreeWidgetItem * item = new QTreeWidgetItem(mConnEditUi.listWireless, itemContents);
            wirelessItems.append(item);
        } else if (type == QLatin1String("Cellular")) {
            QTreeWidgetItem * item = new QTreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == QLatin1String("VPN")) {
            QTreeWidgetItem * item = new QTreeWidgetItem(mConnEditUi.listVpn, itemContents);
            vpnItems.append(item);
        } else if (type == QLatin1String("PPPoE")) {
            QTreeWidgetItem * item = new QTreeWidgetItem(mConnEditUi.listPppoe, itemContents);
            pppoeItems.append(item);
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
}

void ConnectionEditor::addWiredClicked()
{
    KDialog configDialog(this);
    QStringList args;
    args << "testconfigxml";
    KCModuleProxy kcm(QLatin1String("kcm_networkmanager_wired"), &configDialog, args);
    configDialog.setMainWidget(&kcm);
    configDialog.exec();

}

void ConnectionEditor::editWiredClicked()
{

}
void ConnectionEditor::deleteWiredClicked()
{

}

void ConnectionEditor::addWirelessClicked()
{
}

void ConnectionEditor::editWirelessClicked()
{

}
void ConnectionEditor::deleteWirelessClicked()
{

}

void ConnectionEditor::addCellularClicked()
{
}

void ConnectionEditor::editCellularClicked()
{

}
void ConnectionEditor::deleteCellularClicked()
{

}

void ConnectionEditor::addVpnClicked()
{
}

void ConnectionEditor::editVpnClicked()
{

}
void ConnectionEditor::deleteVpnClicked()
{

}

void ConnectionEditor::addPppoeClicked()
{
}

void ConnectionEditor::editPppoeClicked()
{

}
void ConnectionEditor::deletePppoeClicked()
{

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
