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

#include "connectionpersistence.h"

#include <KConfigGroup>
#include <kwallet.h>

#include "connection.h"
#include "setting.h"
#include "settingpersistence.h"

#include "settings/802-11-wireless.h"
#include "settings/802-11-wirelesspersistence.h"
#include "settings/802-11-wireless-security.h"
#include "settings/802-11-wireless-securitypersistence.h"
#include "settings/802-1x.h"
#include "settings/802-1xpersistence.h"
#include "settings/802-3-ethernet.h"
#include "settings/802-3-ethernetpersistence.h"
#include "settings/cdma.h"
#include "settings/cdmapersistence.h"
#include "settings/gsm.h"
#include "settings/gsmpersistence.h"
#include "settings/ipv4.h"
#include "settings/ipv4persistence.h"
#include "settings/ppp.h"
#include "settings/ppppersistence.h"
#include "settings/pppoe.h"
#include "settings/pppoepersistence.h"
#include "settings/serial.h"
#include "settings/serialpersistence.h"
#include "settings/vpn.h"
#include "settings/vpnpersistence.h"

using namespace Knm;

const QString ConnectionPersistence::NETWORKMANAGEMENT_RCFILE = QLatin1String("networkmanagementrc");
const QString ConnectionPersistence::CONNECTION_PERSISTENCE_PATH = QLatin1String("networkmanagement/connections/");

QString ConnectionPersistence::s_walletFolderName = QLatin1String("Network Management");

WId ConnectionPersistence::s_walletWId = 0;

ConnectionPersistence::ConnectionPersistence(Connection * conn, KSharedConfig::Ptr config, SecretStorageMode mode)
    : m_connection(conn), m_config(config), m_storageMode(mode)
{
}

ConnectionPersistence::ConnectionPersistence(KSharedConfig::Ptr config, SecretStorageMode mode)
    : m_config(config), m_storageMode(mode)
{
    KConfigGroup connection(config, "connection");
    QString uuid = connection.readEntry("uuid");
    QString type = connection.readEntry("type");
    if (uuid.isEmpty() || type.isEmpty()) {
        m_connection = 0;
    } else {
        m_connection = new Connection(QUuid(uuid), Connection::typeFromString(type));
        kDebug() << m_connection->uuid();
    }
}


ConnectionPersistence::~ConnectionPersistence()
{
    qDeleteAll(m_persistences.values());
}

Connection * ConnectionPersistence::connection() const
{
    return m_connection;
}

SettingPersistence * ConnectionPersistence::persistenceFor(Setting * setting)
{
    SettingPersistence * sp = m_persistences.value(setting);
    if (!sp)
        switch (setting->type()) {
            case Setting::Cdma:
                sp = new CdmaPersistence(static_cast<CdmaSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Gsm:
                sp = new GsmPersistence(static_cast<GsmSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Ipv4:
                sp = new Ipv4Persistence(static_cast<Ipv4Setting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Ppp:
                sp = new PppPersistence(static_cast<PppSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Pppoe:
                sp = new PppoePersistence(static_cast<PppoeSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Security8021x:
                sp = new Security8021xPersistence(static_cast<Security8021xSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Serial:
                sp = new SerialPersistence(static_cast<SerialSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Vpn:
                sp = new VpnPersistence(static_cast<VpnSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Wired:
                sp = new WiredPersistence(static_cast<WiredSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::Wireless:
                sp = new WirelessPersistence(static_cast<WirelessSetting*>(setting), m_config, m_storageMode);
                break;
            case Setting::WirelessSecurity:
                sp = new WirelessSecurityPersistence(
                        static_cast<WirelessSecuritySetting*>(setting), m_config, m_storageMode
                        );
                break;
        }
    if (sp) {
        m_persistences.insert(setting, sp);
    }
    return sp;
}

void ConnectionPersistence::save()
{
    // save connection settings
    KConfigGroup cg(m_config, "connection");
    cg.writeEntry("id", m_connection->name());
    cg.writeEntry("uuid", m_connection->uuid().toString());
    cg.writeEntry("type", Connection::typeAsString(m_connection->type()));
    cg.writeEntry("autoconnect", m_connection->autoConnect());
    cg.writeEntry("timestamp", m_connection->timestamp());

    // save each setting
    foreach (Setting * setting, m_connection->settings()) {
        SettingPersistence * sp = persistenceFor(setting);
        sp->save();

    }
    m_config->sync();

    // factor out to make a pure Qt version
    bool readyForWalletWrite = false;
    if (m_connection->hasSecrets() && m_storageMode == ConnectionPersistence::Secure) {
        KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), walletWid(), KWallet::Wallet::Synchronous );
        if( wallet && wallet->isOpen() ) {
            if( !wallet->hasFolder( s_walletFolderName ) )
                wallet->createFolder( s_walletFolderName );
            if ( wallet->setFolder( s_walletFolderName ) ) {
                readyForWalletWrite = true;
            }
        }

        // could be merged with above loop for speed but this keeps the
        // kde wallet dependencies in one place
        if (readyForWalletWrite) {
            foreach (Setting * setting, m_connection->settings()) {
                SettingPersistence * sp = persistenceFor(setting);
                QMap<QString,QString> secrets = sp->secrets();
                if (!secrets.isEmpty()) {
                    wallet->writeMap(walletKeyFor(setting), secrets);
                }
            }
        }
    }
}

void ConnectionPersistence::load()
{
    // load connection settings
    KConfigGroup cg(m_config, "connection");
    if (cg.exists()) { // don't bother to try if the KConfigGroup doesn't exist, save opening the wallet too
        m_connection->setName(cg.readEntry("id"));
        m_connection->setAutoConnect(cg.readEntry<bool>("autoconnect", false));
        m_connection->setTimestamp(cg.readEntry<QDateTime>("timestamp", QDateTime()));

        // load each setting
        foreach (Setting * setting, m_connection->settings()) {
            SettingPersistence * sp = persistenceFor(setting);
            sp->load();
        }
    }
}

QString ConnectionPersistence::walletKeyFor(const Setting * setting) const
{
    return m_connection->uuid() + ';' + setting->name();
}

void ConnectionPersistence::loadSecrets()
{
    KConfigGroup cg(m_config, "connection");
    if (cg.exists()) {
        if (m_storageMode != ConnectionPersistence::Secure) {
            foreach (Setting * setting, m_connection->settings()) {
                setting->setSecretsAvailable(true);
                emit loadSecretsResult(EnumError::NoError);
            }
        } else if (!m_connection->hasSecrets() ||
                m_connection->secretsAvailable()) {
            emit loadSecretsResult(EnumError::NoError);
        } else if (KWallet::Wallet::isEnabled()) {
            kDebug() << "opening wallet...";
            KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(),
                    walletWid(), KWallet::Wallet::Asynchronous);
            if (wallet) {
                disconnect(wallet, SIGNAL(walletOpened(bool)), this, 0);
                connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForRead(bool)));
            } else {
                emit loadSecretsResult(EnumError::WalletNotFound);
            }
        } else {
            emit loadSecretsResult(EnumError::WalletDisabled);
        }
    }
    return;
}

void ConnectionPersistence::walletOpenedForRead(bool success)
{
    if (success) {
        KWallet::Wallet * wallet = static_cast<KWallet::Wallet*>(sender());
        if (wallet->isOpen() && wallet->hasFolder(s_walletFolderName) && wallet->setFolder(s_walletFolderName)) {
            kDebug() << "Reading all entries for connection";
            QMap<QString,QMap<QString,QString> > entries;
            QString key = m_connection->uuid() + QLatin1String("*");

            if (wallet->readMapList(key, entries) == 0) {
                foreach (Setting * setting, m_connection->settings()) {
                    QString settingKey = walletKeyFor(setting);

                    if (entries.contains(settingKey)) {
                        QMap<QString,QString> settingSecrets = entries.value(settingKey);
                        kDebug() << settingSecrets;
                        persistenceFor(setting)->restoreSecrets(settingSecrets);
                    }
                }
                kDebug() << "Check connection:";
                kDebug() << "secretsAvailable:" << m_connection->secretsAvailable();

                emit loadSecretsResult(EnumError::NoError);
            } else {
                kDebug() << "Wallet::readEntryList for :" << key << " failed";
                emit loadSecretsResult(EnumError::MissingContents);
            }
        }
    } else {
        emit loadSecretsResult(EnumError::WalletOpenRefused);
    }
}

// vim: sw=4 sts=4 et tw=100
