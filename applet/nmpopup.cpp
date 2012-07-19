/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
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

// Own
#include "nmpopup.h"

// Qt
#include <QGraphicsLinearLayout>
#include <QGraphicsGridLayout>
#include <QTimer>
#include <QDBusConnection>
#include <QCheckBox>

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
#include "paths.h"

NMPopup::NMPopup(RemoteActivatableList * activatableList, QGraphicsWidget* parent)
: QGraphicsWidget(parent),
    m_activatables(activatableList),
    m_hasWirelessInterface(false),
    m_widget(0),
    m_mainLayout(0),
    m_warning(0),
    m_leftWidget(0),
    m_leftLayout(0),
    m_interfaceLayout(0),
    m_connectionList(0),
    m_vpnItem(0)
{
    init();
}

NMPopup::~NMPopup()
{
}

void NMPopup::init()
{
    m_mainLayout = new QGraphicsGridLayout(this);

    m_leftLabel = new Plasma::Label(this);
    m_leftLabel->setMaximumHeight(24);
    m_leftLabel->setMinimumHeight(24);
    m_leftLabel->setText(i18nc("title on the LHS of the plasmoid", "<h3>Interfaces</h3>"));
    m_mainLayout->addItem(m_leftLabel, 0, 0);

    m_rightLabel = new Plasma::Label(this);
    m_rightLabel->setMaximumHeight(24);
    m_rightLabel->setMinimumHeight(24);
    m_rightLabel->setText(i18nc("title on the RHS of the plasmoid", "<h3>Connections</h3>"));
    m_mainLayout->addItem(m_rightLabel, 0, 2);

    Plasma::Separator* sep = new Plasma::Separator(this);
    sep->setOrientation(Qt::Vertical);
    m_mainLayout->addItem(sep, 0, 1, 2, 1, Qt::AlignRight);
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
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkingEnabledChanged(bool)),
            this, SLOT(managerNetworkingEnabledChanged(bool)));

    // flight-mode checkbox
    m_wwanCheckBox = new Plasma::CheckBox(m_leftWidget);
    m_wwanCheckBox->setText(i18nc("CheckBox to enable or disable wwan (mobile broadband) interface)", "Enable mobile broadband"));
    m_wwanCheckBox->hide();
    m_wwanCheckBox->nativeWidget()->setTristate(true);
    checkboxLayout->addItem(m_wwanCheckBox, 0, 1);

    connect(m_wwanCheckBox, SIGNAL(toggled(bool)), SLOT(wwanEnabledToggled(bool)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wwanEnabledChanged(bool)),
            this, SLOT(managerWwanEnabledChanged(bool)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wwanHardwareEnabledChanged(bool)),
            this, SLOT(managerWwanHardwareEnabledChanged(bool)));

    // flight-mode checkbox
    m_wifiCheckBox = new Plasma::CheckBox(m_leftWidget);
    m_wifiCheckBox->setText(i18nc("CheckBox to enable or disable wireless interface (rfkill)", "Enable wireless"));
    m_wifiCheckBox->hide();
    checkboxLayout->addItem(m_wifiCheckBox, 1, 0);

    connect(m_wifiCheckBox, SIGNAL(toggled(bool)), SLOT(wirelessEnabledToggled(bool)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wirelessEnabledChanged(bool)),
            this, SLOT(managerWirelessEnabledChanged(bool)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(wirelessHardwareEnabledChanged(bool)),
            this, SLOT(managerWirelessHardwareEnabledChanged(bool)));

    m_leftLayout->addItem(checkboxWidget);

    m_leftWidget->addTab(i18nc("tabbar on the left side", "Interfaces"), m_leftLayout);
    m_leftWidget->setTabBarShown(false); // TODO: enable

    m_interfaceDetailsWidget = new InterfaceDetailsWidget(m_leftWidget);
    m_interfaceDetailsWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    connect(m_interfaceDetailsWidget, SIGNAL(back()), this, SLOT(toggleInterfaceTab()));

    // Hack to prevent graphical artifacts during tab transition.
    connect(m_leftWidget, SIGNAL(currentChanged(int)), SLOT(refresh()));

    m_leftWidget->addTab(i18nc("details for the interface", "Details"), m_interfaceDetailsWidget);

    m_mainLayout->addItem(m_leftWidget, 1, 0);

    m_rightWidget = new QGraphicsWidget(this);
    m_rightWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_rightLayout = new QGraphicsLinearLayout(m_rightWidget);
    m_rightLayout->setOrientation(Qt::Vertical);

    // List with activatables
    m_connectionList = new ActivatableListWidget(m_activatables, m_rightWidget);
    m_connectionList->addType(Knm::Activatable::InterfaceConnection);
    m_connectionList->addType(Knm::Activatable::WirelessInterfaceConnection);
    m_connectionList->addType(Knm::Activatable::VpnInterfaceConnection);
    m_connectionList->addType(Knm::Activatable::GsmInterfaceConnection);
    m_connectionList->init();

    m_connectionList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_connectionList->setPreferredHeight(240);
    m_connectionList->setShowAllTypes(true, true);
    connect(m_connectionList, SIGNAL(showInterfaceDetails(QString)), SLOT(showInterfaceDetails(QString)));

    m_rightLayout->addItem(m_connectionList);

    m_connectionsButton = new Plasma::IconWidget(m_rightWidget);
    m_connectionsButton->setDrawBackground(true);
    m_connectionsButton->setOrientation(Qt::Horizontal);
    //m_connectionsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    m_connectionsButton->setIcon(KIcon("configure"));
    m_connectionsButton->setText(i18nc("manage connections button in the applet's popup", "Manage Connections..."));
    m_connectionsButton->setMinimumHeight(28);
    m_connectionsButton->setMaximumHeight(28);
    connect(m_connectionsButton, SIGNAL(clicked()), this, SLOT(manageConnections()));

    m_showMoreButton = new Plasma::IconWidget(this);
    m_showMoreButton->setDrawBackground(true);
    m_showMoreButton->setOrientation(Qt::Horizontal);
    m_showMoreButton->setToolTip(i18nc("@info:tooltip tooltip for the 'Show More' button", "Show all available networks"));
    // Do not use this according to KDE HIG. Bug #272492
    //m_showMoreButton->setCheckable(true);
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
    connect(m_activatables, SIGNAL(activatableAdded(RemoteActivatable*,int)), this, SLOT(uncheckShowMore(RemoteActivatable*)));
    connect(m_activatables, SIGNAL(activatableRemoved(RemoteActivatable*)), this, SLOT(checkShowMore(RemoteActivatable*)));

    QGraphicsLinearLayout* connectionLayout = new QGraphicsLinearLayout;
    connectionLayout->addItem(m_showMoreButton);
    connectionLayout->addItem(m_connectionsButton);

    m_rightLayout->addItem(connectionLayout);

    m_mainLayout->addItem(m_rightWidget, 1, 2);
    m_mainLayout->setColumnFixedWidth(2, m_showMoreButton->size().rwidth() + m_connectionsButton->size().rwidth());

    //createTab(Knm::Activatable::WirelessInterfaceConnection);
    kDebug() << "Adding interfaces initially";
    foreach (Solid::Control::NetworkInterfaceNm09 * iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        addInterfaceInternal(iface);
        kDebug() << "Network Interface:" << iface->interfaceName() << iface->driver() << iface->designSpeed();
    }
    addVpnInterface();
    // hook up signals to allow us to change the connection list depending on APs present, etc
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceAdded(QString)),
            SLOT(interfaceAdded(QString)));
    connect(Solid::Control::NetworkManagerNm09::notifier(), SIGNAL(networkInterfaceRemoved(QString)),
            SLOT(interfaceRemoved(QString)));

    m_showMoreChecked = false;
    m_oldShowMoreChecked = true;
    wicCount = 0; // number of wireless networks which user explicitly configured using the kcm module.

    KNetworkManagerServicePrefs::instance(Knm::NETWORKMANAGEMENT_RCFILE);
    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));
    m_oldShowMoreChecked = config.readEntry(QLatin1String("ShowAllConnections"), true);
    showMore(m_oldShowMoreChecked);

    readConfig();

    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.connect("org.kde.Solid.PowerManagement", "/org/kde/Solid/PowerManagement", "org.kde.Solid.PowerManagement", "resumingFromSuspend", this, SLOT(readConfig()));
    dbus.connect("org.kde.kded", "/org/kde/networkmanagement", "org.kde.networkmanagement", "ReloadConfig", this, SLOT(readConfig()));

    adjustSize();
}

void NMPopup::readConfig()
{
    kDebug();
    KNetworkManagerServicePrefs::self()->readConfig();

    m_networkingCheckBox->setChecked(Solid::Control::NetworkManagerNm09::isNetworkingEnabled());
    m_wifiCheckBox->setChecked(Solid::Control::NetworkManagerNm09::isWirelessEnabled());

    /* There is a bug in Solid < 4.6.2 where it does not emit the wirelessHardwareEnabledChanged signal.
     * So we always enable the wireless checkbox for versions below 4.6.2. */
#if KDE_IS_VERSION(4,6,2)
    m_wifiCheckBox->setEnabled(Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled());
#else
    m_wifiCheckBox->setEnabled(true);
#endif

    m_showMoreButton->setEnabled(Solid::Control::NetworkManagerNm09::isNetworkingEnabled() &&
                                 Solid::Control::NetworkManagerNm09::isWirelessEnabled());

    m_wwanCheckBox->nativeWidget()->setCheckState(Solid::Control::NetworkManagerNm09::isWwanEnabled() ? Qt::Checked : Qt::Unchecked);
    m_wwanCheckBox->setEnabled(Solid::Control::NetworkManagerNm09::isWwanHardwareEnabled());

    foreach(InterfaceItem * i, m_interfaces) {
        i->setNameDisplayMode(InterfaceItem::InterfaceName);
    }

    QString version = Solid::Control::NetworkManagerNm09::version();
    if (version.isEmpty()) {
        if (!m_warning) {
            m_warning = new Plasma::Label(this);
        }
        m_warning->setText(i18nc("Warning about wrong NetworkManager version", "NetworkManager is not running. Please start it."));
        m_interfaceLayout->addItem(m_warning);
    } else if (Solid::Control::NetworkManagerNm09::compareVersion(QString(MINIMUM_NM_VERSION_REQUIRED)) < 0) {
        if (!m_warning) {
            m_warning = new Plasma::Label(this);
        }
        m_warning->setText(i18nc("Warning about wrong NetworkManager version", "We need at least NetworkManager-%1 to work properly, found '%2'. Please upgrade to a newer version.", QString(MINIMUM_NM_VERSION_REQUIRED), version));
        m_interfaceLayout->addItem(m_warning);
    } else if (m_warning) {
        m_warning->deleteLater();
        m_warning = 0;
    }
}

// Interfaces
void NMPopup::interfaceAdded(const QString& uni)
{
    if (m_interfaces.contains(uni)) {
        return;
    }
    Solid::Control::NetworkInterfaceNm09 * iface = Solid::Control::NetworkManagerNm09::findNetworkInterface(uni);
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
        updateHasWwan();
    }
}

void NMPopup::deleteInterfaceItem()
{
    // slot is called from animation's finished()
    InterfaceItem* item = dynamic_cast<InterfaceItem*>(sender());
    m_interfaceLayout->removeItem(item);
    delete item;
}

Solid::Control::NetworkInterfaceNm09* NMPopup::defaultInterface()
{
    // In fact we're returning the first available interface,
    // and if there is none available just the first one we have
    // and if we don't have one, 0. Make sure you check though.
    if (Solid::Control::NetworkManagerNm09::networkInterfaces().isEmpty()) {
        return 0;
    }
    Solid::Control::NetworkInterfaceNm09* iface = Solid::Control::NetworkManagerNm09::networkInterfaces().first();
    foreach (Solid::Control::NetworkInterfaceNm09 * _iface, Solid::Control::NetworkManagerNm09::networkInterfaces()) {
        switch (_iface->connectionState()) {
            case Solid::Control::NetworkInterfaceNm09::Disconnected:
            case Solid::Control::NetworkInterfaceNm09::Preparing:
            case Solid::Control::NetworkInterfaceNm09::Configuring:
            case Solid::Control::NetworkInterfaceNm09::NeedAuth:
            case Solid::Control::NetworkInterfaceNm09::IPConfig:
            case Solid::Control::NetworkInterfaceNm09::IPCheck:
            case Solid::Control::NetworkInterfaceNm09::Secondaries:
            case Solid::Control::NetworkInterfaceNm09::Activated:
            case Solid::Control::NetworkInterfaceNm09::Deactivating:
            case Solid::Control::NetworkInterfaceNm09::Failed:
                return _iface;
                break;
            case Solid::Control::NetworkInterfaceNm09::Unavailable:
            case Solid::Control::NetworkInterfaceNm09::Unmanaged:
            case Solid::Control::NetworkInterfaceNm09::UnknownState:
            default:
                break;
        }
    }
    return iface;
}

void NMPopup::addInterfaceInternal(Solid::Control::NetworkInterfaceNm09* iface)
{
    if (!iface) {
        // the interface might be gone in the meantime...
        return;
    }
    if (!m_interfaces.contains(iface->uni())) {
        InterfaceItem * ifaceItem = 0;
        if (iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
            // Create the wireless interface item
            WirelessInterfaceItem* wifiItem = 0;
            wifiItem = new WirelessInterfaceItem(static_cast<Solid::Control::WirelessNetworkInterfaceNm09 *>(iface), m_activatables, InterfaceItem::InterfaceName, this);
            ifaceItem = wifiItem;
            //connect(wirelessinterface, SIGNAL(stateChanged()), this, SLOT(updateNetworks()));
            wifiItem->setEnabled(Solid::Control::NetworkManagerNm09::isWirelessEnabled());
            kDebug() << "WiFi added";
            connect(wifiItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        } else {
            // Create the interfaceitem
            ifaceItem = new InterfaceItem(static_cast<Solid::Control::WiredNetworkInterfaceNm09 *>(iface), m_activatables, InterfaceItem::InterfaceName, this);
            connect(ifaceItem, SIGNAL(disconnectInterfaceRequested(QString)), m_connectionList, SLOT(deactivateConnection(QString)));
        }
        connect(ifaceItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
        connect(ifaceItem, SIGNAL(clicked(Solid::Control::NetworkInterfaceNm09*)),
                m_connectionList,  SLOT(addInterface(Solid::Control::NetworkInterfaceNm09*)));
        connect(ifaceItem, SIGNAL(hoverEnter(QString)), m_connectionList, SLOT(hoverEnter(QString)));
        connect(ifaceItem, SIGNAL(hoverLeave(QString)), m_connectionList, SLOT(hoverLeave(QString)));

        // Catch connection changes
        connect(iface, SIGNAL(connectionStateChanged(int,int,int)), this, SLOT(handleConnectionStateChange(int,int,int)));
        m_interfaceLayout->addItem(ifaceItem);
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
    m_vpnItem = new VpnInterfaceItem(0, m_activatables, InterfaceItem::InterfaceName, this);
    connect(m_vpnItem, SIGNAL(clicked()), this, SLOT(toggleInterfaceTab()));
    connect(m_vpnItem, SIGNAL(clicked(Solid::Control::NetworkInterfaceNm09*)),
            m_connectionList,  SLOT(addInterface(Solid::Control::NetworkInterfaceNm09*)));
    connect(m_vpnItem, SIGNAL(hoverEnter()), m_connectionList, SLOT(vpnHoverEnter()));
    connect(m_vpnItem, SIGNAL(hoverLeave()), m_connectionList, SLOT(vpnHoverLeave()));

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
        case Solid::Control::NetworkInterfaceNm09::Disconnected:
        case Solid::Control::NetworkInterfaceNm09::Preparing:
        case Solid::Control::NetworkInterfaceNm09::Configuring:
        case Solid::Control::NetworkInterfaceNm09::NeedAuth:
        case Solid::Control::NetworkInterfaceNm09::IPConfig:
        case Solid::Control::NetworkInterfaceNm09::IPCheck:
        case Solid::Control::NetworkInterfaceNm09::Secondaries:
        case Solid::Control::NetworkInterfaceNm09::Activated:
        case Solid::Control::NetworkInterfaceNm09::Deactivating:
        case Solid::Control::NetworkInterfaceNm09::Failed:
            return true;
            break;
        case Solid::Control::NetworkInterfaceNm09::Unavailable:
        case Solid::Control::NetworkInterfaceNm09::Unmanaged:
        case Solid::Control::NetworkInterfaceNm09::UnknownState:
        default:
            return false;
            break;
    }
    return false;
}

void NMPopup::wirelessEnabledToggled(bool checked)
{
    kDebug() << "Applet wireless enable switch toggled" << checked;
    if (Solid::Control::NetworkManagerNm09::isWirelessEnabled() != checked) {
        Solid::Control::NetworkManagerNm09::setWirelessEnabled(checked);
    }
    updateHasWireless(checked);
}

void NMPopup::wwanEnabledToggled(bool checked)
{
    kDebug() << "Applet wwan enable switch toggled" << checked;
    if (Solid::Control::NetworkManagerNm09::isWwanEnabled() != checked) {
        Solid::Control::NetworkManagerNm09::setWwanEnabled(checked);
    }
}

void NMPopup::networkingEnabledToggled(bool checked)
{
    if (checked && m_warning) {
        m_warning->deleteLater();
        m_warning = 0;
    }

    // Switch networking on / off
    if (Solid::Control::NetworkManagerNm09::isNetworkingEnabled() != checked) {
        Solid::Control::NetworkManagerNm09::setNetworkingEnabled(checked);
    }
    // Update wireless checkbox
    m_wifiCheckBox->setChecked(Solid::Control::NetworkManagerNm09::isWirelessEnabled());

kDebug() << "Wireless hardware enabled ==" << Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled();

#if KDE_IS_VERSION(4,6,2)
    m_wifiCheckBox->setEnabled(Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled() || Solid::Control::NetworkManagerNm09::isWirelessEnabled());
#endif

    m_wwanCheckBox->nativeWidget()->setCheckState(Solid::Control::NetworkManagerNm09::isWwanEnabled() ? Qt::Checked : Qt::Unchecked);
    m_wwanCheckBox->setEnabled(Solid::Control::NetworkManagerNm09::isWwanHardwareEnabled() || Solid::Control::NetworkManagerNm09::isWwanEnabled());

    updateHasWireless(checked);
}

void NMPopup::updateHasWireless(bool checked)
{
    //kDebug() << "UPDATE!!!!!!!!!!!!";
    bool hasWireless = true;
    if (!Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled() ||
        !Solid::Control::NetworkManagerNm09::isNetworkingEnabled() ||
        !Solid::Control::NetworkManagerNm09::isWirelessEnabled() || !checked) {

        //kDebug () << "networking enabled?" << Solid::Control::NetworkManagerNm09::isNetworkingEnabled();
        //kDebug () << "wireless hardware enabled?" << Solid::Control::NetworkManagerNm09::isWirelessHardwareEnabled();
        //kDebug () << "wireless enabled?" << Solid::Control::NetworkManagerNm09::isWirelessEnabled();

        // either networking is disabled, or wireless is disabled
        hasWireless = false;
    }
    //solid is too slow, we need to see if the checkbox was checked by the user
    if (checked)
        hasWireless = true;
    kDebug() << "After chckboxn" << hasWireless;

    foreach (InterfaceItem* ifaceitem, m_interfaces) {
        Solid::Control::NetworkInterfaceNm09* iface = ifaceitem->interface();
        if (iface && iface->type() == Solid::Control::NetworkInterfaceNm09::Wifi) {
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
        Solid::Control::ModemNetworkInterfaceNm09* iface = qobject_cast<Solid::Control::ModemNetworkInterfaceNm09 *>(ifaceitem->interface());
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

    m_showMoreButton->setEnabled(enabled && Solid::Control::NetworkManagerNm09::isNetworkingEnabled());
}

void NMPopup::managerWirelessHardwareEnabledChanged(bool enabled)
{
    kDebug() << "Hardware wireless enable switch state changed" << enabled;
    m_wifiCheckBox->setEnabled(enabled);
    updateHasWireless(enabled);
    m_showMoreButton->setEnabled(enabled && Solid::Control::NetworkManagerNm09::isNetworkingEnabled());
}

void NMPopup::managerNetworkingEnabledChanged(bool enabled)
{
    kDebug() << "NM daemon changed networking enable state" << enabled;
    m_networkingCheckBox->setChecked(enabled);
    m_showMoreButton->setEnabled(enabled);
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
    config.writeEntry(QLatin1String("ShowAllConnections"), m_oldShowMoreChecked);
    config.sync();
}

void NMPopup::showMore(bool more)
{
    if (!Solid::Control::NetworkManagerNm09::isWirelessEnabled()) {
        more = false;
    }

    if (more) {
        m_showMoreButton->setText(i18nc("pressed show more button", "Show Less..."));
        m_showMoreButton->setIcon(KIcon("list-remove"));
        m_showMoreChecked = true;
        m_connectionList->setShowAllTypes(true, true); // also refresh list
    } else {
        m_showMoreButton->setText(i18nc("unpressed show more button", "Show More..."));
        m_showMoreChecked = false;
        m_connectionList->setShowAllTypes(false, true); // also refresh list
        m_showMoreButton->setIcon(KIcon("list-add"));
    }
    kDebug() << m_showMoreButton->text();
}

void NMPopup::checkShowMore(RemoteActivatable * ra)
{
    RemoteWirelessInterfaceConnection * wic = qobject_cast<RemoteWirelessInterfaceConnection*>(ra);
    if (wic) {
        if (wic->operationMode() == Solid::Control::WirelessNetworkInterfaceNm09::Adhoc &&
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
        if (wic->operationMode() == Solid::Control::WirelessNetworkInterfaceNm09::Adhoc &&
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
    InterfaceItem* item = qobject_cast<InterfaceItem*>(sender());
    if (item) {
        m_interfaceDetailsWidget->setInterface(item->interface());
    }

    if (m_leftWidget->currentIndex() == 0) {
        // Enable / disable updating of the details widget
        m_interfaceDetailsWidget->setUpdateEnabled(true);

        if (item) {
            if (item->interface()) {
                // Temporaly disables hightlight for all connections of this interface.
                QMetaObject::invokeMethod(item, "hoverLeave", Qt::QueuedConnection,
                                          Q_ARG(QString, item->interface()->uni()));

                m_leftLabel->setText(QString("<h3>%1</h3>").arg(
                                    UiUtils::interfaceNameLabel(item->interface()->uni())));
            } else {
                // Temporaly disables hightlight for all VPN connections.
                QMetaObject::invokeMethod(item, "hoverLeave", Qt::QueuedConnection);
            }
        }
        showMore(true);

        // Hack to prevent graphical artifact during tab transition.
        // m_interfaceDetailsWidget will be shown again when transition finishes.
        m_interfaceDetailsWidget->hide();
        m_leftWidget->setCurrentIndex(1);
    } else {
        m_leftLabel->setText(i18nc("title on the LHS of the plasmoid", "<h3>Interfaces</h3>"));
        m_connectionList->clearInterfaces();
        showMore(m_oldShowMoreChecked);
        m_interfaceDetailsWidget->setUpdateEnabled(false);
        m_leftWidget->setCurrentIndex(0);
    }
}

void NMPopup::refresh()
{
    m_interfaceDetailsWidget->show();
}
// vim: sw=4 sts=4 et tw=100

