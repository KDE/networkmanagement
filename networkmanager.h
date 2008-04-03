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

#include "nmmenu.h"

#include <QHash>
#include <QMenu>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/svg.h>

class QPointF;
class QGraphicsSceneMouseEvent;
class QAction;

class NetworkManager : public Plasma::Applet
{
    Q_OBJECT

    public:
        NetworkManager(QObject *parent, const QVariantList &args);
        ~NetworkManager();

        void init();
        void constraintsUpdated(Plasma::Constraints constraints);
        Qt::Orientations expandingDirections() const;
        void paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect);
        QSizeF contentSizeHint() const;

    public Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
        void showMenu(QPointF clickedPos);
        void createProfile();
        void scanForNetworks();
        void launchProfile(const QString &profile);

    Q_SIGNALS:
        void clicked(QPointF clickedPos);

    private:
        void paintNetworkStatus(QPainter *p, const QRect &contentsRect);

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        
        QString m_svgFile;
        Plasma::Svg m_icon;
        QString m_elementName;
        Plasma::DataEngine *m_networkEngine;
        QSizeF m_iconSize;
        QPointF m_clickStartPos;

        NMMenu *m_profileMenu;
};

K_EXPORT_PLASMA_APPLET(networkmanager, NetworkManager)

#endif
