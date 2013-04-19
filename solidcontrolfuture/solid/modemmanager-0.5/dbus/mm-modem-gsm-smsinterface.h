/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -N -m -p mm-modem-gsm-smsinterface introspection/mm-modem-gsm-sms.xml
 *
 * qdbusxml2cpp is Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef MM_MODEM_GSM_SMSINTERFACE_H
#define MM_MODEM_GSM_SMSINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

#include "generic-types.h"

/*
 * Proxy class for interface org.freedesktop.ModemManager.Modem.Gsm.SMS
 */
class OrgFreedesktopModemManagerModemGsmSMSInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.freedesktop.ModemManager.Modem.Gsm.SMS"; }

public:
    OrgFreedesktopModemManagerModemGsmSMSInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgFreedesktopModemManagerModemGsmSMSInterface();

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<> Delete(uint index)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(index);
        return asyncCallWithArgumentList(QLatin1String("Delete"), argumentList);
    }

    inline QDBusPendingReply<QVariantMap> Get(uint index)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(index);
        return asyncCallWithArgumentList(QLatin1String("Get"), argumentList);
    }

    inline QDBusPendingReply<uint> GetFormat()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetFormat"), argumentList);
    }

    inline QDBusPendingReply<QString> GetSmsc()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("GetSmsc"), argumentList);
    }

    inline QDBusPendingReply<QVariantMapList> List()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QLatin1String("List"), argumentList);
    }

    inline QDBusPendingReply<uint> Save(const QVariantMap &properties)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(properties);
        return asyncCallWithArgumentList(QLatin1String("Save"), argumentList);
    }

    inline QDBusPendingReply<uint> Send(const QVariantMap &properties)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(properties);
        return asyncCallWithArgumentList(QLatin1String("Send"), argumentList);
    }

    inline QDBusPendingReply<> SendFromStorage(uint index)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(index);
        return asyncCallWithArgumentList(QLatin1String("SendFromStorage"), argumentList);
    }

    inline QDBusPendingReply<> SetFormat(uint format)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(format);
        return asyncCallWithArgumentList(QLatin1String("SetFormat"), argumentList);
    }

    inline QDBusPendingReply<> SetIndication(uint mode, uint mt, uint bm, uint ds, uint bfr)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(mode) << qVariantFromValue(mt) << qVariantFromValue(bm) << qVariantFromValue(ds) << qVariantFromValue(bfr);
        return asyncCallWithArgumentList(QLatin1String("SetIndication"), argumentList);
    }

    inline QDBusPendingReply<> SetSmsc(const QString &smsc)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(smsc);
        return asyncCallWithArgumentList(QLatin1String("SetSmsc"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void Completed(uint index, bool completed);
    void SmsReceived(uint index, bool complete);
};

#endif
