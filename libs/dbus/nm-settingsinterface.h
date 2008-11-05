/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -N -m -p nm-settingsinterface /space/kde/sources/trunk/playground/base/plasma/applets/networkmanager/settings/dbus/introspection/nm-settings.xml
 *
 * dbusxml2cpp is Copyright (C) 2006 Trolltech ASA. All rights reserved.
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef NM_SETTINGSINTERFACE_H_1223376691
#define NM_SETTINGSINTERFACE_H_1223376691

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include "../knm_export.h"

/*
 * Proxy class for interface org.freedesktop.NetworkManagerSettings
 */
class KNM_EXPORT OrgFreedesktopNetworkManagerSettingsInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.NetworkManagerSettings"; }

public:
    OrgFreedesktopNetworkManagerSettingsInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgFreedesktopNetworkManagerSettingsInterface();

public Q_SLOTS: // METHODS
    inline QDBusReply<QList<QDBusObjectPath> > ListConnections()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("ListConnections"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void NewConnection(const QDBusObjectPath &connection);
};

#endif
