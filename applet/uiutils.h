/*
Copyright 2008,2009 Sebastian K?gler <sebas@kde.org>

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

#ifndef UIUTILS_H
#define UIUTILS_H

class QPixmap;
class QSizeF;

#include <solid/control/networkinterface.h>

class UiUtils
{
public:
    /**
     * @param state
     * @return A human-readable string describing the network state
     *
     */
    static QString stateDescription();

    /**
     * @return a human-readable name for the network interface type for use as label
     * @param type the type of the network interface
     */
    static QString descriptiveInterfaceName(const Solid::Control::NetworkInterface::Type type);

    /**
     * @return an icon name suitable for the interface type
     * @param type the type of the network interface
     */
    static QString iconName(const Solid::Control::NetworkInterface::Type type);

    static QPixmap interfacePixmap(const QSizeF size, const Solid::Control::NetworkInterface::Type type);

};
#endif // UIUTILS_H
