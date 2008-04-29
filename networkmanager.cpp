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

NetworkManager::NetworkManager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_svgFile("networkmanager/networkmanager"),
      m_icon(this),
      m_elementName("app-knetworkmanager"),
      m_networkEngine(0),
      m_iconSize(64,64),
      m_profileMenu(new NMMenu())
{
    setHasConfigurationInterface(false);
    m_icon.setImagePath(m_svgFile);
}

void NetworkManager::init()
{
    m_profileMenu->setConfig(globalConfig());
    connect(m_profileMenu, SIGNAL(createProfileRequested()), this, SLOT(createProfile()));
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
        disconnect(m_profileMenu, SIGNAL(editProfileRequested()), this, SLOT(editProfile()));
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
    m_profileMenu->popup(popupPosition(m_profileMenu->geometry().size()));
}

void NetworkManager::editProfile()
{
    kDebug() << "Creating a new profile.";
}

void NetworkManager::scanForNetworks()
{
    kDebug() << "Scanning for networks.";
}

void NetworkManager::launchProfile(const QString &profile)
{
    kDebug() << profile << " has been launched.";
}

#include "networkmanager.moc"
