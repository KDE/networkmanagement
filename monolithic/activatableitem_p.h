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

#include "activatableitem.h"

#include "activatable.h"

class QGridLayout;
class QHBoxLayout;
class QLabel;
class IconSizeSpacer;

class ActivatableItemPrivate
{
Q_DECLARE_PUBLIC(ActivatableItem)
protected:
    ActivatableItem *q_ptr;
public:
    ActivatableItemPrivate();
    virtual ~ActivatableItemPrivate();
    void init(QWidget * widgetParent);
    Knm::Activatable * activatable;

    QGridLayout * outerLayout;
    QHBoxLayout * innerLayout;

    QLabel * activeIcon;
    QLabel * mainLabel;

    IconSizedSpacer * spacer;

    bool hovered;
    bool first;
};

