/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include "networkmanager.h"

#include <QPainter>
#include <QPointF>
#include <QGraphicsSceneMouseEvent>
#include <QAction>

//solid specific includes
//solid is only used directly until Plasma::Services are complete.
#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

//kde specific includes
#include <KComponentData>
#include <kcmdlineargs.h>
#include <KLocale>
#include <KDebug>

NetworkManager::NetworkManager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_profileConfig(),
      m_svgFile("networkmanager/networkmanager"),
      m_icon(this),
      m_elementName("app-knetworkmanager"),
      m_networkEngine(0),
      m_iconSize(64,64),
      m_profileMenu(new NMMenu()),
      m_profileDlg(0),
      m_interfaceList(),
      m_activeProfile(),
      m_currentInterfaceIndex(-1),
      m_stayConnected(false)
{
    setHasConfigurationInterface(false);
    m_icon.setImagePath(m_svgFile);
}

void NetworkManager::init()
{
    KConfigGroup gconfig = globalConfig();
    m_profileConfig= KConfigGroup(&gconfig, "Profiles");
    m_profileMenu->setConfig(m_profileConfig);
    connect(m_profileMenu, SIGNAL(manageProfilesRequested()), this, SLOT(manageProfiles()));
    connect(m_profileMenu, SIGNAL(scanForNetworksRequested()), this, SLOT(scanForNetworks()));
    connect(m_profileMenu, SIGNAL(launchProfileRequested(const QString&)), this, SLOT(launchProfile(const QString&)));
    connect(this, SIGNAL(clicked(QPointF)), this, SLOT(showMenu(QPointF)));

    m_icon.setContainsMultipleImages(false);
    m_icon.resize(size());

    setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
    
    m_networkEngine = dataEngine("networkmanager");
    if (!m_networkEngine) {
        setFailedToLaunch(true, i18n("The Network Manager data engine could not be loaded.  Please check to ensure it is installed."));
    }
    m_networkEngine->connectSource("Network Management", this);

    Plasma::DataEngine::Data data = m_networkEngine->query("Network Management");
    if (data["Status"].toString() == "Unknown") {
        setFailedToLaunch(true, i18n("Solid could not determine your connection status.  Ensure that you have a network backend installed."));
    }
    m_elementName = data["icon"].toString();

    if (m_elementName.isEmpty()) {
        //something is wrong here.
        setFailedToLaunch(true, i18n("Network Manager could not determine you connection status."));
    }
}

NetworkManager::~NetworkManager()
{
    if (!hasFailedToLaunch()) {
        disconnect(m_profileMenu, SIGNAL(manageProfilesRequested()), this, SLOT(manageProfiles()));
        disconnect(m_profileMenu, SIGNAL(scanForNetworksRequested()), this, SLOT(scanForNetworks()));
        disconnect(m_profileMenu, SIGNAL(launchProfileRequested(const QString&)), this, SLOT(launchProfile(const QString&)));
        disconnect(this, SIGNAL(clicked(QPointF)), this, SLOT(showMenu(QPointF)));
    }
    delete m_profileMenu;
}

void NetworkManager::constraintsEvent(Plasma::Constraints constraints)
{
    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Vertical) {
            kDebug() << "Vertical FormFactor";
            // TODO: set background(true) on panel causes 0 height, so do not use it
            setBackgroundHints(NoBackground);
        } else if (formFactor() == Plasma::Horizontal) {
            kDebug() << "Horizontal FormFactor";
            // TODO: set background(true) on panel causes 0 height, so do not use it
            setBackgroundHints(NoBackground);
        } else if (formFactor() == Plasma::Planar) {
            kDebug() << "Planar FormFactor";
            setBackgroundHints(DefaultBackground);
        } else if (formFactor() == Plasma::MediaCenter) {
            kDebug() << "MediaCenter FormFactor";
            setBackgroundHints(DefaultBackground);
        } else {
            kDebug() << "Other FormFactor" << formFactor();
            setBackgroundHints(DefaultBackground);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        m_icon.resize(size());
    }
}

void NetworkManager::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &contentsRect)
{
    if (&contentsRect == 0) {
        Applet::paintInterface(p,option,contentsRect);
        return;
    }

    paintNetworkStatus(p,contentsRect);
}

void NetworkManager::paintNetworkStatus(QPainter *p, const QRect &contentsRect)
{
    if(!m_elementName.isEmpty()) {
        m_icon.paint(p,contentsRect,m_elementName);
    } /*else {
        kDebug() << "Couldn't find a valid icon. Tried: " << m_elementName;
    }*/
}

Qt::Orientations NetworkManager::expandingDirections() const
{
    if (formFactor() == Plasma::Horizontal) {
        return Qt::Vertical;
    } else {
        return Qt::Horizontal;
    }
}

void NetworkManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    m_clickStartPos = scenePos();
}

void NetworkManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }
    if (m_clickStartPos == scenePos()) {
        if (boundingRect().contains(event->pos())) {
            emit clicked(event->pos());
        }
    }
}

void NetworkManager::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (source == "Network Management") {
        m_elementName = data["icon"].toString();
    } else if (data["NetworkType"].toString() == "NetworkInterface") {
        int index = m_interfaceList.indexOf(source);
        if (index == -1) {
            kDebug() << "Source could not be found.  Ensure that signals are being cleaned up when a new profile is loaded.";
            return;
        }
        if (index == m_currentInterfaceIndex && data["Connection State"] == "Failed") {
            kDebug() << "Network Connection failed.  Trying next . . . ";
            onNetworkConnectionFailed();
        }
        if (data["Link Up"].toBool() != m_interfaceUpList[index]) {
            if (data["Link Up"].toBool()) {
                kDebug() << "A new interface has come online.";
                onInterfaceLinkUp(index);
            } else {
                kDebug() << "A new interface has gone offline.";
                onNetworkConnectionFailed();
            }
        }
        m_interfaceUpList[index] = data["Link Up"].toBool();
    }
    update();
}

void NetworkManager::showMenu(QPointF clickedPos)
{
    Q_UNUSED(clickedPos)

    m_profileMenu->popup(popupPosition(m_profileMenu->geometry().size()));
}

void NetworkManager::manageProfiles()
{
    /*if (m_profileDlg == 0) {
        kDebug() << "Creating a new profile.";
        m_profileDlg = new KDialog();
        m_profileDlg->setCaption("Manage Profiles");
        m_profileDlg->setButtons( KDialog::Ok | KDialog::Cancel);
        m_manageProfile = new ManageProfileWidget(m_profileDlg);
        m_manageProfile->setConfig(m_profileConfig);
        m_profileDlg->setMainWidget(m_manageProfile);
        connect(m_profileDlg, SIGNAL(okClicked()), m_profileMenu, SLOT(reloadProfiles()));
        connect(m_profileDlg, SIGNAL(okClicked()), this, SLOT(saveConfig()));
    }
    m_profileDlg->show();*/
}

void NetworkManager::scanForNetworks()
{
    kDebug() << "Scanning for networks.";
}

void NetworkManager::launchProfile(const QString &profile)
{
    kDebug() << profile << " has been launched.";

//     deactivateCurrentProfile();
//     loadProfile(profile);
//     activateCurrentProfile();
}

void NetworkManager::deactivateCurrentProfile()
{
    //don't try to reconnect when the network is taken down
    m_stayConnected = false;
    disconnectInterface(m_currentInterfaceIndex);
    m_activeProfile.clear();
    m_currentInterfaceIndex=-1;

    if (m_interfaceList.isEmpty()) {
        return;
    }
}

void NetworkManager::loadProfile(const QString &profile)
{
    //unload previous profile
    /*foreach (const QString &interface, m_interfaceList) {
        //disconnect all sources
        m_networkEngine->disconnectSource(interface, this);
    }
    m_interfaceList.clear();
    m_interfaceUpList.clear();

    //load the new profile
    m_stayConnected = true;
    m_activeProfile = profile;

    KConfigGroup config(&m_profileConfig, profile);
    m_interfaceList = config.readEntry("InterfaceList", QStringList());
    foreach (const QString &interface, m_interfaceList) {
        m_networkEngine->connectSource(interface, this);
        m_interfaceUpList << m_networkEngine->query(interface)["Link Up"].toBool();
    }*/
    return;
}

void NetworkManager::activateCurrentProfile()
{
    /*if (m_interfaceList.isEmpty()) {
        kDebug() << "No profile has been loaded.";
        return;
    }

    connectInterface(0);//connect to the first interface*/
    return;
}

void NetworkManager::disconnectInterface(int interfaceIndex)
{
    /*if (interfaceIndex < 0 || interfaceIndex >= m_interfaceList.size()) {
        kDebug() << "Tried to load an out-of-bound interface number: " << interfaceIndex << ".  Only " << m_interfaceList.size() << " are known.";
        return;
    }
    
    Solid::Control::NetworkInterface iface(m_interfaceList[interfaceIndex]);
    Solid::Control::Network *activeNetwork = iface.findNetwork(iface.activeNetwork());
    if (activeNetwork != 0) {
        return;//FIXME: the instruction below causes a crash.  This should change with the Solid::Control::Network* re-write
        //activeNetwork->setActivated(false);
    }*/
    return;
}

void NetworkManager::connectInterface(int interfaceIndex)
{
    /*if (interfaceIndex < 0 || interfaceIndex >= m_interfaceList.size()) {
        kDebug() << "Tried to load an out-of-bound interface number: " << interfaceIndex << ".  Only " << m_interfaceList.size() << " are known.";
        return;
    }
    m_currentInterfaceIndex = interfaceIndex;
    
    Solid::Control::NetworkInterface iface(m_interfaceList[m_currentInterfaceIndex]);
    if (iface.type() == Solid::Control::NetworkInterface::Ieee8023) {
        connectWiredNetwork(iface);
    } else if(iface.type() == Solid::Control::NetworkInterface::Ieee80211) {
        connectWirelessNetwork(iface);
    }*/return;
}

void NetworkManager::connectWiredNetwork(Solid::Control::NetworkInterface *iface)
{
    /*if (!iface.isValid() ) {
        kDebug() << "Wired interface could not be created.";
        return;
    }

    Solid::Control::Network *network = iface.networks()[0];
    network->setActivated(true);*/return;
}

void NetworkManager::connectWirelessNetwork(Solid::Control::NetworkInterface *iface)
{
    /*if (!iface.isValid() ) {
        kDebug() << "Wired interface could not be created.";
        return;
    }

    KConfigGroup config(&m_profileConfig, m_activeProfile);
    foreach (Solid::Control::Network *network, iface.networks()) {
        Solid::Control::WirelessNetwork *wifiNet = (Solid::Control::WirelessNetwork*)network;
        if(wifiNet->essid() == config.readEntry("ESSID", QString())) {
            kDebug() << wifiNet->essid() << " found.  Connecting . . . ";
            loadEncryption((Solid::Control::WirelessNetwork*)network, config);
            network->setActivated(true);
        }
    }*/return;
}

void NetworkManager::loadEncryption(Solid::Control::WirelessNetworkInterface *wifiNet, const KConfigGroup &config)
{
    /*int encType = config.readEntry("WirelessSecurityType", (int)EncryptionSettingsWidget::None);
    kDebug() << "Using encryption type: " << encType;

    KConfigGroup authGroup(&config, "Encryption");
    Solid::Control::Authentication *auth;
    Solid::Control::Authentication::SecretMap secrets;
    
    switch (encType) {
        case EncryptionSettingsWidget::None:
            kDebug() << "No encryption loaded.";
            auth = new Solid::Control::AuthenticationNone();
            break;
        case EncryptionSettingsWidget::Wep:
            kDebug() << "Using Wep.";
            Solid::Control::AuthenticationWep *authwep = new Solid::Control::AuthenticationWep();;
            authwep->setType((Solid::Control::AuthenticationWep::WepType)authGroup.readEntry("WEPEncryptionKeyType", 0));
            authwep->setMethod((Solid::Control::AuthenticationWep::WepMethod)authGroup.readEntry("WEPAuthentication", 0));
            int wepType = authGroup.readEntry("WEPType", 0);
            if (wepType == 0) {
                authwep->setKeyLength(64);
            } else {
                authwep->setKeyLength(128);
            }
            switch(authwep->type()) {
                case EncryptionSettingsWidget::Ascii:
                case EncryptionSettingsWidget::Hex:
                    secrets["key"] = authGroup.readEntry(QString("WEPStaticKey%1").arg(authGroup.readEntry("WEPKey", 0)+1), QString());//key is zero indexed.
                    authwep->setSecrets(secrets);
                    break;
                case EncryptionSettingsWidget::Passphrase:
                    secrets["key"] = authGroup.readEntry("WEPPassphrase", QString());
                    authwep->setSecrets(secrets);
                    break;
            }
            auth = dynamic_cast<Solid::Control::Authentication*>(authwep);
            break;
    }
    wifiNet->setAuthentication(auth);*/return;
}

void NetworkManager::onNetworkConnectionFailed()
{
    /*kDebug() << "Connection failed.";
    //connection failed.  Try to connect to the next network.
    if (m_currentInterfaceIndex+1 == m_interfaceList.size()) {
        kDebug() << "All interfaces have failed.  Aborting.";
        return;
    }

    if (m_stayConnected) {
        connectInterface(m_currentInterfaceIndex+1);
    }*/return;
}

void NetworkManager::onInterfaceLinkUp(int interfaceIndex)
{
    /*if (interfaceIndex < m_currentInterfaceIndex) {
        disconnectInterface(m_currentInterfaceIndex);
        connectInterface(interfaceIndex); //note that if this fails the previous interface will eventually become active again.
    }*/return;
}

void NetworkManager::saveConfig()
{
    globalConfig().sync();
}

#include "networkmanager.moc"
