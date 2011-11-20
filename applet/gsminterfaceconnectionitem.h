/*
Copyright 2008-2010 Sebastian Kügler <sebas@kde.org>
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010-2011 Lamarque Souza <lamarque@gmail.com>

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

#ifndef APPLET_GSMINTERFACECONNECTIONITEM_H
#define APPLET_GSMINTERFACECONNECTIONITEM_H

#include "activatableitem.h"
#include "remotegsminterfaceconnection.h"

class QGraphicsGridLayout;

namespace Plasma
{
    class IconWidget;
    class Label;
    class Meter;
}


/**
    Represents a gsm network or a known connection
 */
class GsmInterfaceConnectionItem : public ActivatableItem
{
    Q_OBJECT
    public:
        GsmInterfaceConnectionItem(RemoteGsmInterfaceConnection * remote, QGraphicsItem * parent = 0);
        virtual ~GsmInterfaceConnectionItem();
        void setupItem();

    private Q_SLOTS:
        void setQuality(int quality);
        void setAccessTechnology(const int tech);
        void updateGsmInfo();
        void activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState);

    private:
        void paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = 0);

        //bool readSettings();
        QGraphicsGridLayout* m_layout;
        Plasma::Meter* m_strengthMeter;
        bool m_layoutIsDirty;
};

#endif //#define APPLET_GSMCONNECTIONITEM_H
