/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -N -m -i generic-types.h -p nm-ip6-configinterface /home/jgrulich/projects/libnm-qt/dbus/introspection/nm-ip6-config.xml
 *
 * qdbusxml2cpp is Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "nm-ip6-config-interface.h"

/*
 * Implementation of interface class OrgFreedesktopNetworkManagerIP6ConfigInterface
 */

OrgFreedesktopNetworkManagerIP6ConfigInterface::OrgFreedesktopNetworkManagerIP6ConfigInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
    qDBusRegisterMetaType<IpV6AddressMap>();
    qDBusRegisterMetaType< QList<IpV6AddressMap> >();
}

OrgFreedesktopNetworkManagerIP6ConfigInterface::~OrgFreedesktopNetworkManagerIP6ConfigInterface()
{
}

#include "nm-ip6-config-interface.moc"