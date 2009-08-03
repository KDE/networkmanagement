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

#include "openvpnwidget.h"
#include "nm-openvpn-service.h"

#include <KDebug>
#include <KProcess>
#include <nm-setting-vpn.h>
#include "settings/vpn.h"
#include "connection.h"

class OpenVpnSettingWidget::Private
{
public:
    Ui_OpenVPNProp ui;
    Knm::VpnSetting * setting;
    KProcess * openvpnProcess;
    QByteArray openVpnCiphers;
    bool gotOpenVpnCiphers;
    bool readConfig;
};


OpenVpnSettingWidget::OpenVpnSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d(new Private)
{
    d->ui.setupUi(this);
    d->ui.x509CaFile->setMode(KFile::LocalOnly);
    d->ui.x509Cert->setMode(KFile::LocalOnly);
    d->ui.x509Key->setMode(KFile::LocalOnly);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    d->openvpnProcess = 0;
    d->gotOpenVpnCiphers = false;
    d->readConfig = false;

    // start openVPN process and get its cipher list
    QString openVpnBinary = "/usr/sbin/openvpn";
    QStringList args(QLatin1String("--show-ciphers"));
    d->openvpnProcess = new KProcess(this);
    d->openvpnProcess->setOutputChannelMode(KProcess::OnlyStdoutChannel);
    d->openvpnProcess->setReadChannel(QProcess::StandardOutput);
    connect(d->openvpnProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(openVpnError(QProcess::ProcessError)));
    connect(d->openvpnProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(gotOpenVpnOutput()));
    connect(d->openvpnProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(openVpnFinished(int,QProcess::ExitStatus)));

    d->openvpnProcess->setProgram(openVpnBinary, args);
}

OpenVpnSettingWidget::~OpenVpnSettingWidget()
{
    delete d;
}


void OpenVpnSettingWidget::init()
{
    d->openvpnProcess->start();
}

void OpenVpnSettingWidget::gotOpenVpnOutput()
{
    d->openVpnCiphers.append(d->openvpnProcess->readAll());
}

void OpenVpnSettingWidget::openVpnError(QProcess::ProcessError)
{
    d->ui.cboCipher->removeItem(0);
    d->ui.cboCipher->addItem(i18nc("@item:inlist Item added when openvpn cipher lookup failed", "Openvpn cipher lookup failed"));
}

void OpenVpnSettingWidget::openVpnFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    d->ui.cboCipher->removeItem(0);
    if (!exitCode && exitStatus == QProcess::NormalExit) {
        d->ui.cboCipher->addItem(i18nc("@item::inlist Default openvpn cipher item", "Default"));
        QList<QByteArray> rawOutputLines = d->openVpnCiphers.split('\n');
        QStringList ciphers;
        bool foundFirstSpace = false;;
        foreach (QByteArray cipher, rawOutputLines) {
            if (cipher.length() == 0) {
                foundFirstSpace = true;
            } else if (foundFirstSpace) {
                d->ui.cboCipher->addItem(QString::fromLocal8Bit(cipher.left(cipher.indexOf(' '))));
            }
        }

        if (d->ui.cboCipher->count()) {
            d->ui.cboCipher->setEnabled(true);
        } else {
            d->ui.cboCipher->addItem(i18nc("@item:inlist Item added when openvpn cipher lookup failed", "No openvpn ciphers found"));
        }
    } else {
        d->ui.cboCipher->addItem(i18nc("@item:inlist Item added when openvpn cipher lookup failed", "openvpn cipher lookup failed"));
    }
    delete d->openvpnProcess;
    d->openvpnProcess = 0;
    d->openVpnCiphers = QByteArray();
    d->gotOpenVpnCiphers = true;

    if (d->readConfig) {
        QStringMap dataMap = d->setting->data();
        if (dataMap.contains(NM_OPENVPN_KEY_CIPHER)) {
            d->ui.cboCipher->setCurrentIndex(d->ui.cboCipher->findText(dataMap[NM_OPENVPN_KEY_CIPHER]));
        }
    }
}

void OpenVpnSettingWidget::readConfig()
{
    kDebug();

    // General settings
    QStringMap dataMap = d->setting->data();

    QString cType = dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE];

    if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_PASSWORD_TLS ) ) {
        d->ui.cmbConnectionType->setCurrentIndex( 3 );
        d->ui.x509PassUsername->setText( d->setting->userName() );
        d->ui.x509PassCaFile->setUrl( KUrl(dataMap[NM_OPENVPN_KEY_CA]) );
        d->ui.x509PassCert->setUrl(KUrl( dataMap[NM_OPENVPN_KEY_CERT] ));
        d->ui.x509PassKey->setUrl(KUrl( dataMap[NM_OPENVPN_KEY_KEY] ));
    } else if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_STATIC_KEY ) ) {
        d->ui.cmbConnectionType->setCurrentIndex( 1 );
        d->ui.pskSharedKey->setText( dataMap[NM_OPENVPN_KEY_STATIC_KEY]);
        d->ui.pskRemoteIp->setText( dataMap[NM_OPENVPN_KEY_REMOTE_IP]);
        d->ui.pskLocalIp->setText( dataMap[NM_OPENVPN_KEY_LOCAL_IP]);
    } else if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_PASSWORD ) ) {
        d->ui.passUserName->setText( dataMap[NM_OPENVPN_KEY_USERNAME] );
        d->ui.passCaFile->setUrl(KUrl( dataMap[NM_OPENVPN_KEY_CA] ));
        d->ui.cmbConnectionType->setCurrentIndex( 2 );
    } else { // default
        d->ui.cmbConnectionType->setCurrentIndex( 0 );
        d->ui.x509CaFile->setUrl(KUrl( dataMap[NM_OPENVPN_KEY_CA] ));
        d->ui.x509Cert->setUrl(KUrl( dataMap[NM_OPENVPN_KEY_CERT] ));
        d->ui.x509Key->setUrl(KUrl( dataMap[NM_OPENVPN_KEY_KEY] ));
    }

    d->ui.gateway->setText( dataMap[NM_OPENVPN_KEY_REMOTE] );

    // Optional Settings
    d->ui.chkCustomPort->setChecked(d->ui.sbCustomPort->value() !=0);
    d->ui.sbCustomPort->setValue(dataMap[NM_OPENVPN_KEY_PORT].toUInt());
    d->ui.chkUseLZO->setChecked( dataMap[NM_OPENVPN_KEY_COMP_LZO] == "yes" );
    d->ui.chkUseTCP->setChecked( dataMap[NM_OPENVPN_KEY_PROTO_TCP] == "yes" );
    d->ui.chkUseTAP->setChecked( dataMap[NM_OPENVPN_KEY_TAP_DEV] == "yes" );
    // Optional Security Settings
    QString hmacKeyAuth = dataMap[NM_OPENVPN_KEY_AUTH];
    if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_NONE)) {
        d->ui.cboHmac->setCurrentIndex(1);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_MD5)) {
        d->ui.cboHmac->setCurrentIndex(2);
    } else if (hmacKeyAuth == QLatin1String(NM_OPENVPN_AUTH_MD5)) {
        d->ui.cboHmac->setCurrentIndex(3);
    }
    // ciphers populated above?
    if (d->gotOpenVpnCiphers && dataMap.contains(NM_OPENVPN_KEY_CIPHER)) {
        d->ui.cboCipher->setCurrentIndex(d->ui.cboCipher->findText(dataMap[NM_OPENVPN_KEY_CIPHER]));
    }

    // Optional TLS
    d->ui.useExtraTlsAuth->setChecked(!dataMap[NM_OPENVPN_KEY_TA].isEmpty());
    d->ui.kurlTlsAuthKey->setUrl(KUrl(dataMap[NM_OPENVPN_KEY_TA]) );
    if (dataMap.contains(NM_OPENVPN_KEY_TA_DIR)) {
        uint tlsAuthDirection = dataMap[NM_OPENVPN_KEY_TA_DIR].toUInt();
        d->ui.cboDirection->setCurrentIndex(tlsAuthDirection + 1);
    }
    d->readConfig = true;
}

void OpenVpnSettingWidget::writeConfig()
{
    kDebug();

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_OPENVPN));

    QStringMap data;
    QVariantMap secretData;

    // required settings
    data.insert(NM_OPENVPN_KEY_REMOTE, d->ui.gateway->text());

    const char *contype = NM_OPENVPN_CONTYPE_TLS;

    switch ( d->ui.cmbConnectionType->currentIndex())
    {
    case 0:
        contype = NM_OPENVPN_CONTYPE_TLS;
        kDebug() << "saving VPN TLS settings as urls:" << d->ui.x509CaFile->url().url() << d->ui.x509Cert->url().url() << d->ui.x509Key->url().url();
        data.insert( NM_OPENVPN_KEY_CA, d->ui.x509CaFile->url().url().toUtf8());
        data.insert( NM_OPENVPN_KEY_CERT, d->ui.x509Cert->url().url().toUtf8());
        data.insert( NM_OPENVPN_KEY_KEY, d->ui.x509Key->url().url().toUtf8());
        break;
    case 1:
        contype = NM_OPENVPN_CONTYPE_STATIC_KEY;
        data.insert( NM_OPENVPN_KEY_STATIC_KEY, d->ui.pskSharedKey->url().url().toUtf8());
        // ip addresses
        data.insert( NM_OPENVPN_KEY_REMOTE_IP, d->ui.pskRemoteIp->text());
        data.insert( NM_OPENVPN_KEY_LOCAL_IP, d->ui.pskLocalIp->text());
        break;
    case 2:
        contype = NM_OPENVPN_CONTYPE_PASSWORD;
        // username
        data.insert( NM_OPENVPN_KEY_USERNAME, d->ui.passUserName->text());
        d->setting->setUserName(d->ui.passUserName->text());
        // password
        secretData.insert(QLatin1String( NM_OPENVPN_KEY_PASSWORD ), d->ui.passPassword->text());
        // ca
        data.insert(NM_OPENVPN_KEY_CA, d->ui.passCaFile->url().url().toUtf8());
        break;
    case 3:
        contype = NM_OPENVPN_CONTYPE_PASSWORD_TLS;
        // username
        data.insert(NM_OPENVPN_KEY_USERNAME, d->ui.x509PassUsername->text());
        d->setting->setUserName(d->ui.x509PassUsername->text());
        // ca
        data.insert(NM_OPENVPN_KEY_CA, d->ui.x509PassCaFile->url().url().toUtf8());
        // cert
        data.insert(NM_OPENVPN_KEY_CERT, d->ui.x509PassCert->url().url().toUtf8());
        // key file
        data.insert(NM_OPENVPN_KEY_KEY, d->ui.x509PassKey->url().url().toUtf8());
        // password
        secretData.insert(NM_OPENVPN_KEY_PASSWORD, d->ui.x509PassPassword->text());
        break;
    }
    data.insert( NM_OPENVPN_KEY_CONNECTION_TYPE, contype);

    // optional settings
    if ( !d->ui.chkCustomPort->isChecked() )
    {
        data.insert(NM_OPENVPN_KEY_PORT, QString::number(d->ui.sbCustomPort->value()));
    }
    data.insert( NM_OPENVPN_KEY_PROTO_TCP, d->ui.chkUseTCP->isChecked() ? "yes" : "no" );
    data.insert( NM_OPENVPN_KEY_COMP_LZO, d->ui.chkUseLZO->isChecked() ? "yes" : "no" );
    data.insert( NM_OPENVPN_KEY_TAP_DEV, d->ui.chkUseTAP->isChecked() ? "yes" : "no" );

    // Optional Security
    switch ( d->ui.cboHmac->currentIndex()) {
        case 0:
            break;
        case 1:
            data.insert(NM_OPENVPN_KEY_AUTH, NM_OPENVPN_AUTH_NONE);
            break;
        case 2:
            data.insert(NM_OPENVPN_KEY_AUTH, NM_OPENVPN_AUTH_MD5);
            break;
        case 3:
            data.insert(NM_OPENVPN_KEY_AUTH, NM_OPENVPN_AUTH_SHA1);
            break;
    }
    if (d->ui.cboCipher->currentIndex() != 0) {
        data.insert(NM_OPENVPN_KEY_CIPHER, d->ui.cboCipher->currentText());
    }

    // optional tls authentication
    if (d->ui.useExtraTlsAuth->isChecked()) {
        if (d->ui.cboDirection->currentIndex() > 0) {
            data.insert(NM_OPENVPN_KEY_TA_DIR, QString::number(d->ui.cboDirection->currentIndex()));
        }
    }

    d->setting->setData(data);
    d->setting->setVpnSecrets(secretData);
}

void OpenVpnSettingWidget::readSecrets()
{
    QVariantMap secrets = d->setting->vpnSecrets();
    d->ui.x509PassPassword->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_PASSWORD)).toString());
    d->ui.passPassword->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_PASSWORD)).toString());
}

// vim: sw=4 sts=4 et tw=100
