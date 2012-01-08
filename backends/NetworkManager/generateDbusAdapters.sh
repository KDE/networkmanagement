#!/bin/bash

#Adaptors
qdbusxml2cpp -a exportedconnection -N -i marshalarguments.h -i types.h -i busconnection.h -l BusConnection -c ConnectionAdaptor introspection/nm-exported-connection.xml

#Proxies
qdbusxml2cpp -N -i types.h -p nm-active-connectioninterface introspection/nm-active-connection.xml
qdbusxml2cpp -i types.h -N -p nm-settings-connectioninterface introspection/nm-settings-connection.xml
qdbusxml2cpp -N -i types.h -p nm-settingsinterface introspection/nm-settings.xml
qdbusxml2cpp -m -p nm-vpn-connectioninterface introspection/nm-vpn-connection.xml

# 'default' is a forbidden property name, change it to getDefault
sed -i -e "s,Q_PROPERTY(bool Default READ default),Q_PROPERTY(bool Default READ getDefault)," nm-active-connectioninterface.h
sed -i -e "s,inline bool default() const,inline bool getDefault() const," nm-active-connectioninterface.h

# Another manual workarounds should be done while generating adapter and proxy classes
# 0- Remove org.freedesktop.NetworkManager.Connection.Active interface from introspection/nm-vpn-connection.xml
# 1- Fix double inclusion guards of qdbusxml2cpp like: NM-ACTIVE_H_12345 -> NM_ACTIVE_H (no numbers and dashes)
# 2- Add last an additional parameter to GetSecrets method in type: const QDBusMessage & message
# 3- Clean all tp: namespace tags since qdbusxml2cpp does not support namespaces: http://mail.kde.org/pipermail/kde-hardware-devel/2010-November/001136.html
