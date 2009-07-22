/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "connectionsecretsjob.h"

#include <nm-setting-cdma.h>
#include <nm-setting-connection.h>
#include <nm-setting-wired.h>
#include <nm-setting-ip4-config.h>
// Removed in NM0.7rc1
// #include <nm-setting-ip6-config.h>
#include <nm-setting-8021x.h>
#include <nm-setting-gsm.h>
#include <nm-setting-ppp.h>
#include <nm-setting-pppoe.h>
#include <nm-setting-serial.h>
#include <nm-setting-vpn.h>
#include <nm-setting-wireless.h>
#include <nm-setting-wireless-security.h>

#include <QTimer>

#include <KDialog>
#include <KLocale>
#include <KStandardDirs>

#include "802_11_wirelesswidget.h"
#include "cdmawidget.h"
#include "gsmwidget.h"
#include "ipv4widget.h"
#include "pppoewidget.h"
#include "pppwidget.h"
#include "settingwidget.h"
#include "wiredwidget.h"
#include "security/802_11_wireless_security_widget.h"
#include "security/802_1x_security_widget.h"
#include "security/securitywidget.h"

#include "knmserviceprefs.h"
#include "connection.h"

ConnectionSecretsJob::ConnectionSecretsJob(Knm::Connection* connection, const QString &settingName,
                                           const QStringList& secrets, bool requestNew, const QDBusMessage& request)
    : m_connection(connection), m_connectionPersistence(0), mSettingName(settingName), mRequestNew(requestNew),
      mRequest(request), m_askUserDialog(0), m_settingWidget(0)
{
    // record the secrets that we are looking for
    foreach (QString secretKey, secrets) {
        mSecrets.insert(secretKey, QVariant());
    }
}

ConnectionSecretsJob::~ConnectionSecretsJob()
{
}

void ConnectionSecretsJob::start()
{
    QTimer::singleShot(0, this, SLOT(doWork()));
}

void ConnectionSecretsJob::doWork()
{
    kDebug();
    if (mRequestNew) {
        doAskUser();
    } else {
        QString configFile = KStandardDirs::locate("data",
                Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + m_connection->uuid());
        m_connectionPersistence = new Knm::ConnectionPersistence(m_connection,
                KSharedConfig::openConfig(configFile, KConfig::NoGlobals),
                (KNetworkManagerServicePrefs::self()->storeInWallet() ? Knm::ConnectionPersistence::Secure :
                 Knm::ConnectionPersistence::PlainText)
                );
        connect(m_connectionPersistence, SIGNAL(loadSecretsResult(uint)), this, SLOT(gotPersistedSecrets(uint)));
        m_connectionPersistence->loadSecrets();
    }
}

void ConnectionSecretsJob::gotPersistedSecrets(uint result)
{
    kDebug();
    m_connectionPersistence->deleteLater();
    m_connectionPersistence = 0;
    setError(result);
    if (result == Knm::ConnectionPersistence::EnumError::NoError) {
        emitResult();
    } else {
        doAskUser();
    }
}

void ConnectionSecretsJob::doAskUser()
{
    // popup a dialog showing the appropriate UI for the type of connection
    kDebug();
    if ( mSettingName == QLatin1String(NM_SETTING_802_1X_SETTING_NAME)) {
        //m_settingWidget = Wired8021xSecurityWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        m_settingWidget = new CdmaWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        m_settingWidget = new GsmWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME)) {
        m_settingWidget = new IpV4Widget(m_connection, 0);
    //} else if ( mSettingName == QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME)) {
        // not supported yet
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPP_SETTING_NAME)) {
        m_settingWidget = new PppWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        m_settingWidget = new PppoeWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_SERIAL_SETTING_NAME)) {
        m_settingWidget = new PppWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        // not supported yet, figure out the type of the vpn plugin, load it and its m_settingWidgetget
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        m_settingWidget = new WiredWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        m_settingWidget = new Wireless80211SecurityWidget(false, m_connection, 0, 0, 0); // TODO: find out
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        m_settingWidget = new Wireless80211Widget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        // todo: vpn secrets!
    }

    if (m_settingWidget) {
        m_settingWidget->readConfig();
        m_askUserDialog = new KDialog(0);
        m_askUserDialog->setCaption(i18nc("dialog caption for network secrets request", "Enter Network Connection Secrets"));
        m_askUserDialog->setMainWidget(m_settingWidget);
        m_askUserDialog->setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Apply );

        connect(m_askUserDialog, SIGNAL(okClicked()), SLOT(dialogAccepted()));
        connect(m_askUserDialog, SIGNAL(cancelClicked()), SLOT(dialogRejected()));

        m_askUserDialog->show();
    } else {
        kDebug() << "Setting widget for" << mSettingName << "not yet ported, rejecting secrets request.";
        dialogRejected();
    }
}

void ConnectionSecretsJob::dialogAccepted()
{
    // get results from dialog, put them in mSecrets
    kDebug();
    // m_connection is up to date again
    m_settingWidget->writeConfig();
    // persist the changes
    QString configFile = KStandardDirs::locate("data",
            Knm::ConnectionPersistence::CONNECTION_PERSISTENCE_PATH + m_connection->uuid());
    Knm::ConnectionPersistence cp(m_connection,
            KSharedConfig::openConfig(configFile, KConfig::NoGlobals),
            (KNetworkManagerServicePrefs::self()->storeInWallet() ? Knm::ConnectionPersistence::Secure :
             Knm::ConnectionPersistence::PlainText)
            );
    cp.save();
    m_settingWidget->deleteLater();
    m_askUserDialog->deleteLater();
    emitResult();
}

void ConnectionSecretsJob::dialogRejected()
{
    setError(EnumError::UserInputCancelled);
    m_settingWidget->deleteLater();
    m_askUserDialog->deleteLater();
    emitResult();
}

QString ConnectionSecretsJob::settingName() const
{
    return mSettingName;
}

QVariantMap ConnectionSecretsJob::secrets() const
{
    return mSecrets;
}

QDBusMessage ConnectionSecretsJob::requestMessage() const
{
    return mRequest;
}

Knm::Connection * ConnectionSecretsJob::connection() const
{
    return m_connection;
}

#include "connectionsecretsjob.moc"
// vim: sw=4 sts=4 et tw=100
