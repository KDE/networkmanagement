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
#include <solid/device.h>
#include <solid/networking.h>
#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/network.h>
#include <solid/control/wirelessnetwork.h>
#include <solid/control/authentication.h>

//kde specific includes
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>
#include <klocalizedstring.h>

NetworkManager::NetworkManager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_profileConfig(),
      m_svgFile("networkmanager/networkmanager"),
      m_icon(this),
      m_elementName("app-knetworkmanager"),
      m_networkEngine(0),
      m_iconSize(64,64),
      m_profileMenu(new NMMenu()),
      m_profileDlg(0)
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
    if (m_profileDlg == 0) {
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
    m_profileDlg->show();
}

void NetworkManager::scanForNetworks()
{
    kDebug() << "Scanning for networks.";
}

void NetworkManager::launchProfile(const QString &profile)
{
    kDebug() << profile << " has been launched.";

    KConfigGroup activeGroup(&m_profileConfig, profile);
    QStringList unis = activeGroup.readEntry("InterfaceList", QStringList());
    foreach (const QString &uni, unis) {
        Solid::Control::NetworkInterface iface(uni);
        if (iface.signalStrength() != -1) { //wireless
            kDebug() << "Connecting to a wireless network.";
            connectWirelessNetwork(iface, activeGroup);
        }
    }
}

void NetworkManager::connectWiredNetwork(Solid::Control::NetworkInterface &iface, const KConfigGroup &config)
{
    Q_UNUSED(config)

    if (!iface.isValid() ) {
        kDebug() << "Wired interface could not be created.";
        return;
    }

    Solid::Control::Network *network = iface.networks()[0];
    network->setActivated(true);
}

void NetworkManager::connectWirelessNetwork(Solid::Control::NetworkInterface &iface, const KConfigGroup &config)
{
    if (!iface.isValid() ) {
        kDebug() << "Wired interface could not be created.";
        return;
    }

    Solid::Control::NetworkList networks = iface.networks();
    KConfigGroup authGroup(&config, "Encryption");
    Solid::Control::Authentication *auth;
    Solid::Control::Authentication::SecretMap secrets;

    foreach (Solid::Control::Network *network, networks) {
        Solid::Control::WirelessNetwork *wifiNet = (Solid::Control::WirelessNetwork*)network;
        if(wifiNet->essid() == config.readEntry("ESSID", QString())) {
            kDebug() << wifiNet->essid() << " found.  Connecting . . . ";
            switch (authGroup.readEntry("WirelessSecurityType", (int)EncryptionSettingsWidget::None)) {
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
                            secrets["key"] = authGroup.readEntry(QString("WEPKey%1").arg(authGroup.readEntry("WEPStaticKey1", QString())), QString());
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
            wifiNet->setAuthentication(auth);
            network->setActivated(true);
        }
    }
}

void NetworkManager::saveConfig()
{
    globalConfig().sync();
}

#include "networkmanager.moc"
