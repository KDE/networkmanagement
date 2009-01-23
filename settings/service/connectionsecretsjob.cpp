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

#include <KConfigDialog>
#include <KDebug>
#include <KLocale>
#include <KStandardDirs>

#include <kwallet.h>

#include "configxml.h"
#include "datamappings.h"
#include "secretstoragehelper.h"

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
    : m_connection(connection), mSettingName(settingName), mRequestNew(requestNew),
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
    if (mRequestNew) {
        kDebug() << "doAskUser() is under construction";
        doAskUser();
    } else {
        QString configFile = KStandardDirs::locate("data",
                QLatin1String("knetworkmanager/connections/") + m_connection->uuid());
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
    delete m_connectionPersistence;
    setError(result);
    if (result == Knm::ConnectionPersistence::EnumError::NoError) {
        emitResult();
    } else {
        doAskUser();
    }
}

void ConnectionSecretsJob::doAskUser()
{
#if 0
    // popup a dialog showing the appropriate UI for the type of connection
    //
    // all the things the kdialog does for us
    // KDialog
    // Add widget for connection setting type
    // configxml for that
    // kconfigdialogmanager
    // load the settings
    // show
    kDebug();
    if ( mSettingName == QLatin1String(NM_SETTING_802_1X_SETTING_NAME)) {
        //m_settingWidget = Wired8021xSecurityWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        m_settingWidget = new CdmaWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        m_settingWidget = new GsmWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME)) {
        m_settingWidget = new IpV4Widget(mConnectionId, 0);
    //} else if ( mSettingName == QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME)) {
        // not supported yet
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPP_SETTING_NAME)) {
        m_settingWidget = new PppWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        m_settingWidget = new PppoeWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_SERIAL_SETTING_NAME)) {
        m_settingWidget = new PppWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        // not supported yet, figure out the type of the vpn plugin, load it and its m_settingWidgetget
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        m_settingWidget = new WiredWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        m_settingWidget = new Wireless80211SecurityWidget(false, mConnectionId, 0, 0, 0); // TODO: find out
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        m_settingWidget = new Wireless80211Widget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {

    }

    m_askUserDialog = new KConfigDialog(0, QLatin1String("knetworkmanager_askuser"), m_settingWidget->configXml());
    m_askUserDialog->setWindowTitle(i18nc("dialog caption for network secrets request", "Enter network connection secrets"));
    m_askUserDialog->addPage(m_settingWidget, m_settingWidget->windowTitle());
    m_askUserDialog->setFaceType(KPageDialog::Plain);
    connect(m_askUserDialog, SIGNAL(accepted()), SLOT(dialogAccepted()));
    connect(m_askUserDialog, SIGNAL(rejected()), SLOT(dialogRejected()));

    m_askUserDialog->show();
#endif
}

void ConnectionSecretsJob::dialogAccepted()
{
#if 0
    // get results from dialog, put them in mSecrets
    kDebug() << "got secrets from widget:" << m_settingWidget->secrets();
    QMapIterator <QString,QVariant> i(m_settingWidget->secrets());
    while (i.hasNext()) {
        i.next();
        mSecrets.insert(i.key(), i.value());
    }
    kDebug() << "returning merged secrets:" << mSecrets;
    if (mSecrets.isEmpty()) {
        kDebug() << "SECRETS ARE EMPTY";
        setError(UserInputCancelled);
    }

    m_settingWidget->deleteLater();
    m_askUserDialog->deleteLater();
    emitResult();
#endif
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
