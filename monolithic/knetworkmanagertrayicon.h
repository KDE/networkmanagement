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

#include <knotificationitem-1/knotificationitem.h>
#include <activatableobserver.h>

#include <QHash>
#include <QStringList>

#include <Solid/Networking>
#include <solid/control/networkinterface.h>

class QSignalMapper;
class QVBoxLayout; class QWidgetAction;

class KMenu;
class ActivatableItem;
class ActivatableList;
class SortedActivatableList;

class KNetworkManagerTrayIconPrivate;

class KNetworkManagerTrayIcon : public Experimental::KNotificationItem, public ActivatableObserver
{
Q_OBJECT
Q_DECLARE_PRIVATE(KNetworkManagerTrayIcon);
public:
    KNetworkManagerTrayIcon(Solid::Control::NetworkInterface::Types types, const QString & id, ActivatableList * list, QObject * parent);
    virtual ~KNetworkManagerTrayIcon();
    // respond to activatable changes
    void handleAdd(Knm::Activatable *);
    void handleUpdate(Knm::Activatable *);
    void handleRemove(Knm::Activatable *);
protected Q_SLOTS:
    //void updateActionState(Knm::Activatable * changed, QAction * action);
    void slotPreferences();
    /**
     * Switch the popup between showing networks and showing normal activatables
     */
    //void slotShowNetworks();
    /**
     * Hides the icon when network management is unavailable
     */
    void networkingStatusChanged(Solid::Networking::Status);

    /**
     * Disable wireless in response to user action
     */
    void disableWireless(bool);

    /**
     * Update UI in response to system network management state changes
     */
    void wirelessEnabledChanged();

    /**
     * Starts monitoring new network interfaces' state changes
     */
    void networkInterfaceAdded(const QString&);
    //void networkInterfaceRemoved(const QString&);
    void updateTrayIcon();
    void handleConnectionStateChange(int new_state, int old_state, int reason);
private:
    void fillPopup();
    KNetworkManagerTrayIconPrivate * d_ptr;
};

#endif // SIMPLEUI_H
