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

NetworkManager::NetworkManager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(0),
      m_svgFile("widgets/networkmanager"),
      m_elementName("app-knetworkmanager"),
      m_networkEngine(0),
      m_iconSize(48,48)
{
    setDrawStandardBackground(false);
    setHasConfigurationInterface(false);
    setContentSize(48, 48);
}

void NetworkManager::init()
{
    connect(this, SIGNAL(clicked()), this, SLOT(showMenu()));

    m_icon = new Plasma::Svg(m_svgFile, this);
    m_icon->setContentType(Plasma::Svg::ImageSet);
    m_icon->resize(contentSize());
    
    m_networkEngine = dataEngine("networkmanager");
    m_networkEngine->connectSource("Network Management", this);
    m_elementName = m_networkEngine->query("Network Management")["icon"].toString();

    if (m_elementName.isEmpty()) {
        setFailedToLaunch(true, "Icon could not be found.");
    }
}

NetworkManager::~NetworkManager()
{
    if (!failedToLaunch()) {
        m_networkEngine = 0;
        disconnect(this, SIGNAL(clicked()), this, SLOT(showMenu()));
        delete m_icon;
    }
}

void NetworkManager::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Applet::paintInterface(p,option,rect);
    paintNetworkStatus(p,rect);
}

void NetworkManager::paintNetworkStatus(QPainter *p, const QRect &contentsRect)
{
    if(!m_elementName.isEmpty()) {
        m_icon->paint(p,contentsRect,m_elementName);
        kDebug() << "Using icon: " << m_elementName;
    } else {
        kDebug() << "Couldn't find a valid icon. Tried: " << m_elementName;
    }
}

void NetworkManager::constraintsUpdated(Plasma::Constraints constraints)
{
    setDrawStandardBackground(false);
    if (m_icon && constraints & Plasma::SizeConstraint) {
        m_icon->resize(contentSize());
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
            emit clicked();
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

void NetworkManager::showMenu()
{
    kDebug() << "Menu activated.";
}

#include "networkmanager.moc"
