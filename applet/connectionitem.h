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

#ifndef APPLET_CONNECTIONITEM_H
#define APPLET_CONNECTIONITEM_H

#include <QGraphicsWidget>

class QGraphicsGridLayout;

class RemoteConnection;

namespace Plasma
{
    class Icon;
    class Label;
}

/**
 * Represents an inactive connection
 */
class ConnectionItem : public QGraphicsWidget
{
Q_OBJECT
public:
    ConnectionItem(RemoteConnection * conn, QGraphicsItem * parent = 0);
    virtual ~ConnectionItem();
signals:
    void clicked();
protected:
    RemoteConnection * m_connection;
    QGraphicsGridLayout * m_layout;
    Plasma::Icon * m_icon;
    Plasma::Label * m_connectionNameLabel;
    Plasma::Icon * m_connectButton;
};

#endif //#define APPLET_CONNECTIONITEM_H

