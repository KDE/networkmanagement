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
      m_icon(m_svgFile, this),
      m_elementName("app-knetworkmanager"),
      m_networkEngine(0),
      m_iconSize(48,48),
      m_profileMenu(new NMMenu())
{
    setDrawStandardBackground(true);
    setHasConfigurationInterface(false);
    setContentSize(48, 48);
}

void NetworkManager::init()
{
    m_profileMenu->setConfig(globalConfig());
    connect(m_profileMenu, SIGNAL(createProfileRequested()), this, SLOT(createProfile()));
    connect(m_profileMenu, SIGNAL(scanForNetworksRequested()), this, SLOT(scanForNetworks()));
    connect(m_profileMenu, SIGNAL(launchProfileRequested(const QString&)), this, SLOT(launchProfile(const QString&)));
    connect(this, SIGNAL(clicked(QPointF)), this, SLOT(showMenu(QPointF)));

    m_icon.resize(contentSize());
    
    m_networkEngine = dataEngine("networkmanager");
    if (!m_networkEngine) {
        setFailedToLaunch(true, "The Network Manager data engine could not be loaded.  Please check to ensure it is installed.");
    }
    m_networkEngine->connectSource("Network Management", this);

    Plasma::DataEngine::Data data = m_networkEngine->query("Network Management");
    if (data["Status"].toString() == "Unknown") {
        setFailedToLaunch(true, "Solid could not determine your connection status.  Ensure that you have a network backend installed.");
    }
    m_elementName = data["icon"].toString();

    if (m_elementName.isEmpty()) {
        //something is wrong here.
        setFailedToLaunch(true, "Network Manager could not determine you connection status.");
    }
}

NetworkManager::~NetworkManager()
{
    if (!failedToLaunch()) {
        disconnect(m_profileMenu, SIGNAL(createProfileRequested()), this, SLOT(createProfile()));
        disconnect(m_profileMenu, SIGNAL(scanForNetworksRequested()), this, SLOT(scanForNetworks()));
        disconnect(m_profileMenu, SIGNAL(launchProfileRequested(const QString&)), this, SLOT(launchProfile(const QString&)));
        disconnect(this, SIGNAL(clicked(QPointF)), this, SLOT(showMenu(QPointF)));
    }
    delete m_profileMenu;
}

void NetworkManager::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    if (&rect == 0) {
        Applet::paintInterface(p,option,rect);
        return;
    }
    
    paintNetworkStatus(p,rect);
    //Applet::paintInterface(p,option,rect);


    /*p->save();
    p->setPen(Qt::white);
    p->drawText(rect,
                Qt::AlignBottom | Qt::AlignHCenter,
                (!m_elementName.isEmpty()) ? m_elementName : QString("Nothing"));
    p->restore();*/
}

void NetworkManager::paintNetworkStatus(QPainter *p, const QRect &contentsRect)
{
    if(!m_elementName.isEmpty()) {
        m_icon.paint(p,contentsRect,m_elementName);
        kDebug() << "Using icon: " << m_elementName;
    } else {
        kDebug() << "Couldn't find a valid icon. Tried: " << m_elementName;
    }
}

void NetworkManager::constraintsUpdated(Plasma::Constraints constraints)
{
    if (constraints & Plasma::SizeConstraint) {
        m_icon.resize(contentSize());
    }
}

Qt::Orientations NetworkManager::expandingDirections() const
{
    if (formFactor() == Plasma::Horizontal) {
        return Qt::Vertical;
    } else {
        return Qt::Horizontal;
    }
}

QSizeF NetworkManager::contentSizeHint() const
{
    QSizeF sizeHint = contentSize();
    switch (formFactor()) {
    case Plasma::Vertical:
        sizeHint.setHeight(sizeHint.width());
        break;
    case Plasma::Horizontal:
        sizeHint.setWidth(sizeHint.height());
        break;
    default:
        break;
    }

    return sizeHint;
}

void NetworkManager::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        Widget::mousePressEvent(event);
        return;
    }

    m_clickStartPos = scenePos();
}

void NetworkManager::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        Widget::mousePressEvent(event);
        return;
    }
    if (m_clickStartPos == scenePos()) {
        if (boundingRect().contains(event->pos())) {
            emit clicked(event->scenePos());
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
    m_profileMenu->popup(clickedPos.toPoint());
}

void NetworkManager::createProfile()
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
