/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>

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
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QTimer>
#include <QDBusConnection>

// KDE
#include <KDebug>
#include <KIcon>
#include <KIconLoader>
#include <KToolInvocation>

// Plasma
#include <Plasma/Label>
#include <Plasma/Separator>

// Solid::Control
#include <solid/control/networkmanager.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkserialinterface.h>

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
#include "uiutils.h"
#include "knmserviceprefs.h"

NMPopup::NMPopup(RemoteActivatableList * activatableList, QGraphicsWidget* parent)
: QGraphicsWidget(parent),
    m_activatables(activatableList),
    m_hasWirelessInterface(false),
    m_widget(0),
    m_mainLayout(0),
    m_leftWidget(0),
    m_leftLayout(0),
    m_interfaceLayout(0),
    m_connectionList(0),
    m_vpnItem(0)
{
    //setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    init();
}

NMPopup::~NMPopup()
{
}

void NMPopup::init()
{
    m_mainLayout = new QGraphicsGridLayout(this);

    m_leftLabel = new Plasma::Label(this);
    //m_leftLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_leftLabel->setMaximumHeight(24);
    m_leftLabel->setMinimumHeight(24);
    m_leftLabel->setText(i18nc("title on the LHS of the plasmoid", "<h3>Interfaces</h3>"));
    m_mainLayout->addItem(m_leftLabel, 0, 0);

    m_rightLabel = new Plasma::Label(this);
    //m_rightLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_rightLabel->setMaximumHeight(24);
    m_rightLabel->setMinimumHeight(24);
    m_rightLabel->setText(i18nc("title on the RHS of the plasmoid", "<h3>Connections</h3>"));
    m_mainLayout->addItem(m_rightLabel, 0, 2);

    Plasma::Separator* sep = new Plasma::Separator(this);
    sep->setOrientation(Qt::Vertical);
    m_mainLayout->addItem(sep, 0, 1, 2, 1);
    m_mainLayout->setRowFixedHeight(0, 24);

    m_leftWidget = new Plasma::TabBar(this);
    m_leftWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    m_leftLayout = new QGraphicsLinearLayout;
    m_leftLayout->setOrientation(Qt::Vertical);

    m_interfaceLayout = new QGraphicsLinearLayout;
    m_interfaceLayout->setOrientation(Qt::Vertical);
    m_interfaceLayout->setSpacing(0);

    m_leftLayout->addItem(m_interfaceLayout);
    m_leftLayout->addStretch(5);

    Plasma::Separator* sep2 = new Plasma::Separator(this);
    sep->setOrientation(Qt::Vertical);
    m_leftLayout->addItem(sep2);

    QGraphicsWidget* checkboxWidget = new QGraphicsWidget(this);
    QGraphicsGridLayout* checkboxLayout = new QGraphicsGridLayout(checkboxWidget);
    // flight-mode checkbox
    m_networkingCheckBox = new Plasma::CheckBox(m_leftWidget);
    m_networkingCheckBox->setText(i18nc("CheckBox to enable or disable networking completely", "Enable networking"));
    checkboxLayout->addItem(m_networkingCheckBox, 0, 0);
    connect(m_networkingCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(networkingEnabledToggled(bool)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkingEnabledChanged(bool)),
            this, SLOT(managerNetworkingEnabledChanged(bool)));

#ifdef NM_0_8
    // flight-mode checkbox
    m_wwanCheckBox = new Plasma::CheckBox(m_leftWidget);
    m_wwanCheckBox->setText(i18nc("CheckBox to enable or disable wwan (mobile broadband) interface)", "Enable mobile broadband"));
    m_wwanCheckBox->hide();
    checkboxLayout->addItem(m_wwanCheckBox, 0, 1);

    connect(m_wwanCheckBox, SIGNAL(toggled(bool)), SLOT(wwanEnabledToggled(bool)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wwanEnabledChanged(bool)),
            this, SLOT(managerWwanEnabledChanged(bool)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wwanHardwareEnabledChanged(bool)),
            this, SLOT(managerWwanHardwareEnabledChanged(bool)));
#endif

    // flight-mode checkbox
    m_wifiCheckBox = new Plasma::CheckBox(m_leftWidget);
    m_wifiCheckBox->setText(i18nc("CheckBox to enable or disable wireless interface (rfkill)", "Enable wireless"));
    m_wifiCheckBox->hide();
    checkboxLayout->addItem(m_wifiCheckBox, 1, 0);

    connect(m_wifiCheckBox, SIGNAL(toggled(bool)), SLOT(wirelessEnabledToggled(bool)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));

    m_leftLayout->addItem(checkboxWidget);

    //m_leftWidget->setLayout(m_leftLayout);
    m_leftWidget->addTab(i18nc("tabbar on the left side", "Interfaces"), m_leftLayout);
    m_leftWidget->setTabBarShown(false); // TODO: enable

    m_interfaceDetailsWidget = new InterfaceDetailsWidget(m_leftWidget);
    connect(m_interfaceDetailsWidget, SIGNAL(back()), this, SLOT(toggleInterfaceTab()));

    m_leftWidget->addTab(i18nc("details for the interface", "Details"), m_interfaceDetailsWidget);
    m_leftWidget->setPreferredWidth(300);

    m_mainLayout->addItem(m_leftWidget, 1, 0);

    m_rightWidget = new QGraphicsWidget(this);
    m_rightWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
    m_rightLayout = new QGraphicsLinearLayout(m_rightWidget);
    m_rightLayout->setOrientation(Qt::Vertical);

    // List with activatables
    m_connectionList = new ActivatableListWidget(m_activatables, m_rightWidget);
    m_connectionList->addType(Knm::Activatable::InterfaceConnection);
    m_connectionList->addType(Knm::Activatable::WirelessInterfaceConnection);
    m_connectionList->addType(Knm::Activatable::VpnInterfaceConnection);
#ifdef COMPILE_MODEM_MANAGER_SUPPORT
    m_connectionList->addType(Knm::Activatable::GsmInterfaceConnection);
#endif
    m_connectionList->init();

    m_connectionList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //m_connectionList->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
    m_connectionList->setPreferredHeight(240);

    m_connectionList->setMinimumWidth(320);
    m_connectionList->setShowAllTypes(false, true);

    m_rightLayout->addItem(m_connectionList);

    m_connectionsButton = new Plasma::PushButton(m_rightWidget);
    m_connectionsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_connectionsButton->setIcon(KIcon("configure"));
    m_connectionsButton->setText(i18nc("manage connections button in the applet's popup", "Manage Connections..."));
    m_connectionsButton->setMinimumHeight(28);
    m_connectionsButton->setMaximumHeight(28);
    connect(m_connectionsButton, SIGNAL(clicked()), this, SLOT(manageConnections()));

    m_showMoreButton = new Plasma::PushButton(m_rightWidget);
    m_showMoreButton->setCheckable(true);
    m_showMoreButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_showMoreButton->setIcon(KIcon("list-add"));
    m_showMoreButton->setText(i18nc("show more button in the applet's popup", "Show More..."));
    m_showMoreButton->setMinimumHeight(28);
    m_showMoreButton->setMaximumHeight(28);
    QSizeF sMore = m_showMoreButton->size();
    m_showMoreButton->setText(i18nc("show more button in the applet's popup", "Show Less..."));
    QSizeF sLess = m_showMoreButton->size();
    QSizeF sMax = sMore.width() > sLess.width() ? sMore : sLess;
    m_showMoreButton->setMinimumSize(sMax);
    m_showMoreButton->setMaximumSize(sMax);
    connect(m_showMoreButton, SIGNAL(clicked()), this, SLOT(showMore()));
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable *)), this, SLOT(uncheckShowMore(RemoteActivatable *)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable *)), this, SLOT(checkShowMore(RemoteActivatable *)));

    QGraphicsLinearLayout* connectionLayout = new QGraphicsLinearLayout;
    //connectionLayout->addStretch();
    connectionLayout->addItem(m_showMoreButton);
    connectionLayout->addItem(m_connectionsButton);

    m_rightLayout->addItem(connectionLayout);

    m_mainLayout->addItem(m_rightWidget, 1, 2);

    //createTab(Knm::Activatable::WirelessInterfaceConnection);
    kDebug() << "Adding interfaces initially";
    foreach (Solid::Control::NetworkInterface * iface, Solid::Control::NetworkManager::networkInterfaces()) {
        addInterfaceInternal(iface);
        kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
    }
    addVpnInterface();
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceAdded(const QString&)),
            SLOT(interfaceAdded(const QString&)));
    connect(Solid::Control::NetworkManager::notifier(), SIGNAL(networkInterfaceRemoved(const QString&)),
            SLOT(interfaceRemoved(const QString&)));

    oldShowMore = true;
    wicCount = 0; // number of wireless networks which user explicitly configured using the kcm module.
    foreach (RemoteActivatable *ra, m_activatables->activatables()) {
        RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
        if (wic) {
            if (wic->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc &&
                wic->activationState() == Knm::InterfaceConnection::Unknown) {
                continue;
            }
            uncheckShowMore(ra);
        }
    }
    oldShowMore = false;
    showMore(oldShowMore);

    //setPreferredSize(640, 400);

    readConfig();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(readConfig()));
    dbus.connect("org.kde.kded", "/org/kde/networkmanagement", "org.kde.networkmanagement", "ReloadConfig", this, SLOT(readConfig()));
}

void NMPopup::readConfig()
{
    kDebug();
    KNetworkManagerServicePrefs::instance(Knm::ConnectionPersistence::NETWORKMANAGEMENT_RCFILE);
    KNetworkManagerServicePrefs::self()->readConfig();
    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("SystemTray"));
    if (config.exists()) {
        bool networkingEnabled = config.readEntry("NetworkingEnabled",
                                                  Solid::Control::NetworkManager::isNetworkingEnabled());
        bool wirelessEnabled = config.readEntry("WirelessEnabled",
                                                Solid::Control::NetworkManager::isWirelessEnabled());

        Solid::Control::NetworkManager::setNetworkingEnabled(networkingEnabled);
        Solid::Control::NetworkManager::setWirelessEnabled(wirelessEnabled);
#ifdef NM_0_8
        bool wwanEnabled = config.readEntry("WwanEnabled",
                                            Solid::Control::NetworkManager::isWwanEnabled());
        Solid::Control::NetworkManager::setWwanEnabled(wwanEnabled);
#endif
    }
    m_networkingCheckBox->setChecked(Solid::Control::NetworkManager::isNetworkingEnabled());
    m_wifiCheckBox->setChecked(Solid::Control::NetworkManager::isWirelessEnabled());

    /* There is a bug in Solid < 4.6.2 where it does not emit the wirelessHardwareEnabledChanged signal.
     * So we always enable the wireless checkbox for versions below 4.6.2. */
#if KDE_IS_VERSION(4,6,2)
    m_wifiCheckBox->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
#else
    m_wifiCheckBox->setEnabled(true);
#endif

#ifdef NM_0_8
    m_wwanCheckBox->setChecked(Solid::Control::NetworkManager::isWwanEnabled());
    m_wwanCheckBox->setEnabled(Solid::Control::NetworkManager::isWwanHardwareEnabled());
#endif

    foreach(InterfaceItem * i, m_interfaces) {
        i->setNameDisplayMode(InterfaceItem::InterfaceName);
    }
}

void NMPopup::saveConfig()
{
    kDebug();
    /* If networking is disabled system is probably suspending to ram/disk.
       When it resumes we want to put NetworkManager at the same state as before the suspend,
       so we do not save config now. */
    if (Solid::Control::NetworkManager::isNetworkingEnabled()) {
        kDebug() << "Saving config";
        KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("SystemTray"));
        config.writeEntry("NetworkingEnabled", m_networkingCheckBox->isChecked());
        config.writeEntry("WirelessEnabled", m_wifiCheckBox->isChecked());
#ifdef NM_0_8
        config.writeEntry("WwanEnabled", m_wwanCheckBox->isChecked());
#endif
        KNetworkManagerServicePrefs::self()->writeConfig();
    }
}

// Interfaces
void NMPopup::interfaceAdded(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        return;
    }
    Solid::Control::NetworkInterface * iface = Solid::Control::NetworkManager::findNetworkInterface(uni);
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
            m_interfaceDetailsWidget->setInterface(0, false);
            // Since it is invalid go back to "main" window.
            m_leftWidget->setCurrentIndex(0);
        }

        InterfaceItem* item = m_interfaces.take(uni);
        connect(item, SIGNAL(disappearAnimationFinished()), this, SLOT(deleteInterfaceItem()));
        item->disappear();
        updateHasWireless();
#ifdef NM_0_8
        updateHasWwan();
#endif
    }
}

void NMPopup::deleteInterfaceItem()
{
    // slot is called from animation's finished()
    InterfaceItem* item = dynamic_cast<InterfaceItem*>(sender());
    m_interfaceLayout->removeItem(item);
    delete item;
}

Solid::Control::NetworkInterface* NMPopup::defaultInterface()
{
    // In fact we're returning the first available interface,
    // and if there is none available just the first one we have
    // and if we don't have one, 0. Make sure you check though.
    if (!Solid::Control::NetworkManager::networkInterfaces().count()) {
        return 0;
    }
    Solid::Control::NetworkInterface* iface = Solid::Control::NetworkManager::networkInterfaces().first();
    foreach (Solid::Control::NetworkInterface * _iface, Solid::Control::NetworkManager::networkInterfaces()) {
        switch (_iface->connectionState()) {
            case Disconnected:
            case Preparing:
            case Configuring:
            case NeedAuth:
            case IPConfig:
            case IPCheck:
            case Secondaries:
            case Activated:
            case Deactivating:
            case Failed:
                return _iface;
                break;
            case Unavailable:
            case Unmanaged:
            case UnknownState:
            default:
                break;
        }
    }
    return iface;
}

void NMPopup::addInterfaceInternal(Solid::Control::NetworkInterface* iface)
{
    if (!iface) {
        // the interface might be gone in the meantime...
        return;
    }
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ifaceItem = 0;
        if (iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
            // Create the wireless interface item
            WirelessInterfaceItem* wifiItem = 0;
            wifiItem = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterface *>(iface), m_activatables, InterfaceItem::InterfaceName, this);
            ifaceItem = wifiItem;
            //connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
            wifiItem->setEnabled(Solid::Control::NetworkManager::isWirelessEnabled());
            kDebug() << "WiFi added";
            connect(wifiItem, SIGNAL(disconnectInterfaceRequested(const QString&)), m_connectionList, SLOT(deactivateConnection(const QString&)));
        } else {
            // Create the interfaceitem
            ifaceItem = new InterfaceItem(static_cast<Solid::Control::WiredNetworkInterface *>(iface), m_activatables, InterfaceItem::InterfaceName, this);
            connect(ifaceItem, SIGNAL(disconnectInterfaceRequested(const QString&)), m_connectionList, SLOT(deactivateConnection(const QString&)));
        }
        connect(ifaceItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
        connect(ifaceItem, SIGNAL(clicked(Solid::Control::NetworkInterface*)),
                m_connectionList,  SLOT(addInterface(Solid::Control::NetworkInterface*)));
        connect(ifaceItem, SIGNAL(hoverEnter(const QString&)), m_connectionList, SLOT(hoverEnter(const QString&)));
        connect(ifaceItem, SIGNAL(hoverLeave(const QString&)), m_connectionList, SLOT(hoverLeave(const QString&)));

        // Catch connection changes
        connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        m_interfaceLayout->addItem(ifaceItem);
        m_interfaces.insert(iface->uni(), ifaceItem);
    }
    updateHasWireless();
#ifdef NM_0_8
    updateHasWwan();
#endif
}

void NMPopup::addVpnInterface()
{
    m_vpnItem = new VpnInterfaceItem(0, m_activatables, InterfaceItem::InterfaceName, this);
    connect(m_vpnItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
    connect(m_vpnItem, SIGNAL(clicked(Solid::Control::NetworkInterface*)),
            m_connectionList,  SLOT(addInterface(Solid::Control::NetworkInterface*)));

    connect(m_vpnItem, SIGNAL(clicked()), m_connectionList, SLOT(toggleVpn()));

    m_leftLayout->insertItem(2, m_vpnItem);
}

void NMPopup::handleConnectionStateChange(int new_state, int old_state, int reason)
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
        case Disconnected:
        case Preparing:
        case Configuring:
        case NeedAuth:
        case IPConfig:
        case IPCheck:
        case Secondaries:
        case Activated:
        case Deactivating:
        case Failed:
            return true;
            break;
        case Unavailable:
        case Unmanaged:
        case UnknownState:
        default:
            return false;
            break;
    }
    return false;
}

void NMPopup::wirelessEnabledToggled(bool checked)
{
    kDebug() << "Applet wireless enable switch toggled" << checked;
    if (Solid::Control::NetworkManager::isWirelessEnabled() != checked) {
        Solid::Control::NetworkManager::setWirelessEnabled(checked);
    }
    if (checked && Solid::Control::NetworkManager::isNetworkingEnabled()) {
//        showMore(false);
        m_showMoreButton->show();
    } else {
        m_showMoreButton->hide();
    }
    updateHasWireless(checked);
    saveConfig();
}

#ifdef NM_0_8
void NMPopup::wwanEnabledToggled(bool checked)
{
    kDebug() << "Applet wwan enable switch toggled" << checked;
    if (Solid::Control::NetworkManager::isWwanEnabled() != checked) {
        Solid::Control::NetworkManager::setWwanEnabled(checked);
    }
    saveConfig();
}
#endif

void NMPopup::networkingEnabledToggled(bool checked)
{
    // Switch networking on / off
    if (Solid::Control::NetworkManager::isNetworkingEnabled() != checked) {
        Solid::Control::NetworkManager::setNetworkingEnabled(checked);
    }
    // Update wireless checkbox
    m_wifiCheckBox->setChecked(Solid::Control::NetworkManager::isWirelessEnabled());

#if KDE_IS_VERSION(4,6,2)
    m_wifiCheckBox->setEnabled(Solid::Control::NetworkManager::isWirelessHardwareEnabled());
#endif

#ifdef NM_0_8
    m_wwanCheckBox->setChecked(Solid::Control::NetworkManager::isWwanEnabled());
    m_wwanCheckBox->setEnabled(Solid::Control::NetworkManager::isWwanHardwareEnabled());

#endif
    if (checked && Solid::Control::NetworkManager::isWirelessHardwareEnabled() &&
                   Solid::Control::NetworkManager::isWirelessEnabled()) {
//        showMore(false);
        m_showMoreButton->show();
    } else {
        m_showMoreButton->hide();
    }
    updateHasWireless(checked);
    saveConfig();
}

void NMPopup::updateHasWireless(bool checked)
{
    //kDebug() << "UPDATE!!!!!!!!!!!!";
    bool hasWireless = true;
    if (!Solid::Control::NetworkManager::isWirelessHardwareEnabled() ||
        !Solid::Control::NetworkManager::isNetworkingEnabled() ||
        !Solid::Control::NetworkManager::isWirelessEnabled() || !checked) {

        //kDebug () << "networking enabled?" << Solid::Control::NetworkManager::isNetworkingEnabled();
        //kDebug () << "wireless hardware enabled?" << Solid::Control::NetworkManager::isWirelessHardwareEnabled();
        //kDebug () << "wireless enabled?" << Solid::Control::NetworkManager::isWirelessEnabled();

        // either networking is disabled, or wireless is disabled
        hasWireless = false;
    }
    //solid is too slow, we need to see if the checkbox was checked by the user
    if (checked)
        hasWireless = true;
    kDebug() << "After chckboxn" << hasWireless;

    foreach (InterfaceItem* ifaceitem, m_interfaces) {
        Solid::Control::NetworkInterface* iface = ifaceitem->interface();
        if (iface && iface->type() == Solid::Control::NetworkInterface::Ieee80211) {
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

#ifdef NM_0_8
void NMPopup::updateHasWwan()
{
    bool hasWwan = false;
    foreach (InterfaceItem* ifaceitem, m_interfaces) {
        Solid::Control::SerialNetworkInterface* iface = qobject_cast<Solid::Control::SerialNetworkInterface *>(ifaceitem->interface());
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
#endif

void NMPopup::managerWirelessEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed wireless enable state" << enabled;
    // it might have changed because we toggled the switch,
    // but it might have been changed externally, so set it anyway
    m_wifiCheckBox->setChecked(enabled);
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
    m_networkingCheckBox->setChecked(enabled);
}

#ifdef NM_0_8
void NMPopup::enableWwan()
{
    Solid::Control::NetworkManager::setWwanEnabled(true);
}

void NMPopup::disableWwan()
{
    Solid::Control::NetworkManager::setWwanEnabled(false);
}

void NMPopup::managerWwanEnabledChanged(bool enabled)
{
    static bool first = true;

    kDebug() << "NM daemon changed wwan enable state" << enabled;

    /*
     * NetworkManager disables wwan everytime one wwan interface is attached.
     * I am using this hack to force NM to re-enable wwan if wwanCheckBox
     * is checked. The variable "first" is just to prevent infinity loop.
     */
    if (!enabled && m_wwanCheckBox->isChecked() && first) {
        first = false;
        QTimer::singleShot(2000, this, SLOT(enableWwan()));
    } else if (enabled && !m_wwanCheckBox->isChecked() && first) {
        first = false;
        QTimer::singleShot(2000, this, SLOT(disableWwan()));
    } else {
        first = true;
        // it might have changed because we toggled the switch,
        // but it might have been changed externally, so set it anyway
        m_wwanCheckBox->setChecked(enabled);
    }
}

void NMPopup::managerWwanHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wwan enable switch state changed" << enabled;
    m_wwanCheckBox->setEnabled(enabled);
}
#endif

void NMPopup::showMore()
{
    oldShowMore = m_showMoreButton->isChecked();
    showMore(oldShowMore);
}

void NMPopup::showMore(bool more)
{
    if (more) {
        m_showMoreButton->setText(i18nc("pressed show more button", "Show Less..."));
        m_showMoreButton->setIcon(KIcon("list-remove"));
        m_showMoreButton->setChecked(true);
        m_connectionList->setShowAllTypes(true, true); // also refresh list
    } else {
        m_showMoreButton->setText(i18nc("unpressed show more button", "Show More..."));
        m_showMoreButton->setChecked(false);
        m_connectionList->setShowAllTypes(false, true); // also refresh list
        m_showMoreButton->setIcon(KIcon("list-add"));
    }
    kDebug() << m_showMoreButton->text();
}

void NMPopup::checkShowMore(RemoteActivatable * ra)
{
    RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
    if (wic) {
        if (wic->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc &&
            wic->activationState() == Knm::InterfaceConnection::Unknown) {
            return;
        }
        if (wicCount > 0) {
            wicCount--;
        }
        if (wicCount == 0 &&  !m_showMoreButton->isChecked()) {
            // There is no wireless network which the user had explicitly configured around,
            // so temporaly show all the others wireless networks available.
            showMore(true);
        }
    }
}

void NMPopup::uncheckShowMore(RemoteActivatable *ra)
{
    RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
    if (wic) {
        if (wic->operationMode() == Solid::Control::WirelessNetworkInterface::Adhoc &&
            wic->activationState() == Knm::InterfaceConnection::Unknown) {
            return;
        }
        wicCount++;
        if (oldShowMore != m_showMoreButton->isChecked()) {
            // One wireless network explicity configured by the user appeared, reset "Show More" button
            // state to the value before the checkShowMore method above took action.
            showMore(oldShowMore);
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
    InterfaceItem* item = qobject_cast<InterfaceItem*>(sender());
    if (item) {
        m_interfaceDetailsWidget->setInterface(item->interface());
    }

    if (m_leftWidget->currentIndex() == 0) {
        showMore(true);
        m_leftWidget->setCurrentIndex(1);
        // Enable / disable updating of the details widget
        m_interfaceDetailsWidget->setUpdateEnabled(true);

        if (item && item->interface()) {
            // Temporaly disables hightlight for all connections of this interface.
            QMetaObject::invokeMethod(item, "hoverLeave", Qt::QueuedConnection,
                                      Q_ARG(QString, item->interface()->uni()));

            m_leftLabel->setText(QString("<h3>%1</h3>").arg(
                                UiUtils::interfaceNameLabel(item->interface()->uni())));
        }
    } else {
        m_leftLabel->setText(i18nc("title on the LHS of the plasmoid", "<h3>Interfaces</h3>"));
        m_connectionList->clearInterfaces();
        showMore(oldShowMore);
        m_interfaceDetailsWidget->setUpdateEnabled(false);
        m_leftWidget->setCurrentIndex(0);
    }
    //showMore();
}

QSizeF NMPopup::sizeHint (Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF sh = QGraphicsWidget::sizeHint(which, constraint);
    qreal temp1 = m_interfaceDetailsWidget->size().rwidth();

    if (temp1 < 300) {
        temp1 = 300;
    }

    m_leftWidget->setPreferredWidth(temp1);

    return sh;
}
// vim: sw=4 sts=4 et tw=100

