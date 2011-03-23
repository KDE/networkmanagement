/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -i types.h -N -p nm-device-interface introspection/nm-device.xml
 *
 * qdbusxml2cpp is Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef NM_DEVICE_INTERFACE_H
#define NM_DEVICE_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "types.h"

#include "knm_export.h"

/*
 * Proxy class for interface org.freedesktop.NetworkManager.Device
 */
class KNM_EXPORT OrgFreedesktopNetworkManagerDeviceInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.NetworkManager.Device"; }

public:
    OrgFreedesktopNetworkManagerDeviceInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgFreedesktopNetworkManagerDeviceInterface();

    Q_PROPERTY(uint Capabilities READ capabilities)
    inline uint capabilities() const
    { return qvariant_cast< uint >(property("Capabilities")); }

    Q_PROPERTY(uint DeviceType READ deviceType)
    inline uint deviceType() const
    { return qvariant_cast< uint >(property("DeviceType")); }

    Q_PROPERTY(QDBusObjectPath Dhcp4Config READ dhcp4Config)
    inline QDBusObjectPath dhcp4Config() const
    { return qvariant_cast< QDBusObjectPath >(property("Dhcp4Config")); }

    Q_PROPERTY(QDBusObjectPath Dhcp6Config READ dhcp6Config)
    inline QDBusObjectPath dhcp6Config() const
    { return qvariant_cast< QDBusObjectPath >(property("Dhcp6Config")); }

    Q_PROPERTY(QString Driver READ driver)
    inline QString driver() const
    { return qvariant_cast< QString >(property("Driver")); }

    Q_PROPERTY(bool FirmwareMissing READ firmwareMissing)
    inline bool firmwareMissing() const
    { return qvariant_cast< bool >(property("FirmwareMissing")); }

    Q_PROPERTY(QString Interface READ interface)
    inline QString interface() const
    { return qvariant_cast< QString >(property("Interface")); }

    Q_PROPERTY(int Ip4Address READ ip4Address)
    inline int ip4Address() const
    { return qvariant_cast< int >(property("Ip4Address")); }

    Q_PROPERTY(QDBusObjectPath Ip4Config READ ip4Config)
    inline QDBusObjectPath ip4Config() const
    { return qvariant_cast< QDBusObjectPath >(property("Ip4Config")); }

    Q_PROPERTY(QDBusObjectPath Ip6Config READ ip6Config)
    inline QDBusObjectPath ip6Config() const
    { return qvariant_cast< QDBusObjectPath >(property("Ip6Config")); }

    Q_PROPERTY(QString IpInterface READ ipInterface)
    inline QString ipInterface() const
    { return qvariant_cast< QString >(property("IpInterface")); }

    Q_PROPERTY(bool Managed READ managed)
    inline bool managed() const
    { return qvariant_cast< bool >(property("Managed")); }

    Q_PROPERTY(uint State READ state)
    inline uint state() const
    { return qvariant_cast< uint >(property("State")); }

    Q_PROPERTY(QString Udi READ udi)
    inline QString udi() const
    { return qvariant_cast< QString >(property("Udi")); }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> Disconnect()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("Disconnect"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void StateChanged(uint new_state, uint old_state, uint reason);
};

#endif
