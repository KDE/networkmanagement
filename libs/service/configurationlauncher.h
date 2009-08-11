/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFIGURATIONLAUNCHER_H
#define CONFIGURATIONLAUNCHER_H

#include "activatableobserver.h"

#include <solid/control/wirelessaccesspoint.h>

#include "knm_export.h"

class ConfigurationLauncherPrivate;

/**
 * Responsible for starting configuration of unconfigured wireless networks
 */
class KNM_EXPORT ConfigurationLauncher : public QObject, public ActivatableObserver
{
Q_OBJECT
public:
    ConfigurationLauncher(QObject * parent);
    ~ConfigurationLauncher();
    /**
     * 1) Listen to newly added WirelessNetworks' activated signals and configure networks for them
     * 2) examine newly added WirelessInterfaceConnections and activate them if they match the
     * previously activated WirelessNetwork
     * @reimp ActivatableObserver
     */
    void handleAdd(Knm::Activatable *);
    /**
     * noop impl
     * @reimp ActivatableObserver
     */
    void handleUpdate(Knm::Activatable *);
    /**
     * noop impl
     * @reimp ActivatableObserver
     */
    void handleRemove(Knm::Activatable *);

protected Q_SLOTS:
    /**
     * Create a connection to a hidden wireless network
     */
    void configureHiddenWirelessNetwork(const QString & ssid, const QString & deviceUni);
    void wirelessNetworkActivated();
    void unconfiguredInterfaceActivated();
private:
    void configureWirelessNetworkInternal(const QString & ssid, const QString & deviceUni);
    Q_DECLARE_PRIVATE(ConfigurationLauncher)
    ConfigurationLauncherPrivate * d_ptr;
};
#endif // CONFIGURATIONLAUNCHER_H
