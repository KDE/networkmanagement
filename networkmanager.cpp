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
#include <KIconLoader>

NetworkManager::NetworkManager(QObject *parent, const QVariantList &args)
    : Plasma::Applet(parent, args),
      m_icon(0),
      m_svgFile("widgets/networkmanager"),
      m_networkEngine(0),
      m_iconText("app-knetworkmanager"),
      m_iconSize(32,32),
      m_lastSignalStrength(0)
{
    setDrawStandardBackground(false);
    setHasConfigurationInterface(false);
}

void NetworkManager::init()
{
    m_icon = new Plasma::Icon(m_iconText, this);
    m_icon->setSvg(m_svgFile, m_iconText);
    connect(m_icon, SIGNAL(clicked()), this, SLOT(showMenu()));
    
    m_networkEngine = dataEngine("networkmanager");
    m_networkEngine->connectSource("Network Management", this);

   foreach (QString uni, m_networkEngine->query("Network Management")["Network Interfaces"].toStringList()) {
        m_networkEngine->connectSource(uni, this);
    }

    //update status icon
    setIcon(determineNewIcon());

    if (m_iconText.isEmpty()) {
        setFailedToLaunch(true, "Icon could not be found.");
    }
}

NetworkManager::~NetworkManager()
{
    if (!failedToLaunch()) {
        m_networkEngine = 0;
        disconnect(m_icon, SIGNAL(clicked()), this, SLOT(showMenu()));
        delete m_icon;
    }
}

void NetworkManager::paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect)
{
    Applet::paintInterface(p,option,rect);
}

void NetworkManager::constraintsUpdated(Plasma::Constraints constraints)
{
    setDrawStandardBackground(false);

    if (constraints & Plasma::FormFactorConstraint) {
        if (formFactor() == Plasma::Planar ||
            formFactor() == Plasma::MediaCenter) {
            m_icon->setText(m_iconText);
            setMinimumContentSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Desktop)));
            m_icon->setToolTip(Plasma::ToolTipData());
            m_icon->setDrawBackground(false);
        } else {
            m_icon->setText(QString());
            setMinimumContentSize(m_icon->sizeFromIconSize(IconSize(KIconLoader::Small)));
            Plasma::ToolTipData data;
            data.mainText = m_iconText;
            data.image = m_icon->icon().pixmap(IconSize(KIconLoader::Desktop));
            m_icon->setToolTip(data);
            m_icon->setDrawBackground(false);
        }
    }

    if (constraints & Plasma::SizeConstraint) {
        m_icon->resize(contentSize());
        update();
    }
}

void NetworkManager::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    if (source == "Network Management") {
        setIcon(determineNewIcon());
    } else if(data["NetworkType"] == "NetworkInterface") {
        setIcon(determineNewIcon(source, data));
    }
    kDebug() << m_iconText;
    update();
}

QString NetworkManager::determineNewIcon()
{
    Plasma::DataEngine::Data data = m_networkEngine->query("Network Management");
    if (data.contains("Active NetworkInterface")) {
        QString activeUni = data["Active NetworkInterface"].toString();
        kDebug() << "Active network found.  " << activeUni;
        if (!activeUni.isEmpty()) {
            return determineNewIcon(activeUni, m_networkEngine->query(activeUni));
        } else {
            return QString();
        }
    } else if (data["Status"].toString() == "Unconnected") {
        return "action-nm_no_connection";
    } else if (data["Status"].toString() == "Connecting") {
        return "action-nm_connecting";
    }
    kDebug() << "Network Management source not found.";
    return QString();
}

QString NetworkManager::determineNewIcon(const QString &source, const Plasma::DataEngine::Data &data)
{
    kDebug() << "Determining new icon.";
    Plasma::DataEngine::Data nmData = m_networkEngine->query("Network Management");
    if (nmData["Status"].toString() == "Unconnected") {
        return "action-nm_no_connection";
    } else if (data["Connection State"].toString() != "Activated") {
        kDebug() << "Determining Connection State.";
        return determineStageOfConnection(data["Connection State"].toString());
    } else {
        if (source == nmData["Active NetworkInterface"].toString()) {
            if (data["Type"].toString() == "Ieee8023") {
                return "action-nm_device_wired";
            } else if (data["Type"].toString() == "Ieee80211") {
                kDebug() << "Determining Signal Stength.";
                return determineSignalIcon(data["Signal Strength"].toInt());
            } else {
                return QString();
            }
        } else {
            return QString();
        }
    }
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
