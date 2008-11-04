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

#include <KConfigDialog>
#include <KDebug>

#include <kwallet.h>

#include "configxml.h"
#include "datamappings.h"

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

ConnectionSecretsJob::ConnectionSecretsJob(const QString & connectionId, const QString &settingName, const QStringList& secrets, bool requestNew,  QDBusMessage& reply)
    : mConnectionId(connectionId), mSettingName(settingName), mRequestNew(requestNew), mReply(reply), m_askUserDialog(0), m_settingWidget(0)
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
        // do wallet lookup
        if (KWallet::Wallet::isEnabled()) {
            kDebug() << "opening wallet...";
            KWallet::Wallet * wallet = KWallet::Wallet::openWallet(KWallet::Wallet::LocalWallet(), 0/*WId*/, KWallet::Wallet::Asynchronous);
            if (wallet) {
                connect(wallet, SIGNAL(walletOpened(bool)), this, SLOT(walletOpenedForRead(bool)));
            } else {
                setError(WalletNotFound);
                emitResult();
            }
        } else {
            setError(WalletDisabled);
            emitResult();
        }
    }
}

void ConnectionSecretsJob::doAskUser()
{
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
    SettingWidget * wid = 0;
    if ( mSettingName == QLatin1String(NM_SETTING_802_1X_SETTING_NAME)) {
        //wid = new 
        
    } else if ( mSettingName == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        wid = new CdmaWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        wid = new GsmWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME)) {
        wid = new IpV4Widget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME)) {
        // not supported yet
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPP_SETTING_NAME)) {
        wid = new PppWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        wid = new PppoeWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_SERIAL_SETTING_NAME)) {
        wid = new PppWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        // not supported yet, figure out the type of the vpn plugin, load it and its widget
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        wid = new WiredWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        wid = new Wireless80211SecurityWidget(mConnectionId, 0);
    } else if ( mSettingName == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        wid = new Wireless80211Widget(mConnectionId, 0);
    }

    m_askUserDialog = new KConfigDialog(0, QLatin1String("knetworkmanager_askuser"), wid->configXml());
    m_askUserDialog->addPage(wid, wid->windowTitle());
    m_askUserDialog->setFaceType(KPageDialog::Plain);
    connect(m_askUserDialog, SIGNAL(done(int)), SLOT(dialogDone(int)));

    m_askUserDialog->show();
}

void ConnectionSecretsJob::walletOpenedForRead(bool success)
{
    if (success) {
        // get the requested secrets, set our secrets, and emit result
        KWallet::Wallet * wallet = static_cast<KWallet::Wallet*>(sender());
        if (wallet->isOpen() && wallet->hasFolder("NetworkManager") && wallet->setFolder("NetworkManager")) {
            if (mSecrets.isEmpty()) {
                kDebug() << "Reading all entries for connection";
                QMap<QString,QString> entries;
                QString key = mConnectionId + ';' + mSettingName + ";*";
                if (wallet->readPasswordList(key, entries) == 0) {
                    kDebug() << "Got password list: " << entries;
                    DataMappings dm;
                    QMapIterator<QString,QString> i(entries);
                    while (i.hasNext()) {
                        i.next();
                        // the part that NM has asked for is the final part of the key used in
                        // kwallet
                        QString key = i.key().section(';', 2, 2);
                        mSecrets.insert(dm.convertKey(key), dm.convertValue(key, QString(i.value())));
                    }
                    emitResult();
                } else {
                    kDebug() << "Wallet::readEntryList for :" << key << " failed";
                }
            } else {
                kDebug() << "Reading requested entries from wallet: "<< mSecrets.keys();
                bool missingSecret = false;
                foreach (QString key, mSecrets.keys()) {
                    kDebug() << "Requesting password from wallet: " << key;
                    QString secret;
                    if (wallet->readPassword(keyForEntry(key), secret) == 0 ) {
                        kDebug() << "Got: " << key << " : " << secret;
                        mSecrets.insert(key, secret);
                    } else {
                        missingSecret = true;
                    }
                }
                if (missingSecret) {
                    doAskUser();
                    emitResult();
                } else {
                    emitResult();
                }
            }
        }
    } else {
        setError(WalletOpenRefused);
        emitResult();
    }
}

void ConnectionSecretsJob::walletOpenedForWrite(bool success)
{

}

void ConnectionSecretsJob::dialogDone(int result)
{
    kDebug() << result;
    if (result == QDialog::Accepted) {
        // get results from dialog, put them in mSecrets
        kDebug() << "got secrets:" << m_settingWidget->secrets();
        QMapIterator <QString,QVariant> i(m_settingWidget->secrets());
        while (i.hasNext()) {
            i.next();
            mSecrets.insert(i.key(), i.value());
        }
    } else {
        setError(UserInputCancelled);
    }
    delete m_settingWidget;
    delete m_askUserDialog;
}

QString ConnectionSecretsJob::settingName() const
{
    return mSettingName;
}

QVariantMap ConnectionSecretsJob::secrets() const
{
    return mSecrets;
}

QDBusMessage ConnectionSecretsJob::message() const
{
    return mReply;
}

QString ConnectionSecretsJob::keyForEntry(const QString & entry) const
{
    return mConnectionId + ';' + mSettingName + ';' + entry;
}


#include "connectionsecretsjob.moc"
// vim: sw=4 sts=4 et tw=100
