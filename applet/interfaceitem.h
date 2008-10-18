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

#ifndef APPLET_INTERFACEITEM_H
#define APPLET_INTERFACEITEM_H

#include <QGraphicsWidget>

#include <Plasma/Icon>
#include <Plasma/Label>

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
namespace Solid
{
    namespace Control
    {
        class NetworkInterface;
    } // namespace Control
} // namespace Solid

namespace Plasma
{
    class Icon;
    class Label;
} // namespace Plasma

class InterfaceItem : public QGraphicsWidget
{
Q_OBJECT
public:
    enum NameDisplayMode {InterfaceName, HardwareName};
    InterfaceItem(Solid::Control::NetworkInterface * iface, NameDisplayMode = InterfaceName, QGraphicsItem* parent = 0);
    virtual ~InterfaceItem();
    void setNameDisplayMode(NameDisplayMode);
    NameDisplayMode nameDisplayMode() const;

public slots:
    void activeConnectionsChanged();
    void connectionStateChanged(int);
protected:
    /**
     * The interface is unavailable for connections, update the UI
     */
    virtual void setUnavailable();
    /**
     * The interface is inactive but could be activated
     */
    virtual void setInactive();
    /**
     * The interface is currently active, update the UI
     */
    virtual void setActiveConnection(int);

    Solid::Control::NetworkInterface * m_iface;
    //QGraphicsLinearLayout * m_layout; // use with nested linear based layouts, when they work
    QGraphicsGridLayout * m_layout;
    QGraphicsLinearLayout * m_infoLayout;
    Plasma::Icon * m_icon;
    Plasma::Label * m_ifaceNameLabel;
    Plasma::Label * m_connectionNameLabel;
    QGraphicsLinearLayout * m_connectionInfoLayout;
    Plasma::Label * m_connectionInfoIpAddrLabel;
    Plasma::Label * m_connectionInfoStrengthLabel;
    Plasma::Icon * m_connectionInfoSecurityIcon;
    Plasma::Icon * m_connectButton;
    NameDisplayMode m_nameMode;
};
#endif // APPLET_INTERFACEWIDGET_H
