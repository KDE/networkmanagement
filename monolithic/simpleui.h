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
#include <QStringList>

class QSignalMapper;
class QVBoxLayout; class QWidgetAction;

class KMenu;
class ActivatableItem;
class ActivatableList;
class SortedActivatableList;

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
    void handleUpdate(Knm::Activatable *);
    void handleRemove(Knm::Activatable *);
    void activatableActionTriggered();
    //void updateActionState(Knm::Activatable * changed, QAction * action);
    void slotPreferences();
private:
    void fillPopup();
    SortedActivatableList * m_sortedList;
    Experimental::KNotificationItem * m_notificationItem;
    KMenu * m_popup;
    QVBoxLayout * m_popupLayout;
    QHash<Knm::Activatable *, QWidgetAction *> m_actions;
    QStringList m_deviceUnis;
};
#endif // SIMPLEUI_H
