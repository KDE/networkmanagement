/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -a exportedconnectionsecrets -N -i marshalarguments.h -i types.h -i busconnection.h -l BusConnection -c SecretsAdaptor introspection/nm-connection-secrets.xml
 *
 * qdbusxml2cpp is Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef EXPORTEDCONNECTIONSECRETS_H
#define EXPORTEDCONNECTIONSECRETS_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "marshalarguments.h"
#include "types.h"
#include "busconnection.h"
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.freedesktop.NetworkManager.Settings.Connection.Secrets
 */
class SecretsAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.NetworkManager.Settings.Connection.Secrets")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.freedesktop.NetworkManager.Settings.Connection.Secrets\">\n"
"    <method name=\"GetSecrets\">\n"
"      <annotation value=\"impl_exported_connection_get_secrets\" name=\"org.freedesktop.DBus.GLib.CSymbol\"/>\n"
"      <annotation value=\"\" name=\"org.freedesktop.DBus.GLib.Async\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"setting_name\">\n"
"      </arg>\n"
"      <arg direction=\"in\" type=\"as\" name=\"hints\">\n"
"      </arg>\n"
"      <arg direction=\"in\" type=\"b\" name=\"request_new\">\n"
"      </arg>\n"
"      <arg direction=\"out\" type=\"a{sa{sv}}\" name=\"secrets\">\n"
"        <annotation value=\"QVariantMapMap\" name=\"com.trolltech.QtDBus.QtTypeName.Out0\"/>\n"
"      </arg>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    SecretsAdaptor(BusConnection *parent);
    virtual ~SecretsAdaptor();

    inline BusConnection *parent() const
    { return static_cast<BusConnection *>(QObject::parent()); }

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QVariantMapMap GetSecrets(const QString &setting_name, const QStringList &hints, bool request_new, const QDBusMessage &);
Q_SIGNALS: // SIGNALS
};

#endif
