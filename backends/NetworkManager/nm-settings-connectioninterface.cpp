/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -i types.h -N -p nm-settings-connectioninterface introspection/nm-settings-connection.xml
 *
 * qdbusxml2cpp is Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "nm-settings-connectioninterface.h"

/*
 * Implementation of interface class OrgFreedesktopNetworkManagerSettingsConnectionInterface
 */

OrgFreedesktopNetworkManagerSettingsConnectionInterface::OrgFreedesktopNetworkManagerSettingsConnectionInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgFreedesktopNetworkManagerSettingsConnectionInterface::~OrgFreedesktopNetworkManagerSettingsConnectionInterface()
{
}

