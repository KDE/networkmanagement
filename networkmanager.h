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
//#include "widgets/manageprofilewidget.h"

#include <QHash>
#include <QMenu>

#include <KDialog>

#include <plasma/applet.h>
#include <plasma/dataengine.h>
#include <plasma/svg.h>

#include <solid/control/networkmanager.h>
#include <solid/control/networkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirelessaccesspoint.h>

class QPointF;
class QGraphicsSceneMouseEvent;

class NetworkManager : public Plasma::Applet
{
    Q_OBJECT

    public:
        NetworkManager(QObject *parent, const QVariantList &args);
        ~NetworkManager();

        void init();
        Qt::Orientations expandingDirections() const;
        void paintInterface(QPainter *p, const QStyleOptionGraphicsItem *option, const QRect &rect);

    public Q_SLOTS:
        void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
        void showMenu(QPointF clickedPos);
        void manageProfiles();
        void scanForNetworks();
        void launchProfile(const QString &profile);
        void saveConfig();

    Q_SIGNALS:
        void clicked(QPointF clickedPos);

    protected:
        void constraintsEvent(Plasma::Constraints constraints);

    private:
        void paintNetworkStatus(QPainter *p, const QRect &contentsRect);

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        //interface activation methods
        //TODO: move these to the dataengine once Plasma::Services are implemented
        void loadProfile(const QString &profile);
        void activateCurrentProfile();
        void deactivateCurrentProfile();
        void connectInterface(int interfaceIndex);
        void disconnectInterface(int interfaceIndex);
        void connectWiredNetwork(Solid::Control::NetworkInterface *iface);
        void connectWirelessNetwork(Solid::Control::NetworkInterface *iface);
        void loadEncryption(Solid::Control::WirelessNetworkInterface *wifiNet, const KConfigGroup &config);
        void onNetworkConnectionFailed();
        void onInterfaceLinkUp(int interfaceIndex);

        KConfigGroup m_profileConfig;
        QString m_svgFile;
        Plasma::Svg m_icon;
        QString m_elementName;
        Plasma::DataEngine *m_networkEngine;
        QSizeF m_iconSize;
        QPointF m_clickStartPos;

        //creation and selection of profiles
        NMMenu *m_profileMenu;
        KDialog *m_profileDlg;
        //ManageProfileWidget *m_manageProfile;

        //connection management
        QStringList m_interfaceList;
        QList<bool> m_interfaceUpList;
        QString m_activeProfile;
        int m_currentInterfaceIndex;
        bool m_stayConnected;
};

K_EXPORT_PLASMA_APPLET(networkmanager, NetworkManager)

#endif
