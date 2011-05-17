/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -N -i types.h -p nm-exported-connection-secrets-interface introspection/nm-connection-secrets.xml
 *
 * qdbusxml2cpp is Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef NM_EXPORTED_CONNECTION_SECRETS_INTERFACE_H
#define NM_EXPORTED_CONNECTION_SECRETS_INTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "types.h"

/*
 * Proxy class for interface org.freedesktop.NetworkManager.Settings.Connection.Secrets
 */
class OrgFreedesktopNetworkManagerSettingsConnectionSecretsInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.NetworkManager.Settings.Connection.Secrets"; }

public:
    OrgFreedesktopNetworkManagerSettingsConnectionSecretsInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgFreedesktopNetworkManagerSettingsConnectionSecretsInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<QVariantMapMap> GetSecrets(const QString &setting_name)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(setting_name);
        return asyncCallWithArgumentList(QLatin1String("GetSecrets"), argumentList);
    }

Q_SIGNALS: // SIGNALS
};

#endif
