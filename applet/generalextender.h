/***************************************************************************
 *   Copyright 2009 by Sebastian Kügler <sebas@kde.org>                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef GENERALEXTENDER_H
#define GENERALEXTENDER_H

// Qt
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>

// KDE
#include <KToolInvocation>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/CheckBox>
#include <Plasma/ExtenderItem>


class GeneralExtender : public Plasma::ExtenderItem
{
    Q_OBJECT

    public:
        GeneralExtender(Plasma::Extender *ext = 0);
        virtual ~GeneralExtender();

        QGraphicsWidget* graphicsWidget();

    public Q_SLOTS:
        void managerWirelessEnabledChanged(bool);
        void managerWirelessHardwareEnabledChanged(bool);
        void wirelessEnabledToggled(bool checked);
        void manageConnections();

    private:
        QGraphicsWidget* m_widget;
        QGraphicsLinearLayout* m_layout;
        Plasma::CheckBox* m_rfCheckBox;
        Plasma::IconWidget* m_connectionsButton;
};

#endif

