/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2011-2012 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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
#include "treewidgetitem.h"

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
#include <solid/control/networkmodeminterface.h>

#include "connectiondbus.h"
#include "knmserviceprefs.h"
#include "../../libs/internals/paths.h"
#include "connection.h"
#include "connectionlist.h"
#include "connectionprefs.h"
#include "vpnuiplugin.h"
#include "settings/vpn.h"
#include "../../plasma_nm_version.h"

#include <tooltips.h>

#define ConnectionNameColumn 0
#define ConnectionLastUsedColumn 1
#define ConnectionStateColumn 2

K_PLUGIN_FACTORY( ManageConnectionWidgetFactory, registerPlugin<ManageConnectionWidget>();)
K_EXPORT_PLUGIN( ManageConnectionWidgetFactory( "kcm_networkmanagement", "libknetworkmanager" ) )

ManageConnectionWidget::ManageConnectionWidget(QWidget *parent, const QVariantList &args)
: KCModule( ManageConnectionWidgetFactory::componentData(), parent, args ), mEditConnection(0), mWiredMenu(0), mWirelessMenu(0), mCellularMenu(0), mVpnMenu(0), mEditor(new ConnectionEditor(this))
{
    KAboutData* ab = new KAboutData("kcm_networkmanagement", "networkmanagement", ki18n("Manage Connections"), plasmaNmVersion,
        ki18n("Manage Connections Control Panel Module"),
        KAboutData::License_GPL, ki18n("(c) 2008-2009 Will Stephenson<p>(c) 2011-2012 Lamarque V. Souza</p>"));

    ab->addAuthor(ki18n("Lamarque V. Souza"), ki18n("Developer and Maintainer"), "lamarque@kde.org");
    ab->addAuthor(ki18n("Ilia Kats"), ki18n("Developer"));
    ab->addAuthor(ki18n("Rajeesh K Nambiar"), ki18n("Developer"));
    ab->addAuthor(ki18n("Will Stephenson"), ki18n("Developer"), "wstephenson@kde.org");
    ab->addAuthor(ki18n("Sebastian Kügler"), ki18n("Developer"), "sebas@kde.org");
    setAboutData(ab);

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

    connect(mSystemSettings, SIGNAL(getConnectionSecretsCompleted(bool,QString,QString)), this, SLOT(editGotSecrets(bool,QString,QString)) );
    connect(mSystemSettings, SIGNAL(addConnectionCompleted(bool,QString)), this, SLOT(addGotConnection(bool,QString)) );

    connect(mSystemSettings, SIGNAL(connectionsChanged()), this, SLOT(restoreConnections()));

    connectButtonSet(mConnEditUi.buttonSetWired, mConnEditUi.listWired);
    mWiredMenu = new QMenu(this);
    QAction * action = new QAction(i18nc("Like in 'add wired connection'", "Wired"), this);
    action->setData(false);
    mWiredMenu->addAction(action);
    action = new QAction(i18nc("Like in 'add shared connection'", "Shared"), this);
    action->setData(true);
    mWiredMenu->addAction(action);
    connect(mWiredMenu, SIGNAL(triggered(QAction*)), SLOT(connectionTypeMenuTriggered(QAction*)));
    mConnEditUi.buttonSetWired->addButton()->setMenu(mWiredMenu);

    connectButtonSet(mConnEditUi.buttonSetWireless, mConnEditUi.listWireless);
    mWirelessMenu = new QMenu(this);
    action = new QAction(i18nc("Like in 'add wireless connection'", "Wireless"), this);
    action->setData(false);
    mWirelessMenu->addAction(action);
    action = new QAction(i18nc("Like in 'add shared connection'", "Shared"), this);
    action->setData(true);
    mWirelessMenu->addAction(action);
    connect(mWirelessMenu, SIGNAL(triggered(QAction*)), SLOT(connectionTypeMenuTriggered(QAction*)));
    mConnEditUi.buttonSetWireless->addButton()->setMenu(mWirelessMenu);

    connectButtonSet(mConnEditUi.buttonSetCellular, mConnEditUi.listCellular);
    connectButtonSet(mConnEditUi.buttonSetVpn, mConnEditUi.listVpn);
    connectButtonSet(mConnEditUi.buttonSetPppoe, mConnEditUi.listPppoe);
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceAdded(QString)),
            SLOT(updateTabStates()));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceRemoved(QString)),
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

    if (!Solid::Control::NetworkManagerNm09::isWirelessEnabled()) {
        mConnEditUi.tabWidget->setCurrentIndex(0);
    }

    if (QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.NetworkManager.KCModule"))) {
        QDBusConnection::sessionBus().registerObject(QLatin1String("/default"), this, QDBusConnection::ExportScriptableSlots);
    }
    mLastUsedTimer = new QTimer(this);
    connect(mLastUsedTimer, SIGNAL(timeout()), SLOT(updateLastUsed()));
    mLastUsedTimer->start(1000 * 60);

    setButtons(KCModule::Help | KCModule::Apply);
    mMobileConnectionWizard = 0;

    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);
}

ManageConnectionWidget::~ManageConnectionWidget()
{
    //HACK: don't destroy until NMDBusSettingsConnectionProvider returns
    //(prevents crashes when closing kcmshell too fast after adding/editing
    //a connection, as this is then deleted from another thread)
    usleep(100000);
    QDBusConnection::sessionBus().unregisterService(QLatin1String("org.kde.NetworkManager.KCModule"));
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
    mUuidItemHash.clear();
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
        itemContents << QString();

        kDebug() << type << name << lastUsed;
        TreeWidgetItem * item = 0;
        if (type == Knm::Connection::typeAsString(Knm::Connection::Wired)) {
            item = new TreeWidgetItem(mConnEditUi.listWired, itemContents);
            wiredItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Wireless)) {
            item = new TreeWidgetItem(mConnEditUi.listWireless, itemContents);
            wirelessItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Gsm)) {
            item = new TreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Cdma)) {
            item = new TreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Bluetooth)) {
            item = new TreeWidgetItem(mConnEditUi.listCellular, itemContents);
            cellularItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Vpn)) {
            item = new TreeWidgetItem(mConnEditUi.listVpn, itemContents);
            vpnItems.append(item);
        } else if (type == Knm::Connection::typeAsString(Knm::Connection::Pppoe)) {
            item = new TreeWidgetItem(mConnEditUi.listPppoe, itemContents);
            pppoeItems.append(item);
        }

        if (item) {
            mUuidItemHash.insert(connectionId, item);
            item->setData(0, TreeWidgetItem::ConnectionIdRole, connectionId);
            item->setData(0, TreeWidgetItem::ConnectionLastUsedRole, lastUsed);
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

    activeConnectionsChanged();

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
                        case Solid::Control::ModemNetworkInterfaceNm09::Lte:
                            hasCellular = true;
                            break;
                        case Solid::Control::ModemNetworkInterfaceNm09::None:
                            kWarning() << "Unhandled modem sub type: Solid::Control::ModemNetworkInterfaceNm09::None";
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
        kDebug() << "GSM tab selected, launching mobile connection wizard...";

        delete mMobileConnectionWizard;
        mMobileConnectionWizard = new MobileConnectionWizard();

        if (mMobileConnectionWizard->exec() == QDialog::Accepted) {
            if (mMobileConnectionWizard->getError() == MobileProviders::Success) {
                kDebug() << "Got no error, creating connection";
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
        mSystemSettings->addConnection(con);
        emit changed();
    } else {
        kDebug() << "Connection pointer is not set, connection will not be added!";
    }
}

void ManageConnectionWidget::importClicked()
{
    //Get the file from which connection is to be imported
    QString impFile = KFileDialog::getOpenFileName(KUser().homeDir(),mSupportedExtns,this,i18nc("File chooser dialog title for importing VPN","Import VPN connection settings"));
    if (impFile.isEmpty()) {
        return;
    }

    //Try to import the connection with each VPN plugin found
    Knm::Connection * con = 0;
    QString pluginError;
    QString vpnErrorMessage;
    VpnUiPlugin::ErrorType vpnError = VpnUiPlugin::NoError;
    KPluginInfo::List vpnServices = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QLatin1String("NetworkManagement/VpnUiPlugin")));
    foreach (const KPluginInfo &pi, vpnServices) {
        QString serviceType = pi.service()->property("X-NetworkManager-Services", QVariant::String).toString();
        VpnUiPlugin * vpnUi = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &pluginError );
        if (pluginError.isEmpty()) {
            vpnError = VpnUiPlugin::NoError;
            vpnErrorMessage.clear();
            QVariantList conArgs = vpnUi->importConnectionSettings(impFile);
            if (conArgs.isEmpty()) {
                vpnError = vpnUi->lastError();
                vpnErrorMessage = vpnUi->lastErrorMessage();
            } else {
                conArgs.insert(0, serviceType);        //VPN service
                con = mEditor->createConnection(false, Knm::Connection::Vpn, conArgs);

                if (con) {
                    kDebug() << "VPN Connection pointer is set, connection will be added.";
                    // Assuming VPN secrets are always AgentOwned.
                    mSystemSettings->addConnection(con);
                    emit changed();
                    delete vpnUi;
                    break;
                }

                // User has cancelled the create connection dialog.
                if (vpnError == VpnUiPlugin::NoError) {
                    delete vpnUi;
                    break;
                }
            }
        } else {
            kWarning() << "Error loading vpn plugin for" << pi.name() << ":" << pluginError;
        }
        delete vpnUi;
    }

    switch (vpnError) {
    case VpnUiPlugin::NoError:
        break;

    case VpnUiPlugin::NotImplemented:
        KMessageBox::error(this, i18n("None of the supported plugins implement importing operation for file %1.", impFile), i18n("Error importing VPN connection settings")) ;
        break;

    case VpnUiPlugin::Error:
        kDebug() << "VPN import failed";
        KMessageBox::error(this, vpnErrorMessage, i18n("Error importing VPN connection settings")) ;
        break;
    }
}

void ManageConnectionWidget::exportClicked()
{
    QTreeWidgetItem * item = selectedItem();
    Knm::Connection * con = 0;
    QString connectionId = item->data(0, TreeWidgetItem::ConnectionIdRole).toString();
    if (connectionId.isEmpty()) {
        kDebug() << "selected item had no connectionId!";
        return;
    }

    //find clicked connection from our connection list
    con = mConnections->findConnection(connectionId);
    if (!con) {
        kWarning() << "Clicked connection with id" << connectionId << " could not be found in connection list!";
        return;
    }

    QString serviceType = static_cast<Knm::VpnSetting*>(con->setting(Knm::Setting::Vpn))->serviceType();
    QString pluginError;
    VpnUiPlugin * vpnUi = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &pluginError );
    if (pluginError.isEmpty()) {
        QString expFile = KFileDialog::getSaveFileName(KUser().homeDir().append("/" + vpnUi->suggestedFileName(con)),"",this,i18nc("File chooser dialog title for exporting VPN","Export VPN"), KFileDialog::ConfirmOverwrite);
        if (expFile.isEmpty()) {
            delete vpnUi;
            return;
        }

        if (vpnUi->exportConnectionSettings(con, expFile)) {
            KMessageBox::information(this, i18n("VPN connection successfully exported"), i18n("Success"), i18n("Do not show again"), KMessageBox::Notify);
        } else {
            KMessageBox::error(this, vpnUi->lastErrorMessage(), i18n("Error exporting VPN connection settings"), KMessageBox::Notify);
        }
    } else {
        KMessageBox::error(this, i18n("Could not export VPN connection settings"), i18n("Error"), KMessageBox::Notify);
    }

    delete vpnUi;
    return;
}

void ManageConnectionWidget::editClicked()
{
    //edit might be clicked on a system connection, in which case we need a connectionid for it
    QTreeWidgetItem *item = selectedItem();

    if (item) {
        Knm::Connection *con = 0;

        QString connectionId = item->data(0, TreeWidgetItem::ConnectionIdRole).toString();
        if (connectionId.isEmpty()) {
            kDebug() << "selected item had no connectionId!";
            return;
        }

        //QVariantList args;
        //args << connectionId;

        //find clicked connection from our connection list
        con = mConnections->findConnection(connectionId);
        if (!con) {
            kWarning() << "Clicked connection with id" << connectionId << " could not be found in connection list!";
            return;
        }

        mEditConnection = new Knm::Connection(con);
        if (con->hasPersistentSecrets()) {
            bool rep = mSystemSettings->getConnectionSecrets(mEditConnection);

            if (!rep) {
                KMessageBox::error(this, i18n("Connection edit option failed, make sure that NetworkManager is properly running."));
                return;
            }

            kDebug() << "Get secrets " << rep;
        } else {
            kDebug() << "This connection has no secrets, good.";
            editGotSecrets(true, QString(), mEditConnection->uuid());
        }
        //emit changed();
    }
}

void ManageConnectionWidget::editGotSecrets(bool valid, const QString &errorMessage, const QString &uuid)
{
    if (!valid) {
        if (errorMessage.isEmpty())
            KMessageBox::error(this, i18n("Error"));
        else
            KMessageBox::error(this, errorMessage);
    }

    if (!mEditConnection || mEditConnection->uuid() != uuid) {
        return;
    }

    kDebug() << uuid << mEditConnection->uuid();
    Knm::Connection *result = mEditor->editConnection(mEditConnection); //starts editor window
    if (result) {
        mSystemSettings->updateConnection(mEditConnection->uuid().toString(), result);
    }

    delete mEditConnection;
    mEditConnection = 0;
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


void ManageConnectionWidget::deleteClicked()
{
    QTreeWidgetItem * item = selectedItem();
    if ( !item ) {
        kDebug() << "delete clicked, but no selection!";
        return;
    }
    QString connectionId = item->data(0, TreeWidgetItem::ConnectionIdRole).toString();
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

        mSystemSettings->removeConnection(connectionId);
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
            mSupportedExtns.clear();
            mVpnMenu = new QMenu(this);
            // foreach vpn service, add one of these
            KPluginInfo::List vpnServices = KPluginInfo::fromServices(KServiceTypeTrader::self()->query(QLatin1String("NetworkManagement/VpnUiPlugin")));
            foreach (const KPluginInfo &pi, vpnServices) {
                QString serviceType = pi.service()->property("X-NetworkManager-Services", QVariant::String).toString();
                QAction * vpnAction = new QAction(pi.name(), this);
                vpnAction->setData(serviceType);
                mVpnMenu->addAction(vpnAction);
                // Add supported file extensions for import
                QString pluginError;
                VpnUiPlugin * vpnUi = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg( serviceType ), this, QVariantList(), &pluginError );
                if (pluginError.isEmpty()) {
                    QString extn = vpnUi->supportedFileExtensions();
                    if (!extn.isEmpty())
                        mSupportedExtns +=  extn + ' '; // Separate by space
                    delete vpnUi;
                }
            }
            mSupportedExtns = mSupportedExtns.trimmed();    // Remove redundant space, if any
            connect(mVpnMenu, SIGNAL(triggered(QAction*)), SLOT(connectionTypeMenuTriggered(QAction*)));
            mConnEditUi.buttonSetVpn->addButton()->setMenu(mVpnMenu);
            mConnEditUi.buttonSetVpn->addButton()->setEnabled(!mVpnMenu->isEmpty());
            mConnEditUi.buttonSetVpn->importButton()->setEnabled(!mVpnMenu->isEmpty());
            mConnEditUi.buttonSetVpn->importButton()->setVisible(!mVpnMenu->isEmpty());
            mConnEditUi.buttonSetVpn->exportButton()->setVisible(!mVpnMenu->isEmpty());
            connect(mConnEditUi.buttonSetVpn->importButton(),SIGNAL(clicked()),SLOT(importClicked()));
            connect(mConnEditUi.buttonSetVpn->exportButton(),SIGNAL(clicked()),SLOT(exportClicked()));
        }
    }
}

void ManageConnectionWidget::connectionTypeMenuTriggered(QAction* action)
{
    Knm::Connection::Type tabType = connectionTypeForCurrentIndex();
    QVariantList args;

    switch(tabType) {
    case Knm::Connection::Wired:
        if (action->data().toBool()) { // shared connection
            args << QLatin1String("shared");
        }
        break;
    case Knm::Connection::Wireless:
        if (action->data().toBool()) { // shared connection
            args << QString() << QString() << QLatin1String("shared");
        }
        break;
    case Knm::Connection::Vpn:
        // NM uses plugins to handle different VPN types, use the data() on the action to select
        // the correct plugin
        args << action->data();
        break;
    default:
        return;
    }

    Knm::Connection *con = mEditor->createConnection(false, tabType, args);

    if (con) {
        kDebug() << "Connection pointer is set, connection will be added.";
        mSystemSettings->addConnection(con);
        emit changed();
    } else {
        kDebug() << "Connection pointer is not set, connection will not be added!";
    }
}

void ManageConnectionWidget::activeConnectionsChanged()
{
    QTreeWidgetItem * item = 0;
    foreach(QTreeWidgetItem * t, mUuidItemHash.values()) {
        t->setText(ConnectionStateColumn, QString());
    }
    foreach(QString activeConnection, Solid::Control::NetworkManagerNm09::activeConnectionsUuid()) {
        activeConnection = "{" + activeConnection + "}";
        item = mUuidItemHash.value(activeConnection);
        if (item != 0) {
            item->setText(ConnectionStateColumn, i18n("Connected"));
        }
    }
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
        QDateTime lastUsed = (*it)->data(0, TreeWidgetItem::ConnectionLastUsedRole).toDateTime();
        (*it)->setText(ConnectionLastUsedColumn, formatDateRelative(lastUsed));
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
