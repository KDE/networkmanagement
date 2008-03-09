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

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/widgets/icon.h>
#include <KIcon>

class NetworkManager : public Plasma::Applet
{
    Q_OBJECT

    public:
        NetworkManager(QObject *parent, const QVariantList &args);
        ~NetworkManager();

        void init();
        void constraintsUpdated(Plasma::Constraints constraints);
        void paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect);

    public Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
        void showMenu();

    private:
        QString determineNewIcon();
        QString determineNewIcon(const QString &source, const Plasma::DataEngine::Data &data);
        QString determineStageOfConnection(const QString &connectionState);
        QString determineSignalIcon(int strength);
        inline void setIcon(const QString &newIconText);
        
        Plasma::Icon *m_icon;
        QString m_svgFile;
        Plasma::DataEngine *m_networkEngine;
        QString m_iconText;
        QSizeF m_iconSize;
        int m_lastSignalStrength;

        static const int signalStrengthResolution = 25;
        static const int hysteresis = 5;
};

void NetworkManager::setIcon(const QString &newIconText)
{
    m_iconText = (newIconText.isEmpty()) ? m_iconText : newIconText;
    m_icon->setSvg(m_svgFile, m_iconText);
}

K_EXPORT_PLASMA_APPLET(networkmanager, NetworkManager)

#endif
