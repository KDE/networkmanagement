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
      m_iconSize(48,48),
      m_lastSignalStrength(0),
      m_activeNetworkInterface(),
      m_activeNetwork(),
      m_connectionStatus()
{
    setDrawStandardBackground(false);
    setHasConfigurationInterface(false);
    setContentSize(64, 64);
}

void NetworkManager::init()
{
    connect(this, SIGNAL(clicked()), this, SLOT(showMenu()));

    m_icon = new Plasma::Svg(m_svgFile, this);
    m_icon->setContentType(Plasma::Svg::ImageSet);
    m_icon->resize(contentSize());
    
    m_networkEngine = dataEngine("networkmanager");
    m_networkEngine->connectSource("Network Management", this);
    Plasma::DataEngine::Data nmData = m_networkEngine->query("Network Management");

    m_activeNetworkInterface = nmData["Active NetworkInterface"].toString();
    if (!m_activeNetworkInterface.isEmpty()) {
        m_networkEngine->connectSource(m_activeNetworkInterface, this);
        m_activeNetwork = m_networkEngine->query(m_activeNetworkInterface)["Active Network"].toString();
        if (!m_activeNetwork.isEmpty()) {
            m_networkEngine->connectSource(m_activeNetwork, this);
        }
    }
    m_connectionStatus = nmData["Status"].toString();
    
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
    m_elementName = determineIcon();

    if(!m_elementName.isEmpty()) {
        m_icon->paint(p,contentsRect,m_elementName);
        kDebug() << "Using icon: " << m_elementName;
    } else {
        kDebug() << "Couldn't find an valid icon.";
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
    if (m_clickStartPos != scenePos()) {
        if (boundingRect().contains(event->pos())) {
            emit clicked();
        }
    }
}

void NetworkManager::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (source == "Network Management") {
        QString activeIface = data["Active NetworkInterface"].toString();
        if (activeIface != m_activeNetworkInterface) {
            //disconnect the active iface and network
            m_networkEngine->disconnectSource(m_activeNetworkInterface, this);
            m_networkEngine->disconnectSource(m_activeNetwork, this);
            if (!activeIface.isEmpty()) {
                m_networkEngine->connectSource(activeIface, this);
                QString activeNetwork = m_networkEngine->query(activeIface)["Active Network"].toString();
                if (!activeNetwork.isEmpty()) {
                     m_networkEngine->connectSource(activeNetwork, this);
                }
            }
        }
        m_activeNetworkInterface = activeIface;
        m_connectionStatus = data["Status"].toString();
        m_elementName = determineIcon();
    } else if(data["NetworkType"] == "NetworkInterface") {
        if (m_activeNetworkInterface == source) {
            m_activeNetwork = data["Active Network"].toString();
            m_elementName = determineIcon();
        }
    }
    update();
}

QString NetworkManager::determineIcon()
{
    kDebug() << "Determining new icon.";
    kDebug() << "Connections status: " << m_connectionStatus;
    if (!m_activeNetworkInterface.isEmpty()) {
        kDebug() << "Active network found.  " << m_activeNetworkInterface;
        Plasma::DataEngine::Data data = m_networkEngine->query(m_activeNetworkInterface);
        if (data["Type"].toString() == "Ieee8023") {
                return "action-nm_device_wired";
        } else if (data["Type"].toString() == "Ieee80211") {
            kDebug() << "Determining Signal Stength.";
            return determineSignalIcon(data["Signal Strength"].toInt());
        } else {
            return QString();
        }
    } else if (m_connectionStatus == "Unconnected") {
        return "action-nm_no_connection";
    } else if (m_connectionStatus == "Connecting") {
        return determineStageOfConnection(m_networkEngine->query(m_activeNetworkInterface)["Connection State"].toString());
    }
    return QString();
}

QString NetworkManager::determineStageOfConnection(const QString &connectionState)
{
    if (connectionState == "Prepare" || connectionState == "Configure" || connectionState == "NeedUserKey"
        || connectionState == "IPStart") {
        return "action-stage01";
    } else if(connectionState == "IPGet") {
        return "action-stage02";
    } else if(connectionState == "IPCommit") {
        return "action-stage03";
    } else {
        return "action-nm_no_connection";
    }
}

QString NetworkManager::determineSignalIcon(int strength)
{
    if(strength > m_lastSignalStrength-signalStrengthResolution-hysteresis
       && strength <= m_lastSignalStrength+signalStrengthResolution+hysteresis) {
        return QString("action-nm_signal_%1").arg(m_lastSignalStrength, 2, 10, QLatin1Char('0'));
    } else {
        //HACK: this was hardcoded to a resolution of 25 due to the inefficiencies of a variable resolution
        if (strength <= signalStrengthResolution/2) {
            return "action-nm_signal_00";
        } else if (strength <= 25+signalStrengthResolution/2) {
            return "action-nm_signal_25";
        } else if (strength <= 50+signalStrengthResolution/2) {
            return "action-nm_signal_50";
        } else if (strength <= 75+signalStrengthResolution/2) {
            return "action-nm_signal_75";
        } else if (strength <= 100) {
            return "action-nm_signal_100";
        } else {
            return QString();
        }
    }
}

void NetworkManager::showMenu()
{
    kDebug() << "Menu activated.";
}

#include "networkmanager.moc"
