/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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
#include <nm-setting-ip6-config.h>
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
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KWindowSystem>

#include "802_11_wirelesswidget.h"
#include "cdmawidget.h"
#include "gsmwidget.h"
#include "ipv4widget.h"
#include "ipv6widget.h"
#include "pppoewidget.h"
#include "pppwidget.h"
#include "settingwidget.h"
#include "wiredwidget.h"
#include "security/wirelesssecurityauth.h"
#include "security/securitywidget.h"
#include "security/securitywired8021x.h"
#include "security/security8021xauth.h"

#include "settings/vpn.h"

#include "knmserviceprefs.h"
#include "connection.h"
#include "vpnuiplugin.h"

ConnectionSecretsJob::ConnectionSecretsJob(Knm::Connection* connection, const QString &settingName,
                                           const QStringList& secrets)
    : m_connection(connection), mSettingName(settingName), mSecrets(secrets), m_askUserDialog(0), m_settingWidget(0)
{
}

ConnectionSecretsJob::~ConnectionSecretsJob()
{
    delete m_askUserDialog;
    delete m_settingWidget;
}

void ConnectionSecretsJob::start()
{
    QTimer::singleShot(0, this, SLOT(doAskUser()));
}

void ConnectionSecretsJob::doAskUser()
{
    // popup a dialog showing the appropriate UI for the type of connection
    kDebug();
    KDialog::ButtonCodes buttonCodes = KDialog::Ok | KDialog::Cancel;
    if ( mSettingName == QLatin1String(NM_SETTING_802_1X_SETTING_NAME)) {
        m_settingWidget = new Security8021xAuthWidget(m_connection, mSecrets, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        m_settingWidget = new CdmaWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        m_settingWidget = new GsmWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME)) {
        m_settingWidget = new IpV4Widget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME)) {
        m_settingWidget = new IpV6Widget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPP_SETTING_NAME)) {
        m_settingWidget = new PppWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        m_settingWidget = new PppoeWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_SERIAL_SETTING_NAME)) {
        m_settingWidget = new PppWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        Knm::VpnSetting * vpnSetting = static_cast<Knm::VpnSetting*>(m_connection->setting(Knm::Setting::Vpn));
        // load the plugin and get its setting widget
        QString error;
        VpnUiPlugin * uiPlugin = KServiceTypeTrader::createInstanceFromQuery<VpnUiPlugin>( QString::fromLatin1( "NetworkManagement/VpnUiPlugin" ), QString::fromLatin1( "[X-NetworkManager-Services]=='%1'" ).arg(vpnSetting->serviceType() ), this, QVariantList(), &error );
        if (uiPlugin && error.isEmpty()) {
            m_settingWidget= uiPlugin->askUser(m_connection, 0);
            buttonCodes = uiPlugin->suggestAuthDialogButtons();
        } else {
            kWarning() << "Loading vpn plugin for" << vpnSetting->serviceType() << "failed: " << error;
        }
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        m_settingWidget = new WiredWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        m_settingWidget = new WirelessSecurityAuthWidget(m_connection, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        m_settingWidget = new Wireless80211Widget(m_connection, 0);
    }

    if (m_settingWidget) {
        m_settingWidget->readConfig();
        m_settingWidget->readSecrets();
        m_askUserDialog = new KDialog(0);
        m_askUserDialog->setCaption(i18nc("@title:window for network secrets request", "Secrets for %1", m_connection->name()));
        m_askUserDialog->setWindowIcon(KIcon("dialog-password"));
        m_askUserDialog->setMainWidget(m_settingWidget);
        m_askUserDialog->setButtons(buttonCodes);
        m_askUserDialog->enableButtonOk(m_settingWidget->isValid());

        connect(m_askUserDialog, SIGNAL(accepted()), SLOT(dialogAccepted()));
        connect(m_askUserDialog, SIGNAL(rejected()), SLOT(dialogRejected()));
        connect(m_settingWidget, SIGNAL(valid(bool)), m_askUserDialog, SLOT(enableButtonOk(bool)));

        m_askUserDialog->show();
        KWindowSystem::forceActiveWindow(m_askUserDialog->winId());
    } else {
        kDebug() << "Setting widget for" << mSettingName << "not yet ported, rejecting secrets request.";
        dialogRejected();
    }
}

void ConnectionSecretsJob::dialogAccepted()
{
    kDebug();
    // m_connection is up to date again
    m_settingWidget->writeConfig();
    setError(EnumError::NoError);
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

QStringList ConnectionSecretsJob::secrets() const
{
    return mSecrets;
}

Knm::Connection * ConnectionSecretsJob::connection() const
{
    return m_connection;
}


bool ConnectionSecretsJob::doKill()
{
    delete m_askUserDialog;
    delete m_settingWidget;
    return true;
}

// vim: sw=4 sts=4 et tw=100
