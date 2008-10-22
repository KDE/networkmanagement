/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLASMA_NETWORKMANAGER_APPLET_H
#define PLASMA_NETWORKMANAGER_APPLET_H

#include <kdeversion.h>

#include <plasma/svg.h>
//#define KDE_4_1
#if KDE_IS_VERSION(4,1,70)
#include <plasma/popupapplet.h>
#else
#include <plasma/applet.h>
#endif

#include <solid/control/networkinterface.h>
class NetworkManagerPopup;

#if KDE_IS_VERSION(4,1,70)
class NetworkManagerApplet : public Plasma::PopupApplet
#else
namespace Plasma
{
class Dialog;
} // namespace Plasma

class NetworkManagerApplet : public Plasma::Applet
#endif
{
Q_OBJECT
public:
    NetworkManagerApplet(QObject * parent, const QVariantList & args);
    ~NetworkManagerApplet();
    /* reimp Plasma::Applet */
    void init();
    /* reimp Plasma::Applet */
    void paintInterface(QPainter *painter, const QStyleOptionGraphicsItem * option, const QRect & rect);
    /* reimp Plasma::Applet */
    //QSizeF sizeHint(const Qt::SizeHint which, const QSizeF& constraint) const;
    /* reimp Plasma::Applet */
    //Qt::Orientations expandingDirections() const;
    /* reimp Plasma::Applet */
    //void constraintsEvent(Plasma::Constraints constraints);
    /* reimp Plasma::Applet */
    void popupEvent(bool show);
    QGraphicsWidget * graphicsWidget();
#if KDE_IS_VERSION(4,1,70)
#else
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void showLegacyPopup(QGraphicsSceneMouseEvent *event);
private:
    Plasma::Dialog * m_dialog;
#endif
private Q_SLOTS:
    void networkInterfaceAdded(const QString&);
    void networkInterfaceRemoved(const QString&);
    void interfaceConnectionStateChanged(int newState);
private:
    void paintInterfaceStatus(Solid::Control::NetworkInterface*, QPainter *painter, const QStyleOptionGraphicsItem * option, const QRect & rect);
    Solid::Control::NetworkInterfaceList sortInterfacesByImportance(const Solid::Control::NetworkInterfaceList& interfaces) const;
    bool m_iconPerDevice;
    Plasma::Svg m_svg;
    NetworkManagerPopup * m_popup;
    QPoint m_clicked;
};

#endif




