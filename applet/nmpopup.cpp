/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2011 Lamarque Souza <lamarque@kde.org>

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

// Own
#include "nmpopup.h"

// Qt
#include <QCheckBox>
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QTimer>
#include <QDBusConnection>

// KDE
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>
#include <KGlobalSettings>

// Plasma
#include <Plasma/Label>
#include <Plasma/Separator>

// QtNetworkManager
#include <QtNetworkManager/manager.h>
#include <QtNetworkManager/wirelessdevice.h>
#include <QtNetworkManager/wireddevice.h>

// client lib
#include "activatableitem.h"
#include "remoteactivatable.h"
#include "remoteactivatablelist.h"
#include "remotewirelessinterfaceconnection.h"

// More own includes
#include "interfaceitem.h"
#include "wirelessinterfaceitem.h"
#include "vpninterfaceitem.h"
#include "activatablelistwidget.h"
#include "interfacedetailswidget.h"
#include "typelistwidget.h"
#include "uiutils.h"
#include "paths.h"

#define ConnectionsTabIndex 0
#define DetailsTabIndex 1
#define ConnectToAnotherNetworkTabIndex 2

#define ConnectionListTabIndex 0
#define VPNConnectionListTabIndex 1
#define SharedConnectionListTabIndex 2

NMPopup::NMPopup(RemoteActivatableList * activatableList, QGraphicsWidget* parent)
: QGraphicsWidget(parent),
    m_activatables(activatableList),
    m_hasWirelessInterface(false),
    m_widget(0),
    m_tab1Layout(0),
    m_connectionList(0),
    m_currentIfaceItem(0),
    m_vpnItem(0)
{
    init();
}

NMPopup::~NMPopup()
{
    qDeleteAll(m_interfaces);
    if(m_vpnItem) {
        delete m_vpnItem;
    }
}

void NMPopup::init()
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    setMaximumWidth(QFontMetrics(KGlobalSettings::generalFont()).width("123456789012345678901234567890"));
    setMinimumHeight(10 * rowHeight);
    setMaximumHeight(15 * rowHeight);
    m_mainLayout = new QGraphicsLinearLayout(this);
    m_mainLayout->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    int rowMain = 0;

    /**************************
     * Main Window: first tab *
     **************************/
    m_tab1Layout = new QGraphicsGridLayout;

    /*m_title = new Plasma::Label(this);
    m_title->setText(i18nc("title", "<h3>Manage your network connections</h3>"));
    m_tab1Layout->addItem(m_title, 0, 0);
    m_tab1Layout->setRowMaximumHeight(rowMain++, rowHeight);*/

    /*** Inner TabBar ***/
    //Plasma::Frame * connectionsFrame = new Plasma::Frame(this);
    QGraphicsWidget * connectionsFrame = new QGraphicsWidget(this);
//    connectionsFrame->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_connectionTabLayout = new QGraphicsGridLayout(connectionsFrame);
    int innerRow = 0;

    // List with activatables
    m_connectionList = new ActivatableListWidget(m_activatables, connectionsFrame);
//    m_connectionList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_connectionList->setPreferredHeight(10 * rowHeight); // height of 10 activatableItems.
    m_connectionList->addType(Knm::Activatable::InterfaceConnection);
    m_connectionList->addType(Knm::Activatable::WirelessInterfaceConnection);
    m_connectionList->addType(Knm::Activatable::VpnInterfaceConnection);
    m_connectionList->addType(Knm::Activatable::GsmInterfaceConnection);
    m_connectionList->init();
    m_connectionList->setFilter(ActivatableListWidget::NormalConnections);
    connect(m_connectionList, SIGNAL(showInterfaceDetails(QString)), SLOT(showInterfaceDetails(QString)));
    connect(m_connectionList, SIGNAL(showMoreClicked()), SLOT(showMore()));
    m_connectionTabLayout->addItem(m_connectionList, innerRow++, 0, 1, 2);

    // TODO: add an item in the connection list to replace the "Show all" checkbox with the text "Show more %1 networks".
    // TODO: make this work. Change it to always be the last item in the connection list.
    //       Only visible when "Show all" is activated (?).
#if 0
    // Connect to another connection button
    m_connectToAnotherNetwork = new Plasma::IconWidget(connectionsFrame);
    m_connectToAnotherNetwork->setDrawBackground(true);
    m_connectToAnotherNetwork->setOrientation(Qt::Horizontal);
    m_connectToAnotherNetwork->setAcceptsHoverEvents(true);
    //m_connectToAnotherNetwork->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_connectToAnotherNetwork->setIcon(KIcon("network-connect"));
    m_connectToAnotherNetwork->setText(i18nc("button", "Connect to Another Network..."));
    m_connectToAnotherNetwork->setMinimumHeight(rowHeight);
    m_connectToAnotherNetwork->setMaximumHeight(rowHeight);
    connect(m_connectToAnotherNetwork, SIGNAL(clicked()), this, SLOT(connectToAnotherNetwork()));
    m_connectionTabLayout->addItem(m_connectToAnotherNetwork, innerRow, 0, 1, 2);
#endif

    m_connectionsTabBar = new ConnectionsTabBar(this);
    m_connectionsTabBar->addTab(i18nc("connection list", "Connections"), 0);
    m_connectionsTabBar->nativeWidget()->setTabToolTip(0, i18nc("@info:tooltip", "Connections"));
    m_connectionsTabBar->addTab(i18nc("connection list", "VPN"), 0);
    m_connectionsTabBar->nativeWidget()->setTabToolTip(1, i18nc("@info:tooltip", "VPN Connections"));
    m_connectionsTabBar->addTab(i18nc("connection list", "Shared Connections"), 0);
    m_connectionsTabBar->nativeWidget()->setTabToolTip(3, i18nc("@info:tooltip", "Shared Connections"));
    m_connectionsTabBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(m_connectionsTabBar, SIGNAL(currentChanged(int)), SLOT(currentInnerTabChanged(int)));
    /*** Inner TabBar: end ***/

    //m_tab1Layout->setRowMaximumHeight(rowMain, rowHeight);
    m_tab1Layout->addItem(m_connectionsTabBar, rowMain++, 0);
    m_tab1Layout->addItem(connectionsFrame, rowMain++, 0);

    // flight-mode checkbox
/*   m_networkingCheckBox = new Plasma::CheckBox(this);
    m_networkingCheckBox->setText(i18nc("CheckBox to enable or disable networking completely", "Enable networking"));
    m_tab1Layout->addItem(m_networkingCheckBox, 0, 0);
    m_networkingCheckBox->setText(i18nc("CheckBox to enable or disable networking completely", "Enable networking"));
    m_tab1Layout->addItem(m_networkingCheckBox, 0, 0);
    connect(m_networkingCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(networkingEnabledToggled(bool)));*/
    connect(NetworkManager::notifier(), SIGNAL(networkingEnabledChanged(bool)),
            this, SLOT(managerNetworkingEnabledChanged(bool)));

#if 0
    Plasma::Separator * sep = new Plasma::Separator(this);
    //sep->hide();
    m_tab1Layout->setRowMinimumHeight(rowMain, rowHeight);
    m_tab1Layout->addItem(sep, rowMain++, 0, 1, 1, Qt::AlignCenter);
#endif

    // flight-mode checkbox
    m_wifiCheckBox = new Plasma::CheckBox(this);
    m_wifiCheckBox->setText(i18nc("CheckBox to enable or disable wireless interface (rfkill)", "Enable wireless"));
    m_wifiCheckBox->hide();
    m_tab1Layout->addItem(m_wifiCheckBox, rowMain++, 0);

    connect(m_wifiCheckBox, SIGNAL(toggled(bool)), SLOT(wirelessEnabledToggled(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));

    // show all connections checkbox
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)), this, SLOT(uncheckShowMore(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), this, SLOT(checkShowMore(RemoteActivatable*)));

    // flight-mode checkbox
    m_wwanCheckBox = new Plasma::CheckBox(this);
    m_wwanCheckBox->setText(i18nc("CheckBox to enable or disable wwan (mobile broadband) interface)", "Enable mobile broadband"));
    m_wwanCheckBox->hide();
    m_wwanCheckBox->nativeWidget()->setTristate(true);
    m_tab1Layout->addItem(m_wwanCheckBox, rowMain++, 0);

    connect(m_wwanCheckBox, SIGNAL(toggled(bool)), SLOT(wwanEnabledToggled(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wwanEnabledChanged(bool)),
            this, SLOT(managerWwanEnabledChanged(bool)));
    connect(NetworkManager::notifier(), SIGNAL(wwanHardwareEnabledChanged(bool)),
            this, SLOT(managerWwanHardwareEnabledChanged(bool)));

    m_advancedSettingsButton = new Plasma::IconWidget(this);
    m_advancedSettingsButton->setDrawBackground(true);
    m_advancedSettingsButton->setOrientation(Qt::Horizontal);
    m_advancedSettingsButton->setAcceptsHoverEvents(true);
    //m_advancedSettingsButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_advancedSettingsButton->setIcon(KIcon("configure"));
    m_advancedSettingsButton->setText(i18nc("settings button in applet's popup window", "Settings..."));
    m_advancedSettingsButton->setMinimumHeight(28);
    m_advancedSettingsButton->setMaximumHeight(28);
    connect(m_advancedSettingsButton, SIGNAL(clicked()), this, SLOT(manageConnections()));
    m_tab1Layout->addItem(m_advancedSettingsButton, rowMain++, 0);

    /**************************************
     * Main Window: interface details tab *
     **************************************/
    m_interfaceDetailsWidget = new InterfaceDetailsWidget(m_connectionsTabBar);
//    m_interfaceDetailsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(m_interfaceDetailsWidget, SIGNAL(back()), this, SLOT(untoggleInterfaceTab()));

    /**************************************************
     * Main Window: connect to another connection tab *
     **************************************************/
//    TypeListWidget * m_typeListWidget = new TypeListWidget(this);
//    connect(m_typeListWidget, SIGNAL(back()), this, SLOT(untoggleInterfaceTab()));

    m_mainTabBar = new Plasma::TabBar(this);
    m_mainTabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainTabBar->nativeWidget()->setAttribute(Qt::WA_NoSystemBackground);
    m_mainTabBar->nativeWidget()->setMouseTracking(true);
    m_mainTabBar->setTabBarShown(false);
    m_mainTabBar->addTab(i18nc("main window", "Manage your network connections"), m_tab1Layout);
    m_mainTabBar->addTab(i18nc("details for the interface", "Details"), m_interfaceDetailsWidget);
//    m_mainTabBar->addTab(i18nc("connect to another connection window", "Connect to another network"), m_typeListWidget);
    connect(m_mainTabBar, SIGNAL(currentChanged(int)), SLOT(currentTabChanged(int)));
    m_mainLayout->addItem(m_mainTabBar);

    /******************
     * Initialization *
     ******************/
    //createTab(Knm::Activatable::WirelessInterfaceConnection);
    kDebug() << "Adding interfaces initially";
    foreach (NetworkManager::Device * iface, NetworkManager::networkInterfaces()) {
        addInterfaceInternal(iface);
        kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
    }
    addVpnInterface();
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(NetworkManager::notifier(), SIGNAL(deviceAdded(QString)),
            SLOT(interfaceAdded(QString)));
    connect(NetworkManager::notifier(), SIGNAL(deviceRemoved(QString)),
            SLOT(interfaceRemoved(QString)));

    m_showMoreChecked = false;
    m_oldShowMoreChecked = true;
    wicCount = 0; // number of wireless networks which user explicitly configured using the kcm module.

    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));
    m_oldShowMoreChecked = config.readEntry(QLatin1String("ShowMoreConnections"), true);
    showMore(m_oldShowMoreChecked);

    readConfig();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(readConfig()));
    dbus.connect("org.kde.kded", "/org/kde/networkmanagement", "org.kde.networkmanagement", "ReloadConfig", this, SLOT(readConfig()));

    adjustSize();
    m_connectionsTabBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

static int compareVersions(const QString & version1, const QString & version2)
{
    QStringList sl1 = version1.split('.');
    QStringList sl2 = version2.split('.');

    if (sl1.size() > 2 && sl2.size() > 2) {
        int v1[3] = { sl1[0].toInt(), sl1[1].toInt(), sl1[2].toInt() };
        int v2[3] = { sl2[0].toInt(), sl2[1].toInt(), sl2[2].toInt() };

        if (v1[0] > v2[0]) {
            return 1;
        } else if (v1[0] < v2[0]) {
            return -1;
        } else if (v1[1] > v2[1]) {
            return 1;
        } else if (v1[1] < v2[1]) {
            return -1;
        } else if (v1[2] > v2[2]) {
            return 1;
        } else if (v1[2] < v2[2]) {
            return -1;
        } else {
            return 0;
        }
    }
    return 0;
}

void NMPopup::readConfig()
{
    kDebug();
    KNetworkManagerServicePrefs::self()->readConfig();

    //m_networkingCheckBox->setChecked(NetworkManager::isNetworkingEnabled());
    m_wifiCheckBox->setChecked(NetworkManager::isWirelessEnabled());

    /* There is a bug in Solid < 4.6.2 where it does not emit the wirelessHardwareEnabledChanged signal.
     * So we always enable the wireless checkbox for versions below 4.6.2. */
#if KDE_IS_VERSION(4,6,2)
    m_wifiCheckBox->setEnabled(NetworkManager::isWirelessHardwareEnabled());
#else
    m_wifiCheckBox->setEnabled(true);
#endif

    /*m_showMoreButton->setEnabled(NetworkManager::isNetworkingEnabled() &&
                                 NetworkManager::isWirelessEnabled());*/

    m_wwanCheckBox->nativeWidget()->setCheckState(NetworkManager::isWwanEnabled() ? Qt::Checked : Qt::Unchecked);
    m_wwanCheckBox->setEnabled(NetworkManager::isWwanHardwareEnabled());

    foreach(InterfaceItem * i, m_interfaces) {
        i->setNameDisplayMode(InterfaceItem::InterfaceName);
    }

    QString version = NetworkManager::version();
    if (version.isEmpty()) {
        Plasma::Label * warning = new Plasma::Label(this);
        warning->setText(i18nc("Warning about wrong NetworkManager version", "NetworkManager is not running. Please start it."));
        m_tab1Layout->addItem(warning, 10, 0);
    } else if (compareVersions(version, QString(MINIMUM_NM_VERSION_REQUIRED)) < 0) {
        Plasma::Label * warning = new Plasma::Label(this);
        warning->setText(i18nc("Warning about wrong NetworkManager version", "We need at least NetworkManager-%1 to work properly, found '%2'. Please upgrade to a newer version.", QString(MINIMUM_NM_VERSION_REQUIRED), version));
        m_tab1Layout->addItem(warning, 10, 0);
    }
}

// Interfaces
void NMPopup::interfaceAdded(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        return;
    }
    NetworkManager::Device * iface = NetworkManager::findNetworkInterface(uni);
    if (iface) {
        kDebug() << "Interface Added:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
        addInterfaceInternal(iface);
    }
}

void NMPopup::interfaceRemoved(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        // To prevent crashes when the interface removed is the one in interfaceDetailsWidget.
        // the m_iface pointer in interfaceDetailsWidget become invalid in this case.
        if (uni == m_interfaceDetailsWidget->getLastIfaceUni()) {
            m_interfaceDetailsWidget->setInterfaceItem(0, false);
            // Since it is invalid go back to "main" window.
            m_connectionsTabBar->setCurrentIndex(ConnectionsTabIndex);
        }

        InterfaceItem* item = m_interfaces.take(uni);
        connect(item, SIGNAL(disappearAnimationFinished()), this, SLOT(deleteInterfaceItem()));
        item->disappear();
        updateHasWireless();
        updateHasWwan();
    }
}

void NMPopup::deleteInterfaceItem()
{
    // slot is called from animation's finished()
    InterfaceItem* item = dynamic_cast<InterfaceItem*>(sender());
    delete item;
}

NetworkManager::Device* NMPopup::defaultInterface()
{
    // In fact we're returning the first available interface,
    // and if there is none available just the first one we have
    // and if we don't have one, 0. Make sure you check though.
    if (NetworkManager::networkInterfaces().isEmpty()) {
        return 0;
    }
    NetworkManager::Device* iface = NetworkManager::networkInterfaces().first();
    foreach (NetworkManager::Device * _iface, NetworkManager::networkInterfaces()) {
        switch (_iface->state()) {
            case NetworkManager::Device::Disconnected:
            case NetworkManager::Device::Preparing:
            case NetworkManager::Device::ConfiguringHardware:
            case NetworkManager::Device::NeedAuth:
            case NetworkManager::Device::ConfiguringIp:
            case NetworkManager::Device::CheckingIp:
            case NetworkManager::Device::WaitingForSecondaries:
            case NetworkManager::Device::Activated:
            case NetworkManager::Device::Deactivating:
            case NetworkManager::Device::Failed:
                return _iface;
                break;
            case NetworkManager::Device::Unavailable:
            case NetworkManager::Device::Unmanaged:
            case NetworkManager::Device::UnknownState:
            default:
                break;
        }
    }
    return iface;
}

void NMPopup::addInterfaceInternal(NetworkManager::Device* iface)
{
    if (!iface) {
        // the interface might be gone in the meantime...
        return;
    }
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ifaceItem = 0;
        if (iface->type() == NetworkManager::Device::Wifi) {
            // Create the wireless interface item
            WirelessInterfaceItem* wifiItem = 0;
            wifiItem = new WirelessInterfaceItem(static_cast<NetworkManager::WirelessDevice *>(iface), m_activatables, InterfaceItem::InterfaceName);
            ifaceItem = wifiItem;
            wifiItem->setEnabled(NetworkManager::isWirelessEnabled());
            kDebug() << "WiFi added";
            connect(wifiItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        } else {
            // Create the interfaceitem
            ifaceItem = new InterfaceItem(static_cast<NetworkManager::WiredDevice *>(iface), m_activatables, InterfaceItem::InterfaceName);
            connect(ifaceItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        }
        connect(ifaceItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
        /*connect(ifaceItem, SIGNAL(clicked(NetworkManager::Device*)),
                m_connectionList,  SLOT(addInterface(NetworkManager::Device*)));*/
        connect(ifaceItem, SIGNAL(hoverEnter(QString)), m_connectionList, SLOT(hoverEnter(QString)));
        connect(ifaceItem, SIGNAL(hoverLeave(QString)), m_connectionList, SLOT(hoverLeave(QString)));

        // Catch connection changes
        connect(iface, SIGNAL(stateChanged(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)), this, SLOT(handleConnectionStateChange(NetworkManager::Device::State,NetworkManager::Device::State,NetworkManager::Device::StateChangeReason)));
        m_interfaces.insert(iface->uni(), ifaceItem);
    }
    updateHasWireless();
    updateHasWwan();
}

void NMPopup::showInterfaceDetails(const QString & uni)
{
    InterfaceItem * ifaceItem = m_interfaces.value(uni, 0);
    if (!ifaceItem) {
        if (m_vpnItem) {
            ifaceItem = m_vpnItem;
        } else {
            return;
        }
    }
    QMetaObject::invokeMethod(ifaceItem, "clicked", Qt::QueuedConnection);
}

void NMPopup::addVpnInterface()
{
    m_vpnItem = new VpnInterfaceItem(0, m_activatables, InterfaceItem::InterfaceName);
    connect(m_vpnItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
}

void NMPopup::handleConnectionStateChange(NetworkManager::Device::State new_state, NetworkManager::Device::State old_state, NetworkManager::Device::StateChangeReason reason)
{
    Q_UNUSED( reason );
    // Switch to default tab if an interface has become available, or unavailable
    if (available(new_state) != available(old_state)) {
        m_connectionList->clearInterfaces();
    }
}

bool NMPopup::hasWireless() {
    return m_hasWirelessInterface;
}

bool NMPopup::available(int state)
{
    // Can an interface be used?
    switch (state) {
        case NetworkManager::Device::Disconnected:
        case NetworkManager::Device::Preparing:
        case NetworkManager::Device::ConfiguringHardware:
        case NetworkManager::Device::NeedAuth:
        case NetworkManager::Device::ConfiguringIp:
        case NetworkManager::Device::CheckingIp:
        case NetworkManager::Device::WaitingForSecondaries:
        case NetworkManager::Device::Activated:
        case NetworkManager::Device::Deactivating:
        case NetworkManager::Device::Failed:
            return true;
            break;
        case NetworkManager::Device::Unavailable:
        case NetworkManager::Device::Unmanaged:
        case NetworkManager::Device::UnknownState:
        default:
            return false;
            break;
    }
    return false;
}

void NMPopup::wirelessEnabledToggled(bool checked)
{
    kDebug() << "Applet wireless enable switch toggled" << checked;
    if (NetworkManager::isWirelessEnabled() != checked) {
        NetworkManager::setWirelessEnabled(checked);
    }
    updateHasWireless(checked);
}

void NMPopup::wwanEnabledToggled(bool checked)
{
    kDebug() << "Applet wwan enable switch toggled" << checked;
    if (NetworkManager::isWwanEnabled() != checked) {
        NetworkManager::setWwanEnabled(checked);
    }
}

void NMPopup::networkingEnabledToggled(bool checked)
{
    // Switch networking on / off
    if (NetworkManager::isNetworkingEnabled() != checked) {
        NetworkManager::setNetworkingEnabled(checked);
    }
    // Update wireless checkbox
    m_wifiCheckBox->setChecked(NetworkManager::isWirelessEnabled());

kDebug() << "Wireless hardware enabled ==" << NetworkManager::isWirelessHardwareEnabled();

#if KDE_IS_VERSION(4,6,2)
    m_wifiCheckBox->setEnabled(NetworkManager::isWirelessHardwareEnabled() || NetworkManager::isWirelessEnabled());
#endif

    m_wwanCheckBox->nativeWidget()->setCheckState(NetworkManager::isWwanEnabled() ? Qt::Checked : Qt::Unchecked);
    m_wwanCheckBox->setEnabled(NetworkManager::isWwanHardwareEnabled() || NetworkManager::isWwanEnabled());

    updateHasWireless(checked);
}

void NMPopup::updateHasWireless(bool checked)
{
    //kDebug() << "UPDATE!!!!!!!!!!!!";
    bool hasWireless = true;
    if (!NetworkManager::isWirelessHardwareEnabled() ||
        !NetworkManager::isNetworkingEnabled() ||
        !NetworkManager::isWirelessEnabled() || !checked) {

        //kDebug () << "networking enabled?" << NetworkManager::isNetworkingEnabled();
        //kDebug () << "wireless hardware enabled?" << NetworkManager::isWirelessHardwareEnabled();
        //kDebug () << "wireless enabled?" << NetworkManager::isWirelessEnabled();

        // either networking is disabled, or wireless is disabled
        hasWireless = false;
    }
    //solid is too slow, we need to see if the checkbox was checked by the user
    if (checked)
        hasWireless = true;
    kDebug() << "After chckboxn" << hasWireless;

    foreach (InterfaceItem* ifaceitem, m_interfaces) {
        NetworkManager::Device* iface = ifaceitem->interface();
        if (iface && iface->type() == NetworkManager::Device::Wifi) {
            //kDebug() << "there's a wifi iface" << ifaceitem->connectionName() << iface->interfaceName();
            m_hasWirelessInterface = true; // at least one interface is wireless. We're happy.
            m_wifiCheckBox->show();
            break;
        }
    }
    if (!m_hasWirelessInterface) {
        kDebug() << "no ifaces";
        hasWireless = false;
        m_wifiCheckBox->hide();
    }
    m_connectionList->setHasWireless(hasWireless);
}

void NMPopup::updateHasWwan()
{
    bool hasWwan = false;
    foreach (InterfaceItem* ifaceitem, m_interfaces) {
        NetworkManager::ModemDevice* iface = qobject_cast<NetworkManager::ModemDevice *>(ifaceitem->interface());
        if (iface) {
            hasWwan = true;
            break;
        }
    }
    if (hasWwan) {
        m_wwanCheckBox->show();
        hasWwan = m_wwanCheckBox->isChecked();
    } else {
        m_wwanCheckBox->hide();
    }
}

void NMPopup::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    m_wifiCheckBox->setChecked(enabled);
    if (enabled) {
        m_wifiCheckBox->setEnabled(enabled);
    }
}

void NMPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_wifiCheckBox->setEnabled(enabled);
    updateHasWireless(enabled);
}

void NMPopup::managerNetworkingEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed networking enable state" << enabled;
    networkingEnabledToggled(enabled);
}

void NMPopup::managerWwanEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wwan enable state" << enabled;
    m_wwanCheckBox->nativeWidget()->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
    if (enabled) {
        m_wwanCheckBox->setEnabled(enabled);
    }
}

void NMPopup::managerWwanHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wwan enable switch state changed" << enabled;
    m_wwanCheckBox->setEnabled(enabled);
}

void NMPopup::showMore()
{
    // Simulate button toggling.
    m_showMoreChecked = !m_showMoreChecked;
    m_oldShowMoreChecked = m_showMoreChecked;
    showMore(m_oldShowMoreChecked);

    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));
    config.writeEntry(QLatin1String("ShowMoreConnections"), m_oldShowMoreChecked);
    config.sync();
}

void NMPopup::showMore(bool show)
{
    if (!NetworkManager::isWirelessEnabled()) {
        show = false;
    } else if (wicCount == 0) {
        show = true;
    }

    if (show) {
        m_connectionList->getFilter() &= ~ActivatableListWidget::SavedConnections;
    } else {
        m_connectionList->getFilter() |= ActivatableListWidget::SavedConnections;
    }
    m_connectionList->filter();
    m_showMoreChecked = show;
    if (wicCount > 0) {
        emit showMoreChecked(m_showMoreChecked);
    }
    kDebug() << "Show == " << show << " wicCount == " << wicCount;
}

void NMPopup::checkShowMore(RemoteActivatable * ra)
{
    RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
    if (wic) {
        if (wic->operationMode() == NetworkManager::WirelessDevice::Adhoc &&
            wic->activationState() == Knm::InterfaceConnection::Unknown) {
            return;
        }
        if (wicCount > 0) {
            wicCount--;
        }
        if (wicCount == 0 && !m_showMoreChecked) {
            // There is no wireless network around which the user has explicitly configured
            // so temporaly show all wifi available networks.
            showMore(true);
        }
    }
}

void NMPopup::uncheckShowMore(RemoteActivatable *ra)
{
    RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
    if (wic) {
        if (wic->operationMode() == NetworkManager::WirelessDevice::Adhoc &&
            wic->activationState() == Knm::InterfaceConnection::Unknown) {
            return;
        }
        wicCount++;
        if (m_oldShowMoreChecked != m_showMoreChecked) {
            // One wireless network explicitly configured by the user appeared, reset "Show More" button
            // state to the value before the checkShowMore method above took action.
            showMore(m_oldShowMoreChecked);
        }
    }
}

void NMPopup::manageConnections()
{
    //kDebug() << "opening connection management dialog";
    QStringList args;
    args << "--icon" << "networkmanager" << "kcm_networkmanagement" << "kcm_networkmanagement_tray";
    KToolInvocation::kdeinitExec("kcmshell4", args);
}


void NMPopup::toggleInterfaceTab()
{
    m_currentIfaceItem = qobject_cast<InterfaceItem*>(sender());

    if (m_currentIfaceItem) {
        m_interfaceDetailsWidget->setInterfaceItem(m_currentIfaceItem);
    }

    m_mainTabBar->setCurrentIndex(DetailsTabIndex);
}

void NMPopup::untoggleInterfaceTab()
{
    m_mainTabBar->setCurrentIndex(ConnectionsTabIndex);
}

void NMPopup::connectToAnotherNetwork()
{
    //m_mainTabBar->setCurrentIndex(ConnectToAnotherNetworkTabIndex);
    switch (m_connectionsTabBar->currentIndex()) {
    case ConnectionListTabIndex:
        break;
    case VPNConnectionListTabIndex: {
        QStringList args;
        args << "--icon" << "networkmanager" << "kcm_networkmanagement" << "kcm_networkmanagement_tray" << "--args" << "vpn";
        KToolInvocation::kdeinitExec("kcmshell4", args);
        break;
    }
    case SharedConnectionListTabIndex:
        break;
    }
}

void NMPopup::currentTabChanged(int index)
{
    switch (index) {
    case ConnectionsTabIndex:
        m_connectionList->clearInterfaces();
        //showMore(m_oldShowMoreChecked);
        m_interfaceDetailsWidget->setUpdateEnabled(false);
        break;

    case DetailsTabIndex:
        // Enable / disable updating of the details widget
        m_interfaceDetailsWidget->setUpdateEnabled(true);

        if (m_currentIfaceItem && m_currentIfaceItem->interface()) {
            // Temporaly disables hightlight for all connections of this interface.
            QMetaObject::invokeMethod(m_currentIfaceItem, "hoverLeave", Qt::QueuedConnection,
                                      Q_ARG(QString, m_currentIfaceItem->interface()->uni()));
        }
        break;
    }
}

void NMPopup::currentInnerTabChanged(int index)
{
    ActivatableListWidget::FilterTypes filter;
    if (m_showMoreChecked) {
        filter = 0;
    } else {
        filter = ActivatableListWidget::SavedConnections;
    }
    switch (index) {
    case ConnectionListTabIndex:
        //m_connectToAnotherNetwork->setText(i18nc("button", "Connect to Another Network..."));
        m_connectionList->setFilter(filter | ActivatableListWidget::NormalConnections);
        break;
    case VPNConnectionListTabIndex:
        //m_connectToAnotherNetwork->setText(i18nc("button", "Connect to Another VPN Network..."));
        m_connectionList->setFilter(filter | ActivatableListWidget::VPNConnections);
        break;
    case SharedConnectionListTabIndex:
        //m_connectToAnotherNetwork->setText(i18nc("button", "Share Another Network Interface..."));
        m_connectionList->setFilter(filter | ActivatableListWidget::SharedConnections);
        break;
    }
    //m_connectToAnotherNetwork->update();
}
// vim: sw=4 sts=4 et tw=100

