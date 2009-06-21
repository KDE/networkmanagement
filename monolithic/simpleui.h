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

#ifndef SIMPLEUI_H
#define SIMPLEUI_H

#include "activatableobserver.h"

#include <QHash>

class QAction;
class KMenu;
class ActivatableList;

namespace Experimental
{
    class KNotificationItem;
} // namespace Experimental

class SimpleUi : public ActivatableObserver
{
Q_OBJECT
public:
    SimpleUi(ActivatableList * list, QObject * parent);
    ~SimpleUi();
protected slots:
    // respond to activatable changes
    void handleAdd(Knm::Activatable *);
    void handleChange(Knm::Activatable *);
    void handleRemove(Knm::Activatable *);
    void activatableActionTriggered();
private:
    static QString iconForActivatable(Knm::Activatable *);
    Experimental::KNotificationItem * m_notificationItem;
    KMenu * m_popup;
    QHash<Knm::Activatable *, QAction *> m_actions;
};
#endif // SIMPLEUI_H
