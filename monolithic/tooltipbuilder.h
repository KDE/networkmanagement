/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef TOOLTIPBUILDER_H
#define TOOLTIPBUILDER_H

#include <QString>

namespace Knm
{
class InterfaceConnection;
} // namespace Knm

class ToolTipBuilder
{
public:
    /**
     * Taking the deviceUni from the InterfaceConnection, use an ordered list of the selected
     * tooltip item keys, and the classes in Solid::Control to generate a HTML table representing
     * the current state of the device
     */
    static QString toolTipForInterfaceConnection(Knm::InterfaceConnection*);
};

#endif // TOOLTIPBUILDER_H
