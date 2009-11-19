/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "novellvpnwidget.h"
#include "nm-novellvpn-service.h"

#include <KDebug>
#include <KProcess>
#include <nm-setting-vpn.h>
#include "settings/vpn.h"
#include "connection.h"

#include "ui_novellvpnprop.h"

class NovellVpnSettingWidget::Private
{
public:
    Ui_NovellVpnWidget ui;
    Knm::VpnSetting * setting;
};


NovellVpnSettingWidget::NovellVpnSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d(new Private)
{
    d->ui.setupUi(this);
    d->ui.x509Cert->setMode(KFile::LocalOnly);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));

    connect(d->ui.cbShowPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsChanged(bool)));

    connect(d->ui.cmbGwType, SIGNAL(currentIndexChanged(int)), this, SLOT(gatewayTypeChanged(int)));
}

NovellVpnSettingWidget::~NovellVpnSettingWidget()
{
    delete d;
}

void NovellVpnSettingWidget::gatewayTypeChanged(int gatewayType)
{
    if ( gatewayType == 1) {
        d->ui.cmbAuthType->setCurrentIndex(1);
    }
    d->ui.cmbAuthType->setEnabled(gatewayType != 1);
}

void NovellVpnSettingWidget::showPasswordsChanged(bool show)
{
    d->ui.x509CertPass->setPasswordMode(!show);
    d->ui.leUserPass->setPasswordMode(!show);
    d->ui.leGroupPass->setPasswordMode(!show);
}

void NovellVpnSettingWidget::readConfig()
{
    kDebug();

    // General settings
    QStringMap dataMap = d->setting->data();

    d->ui.leGateway->setText( dataMap[NM_NOVELLVPN_KEY_GATEWAY] );

    QString gwType = dataMap[NM_NOVELLVPN_KEY_GWTYPE];

    if (gwType == QLatin1String(NM_NOVELLVPN_GWTYPE_NORTEL_STRING)) {
        d->ui.cmbGwType->setCurrentIndex(0);
        QString authType = dataMap[NM_NOVELLVPN_KEY_AUTHTYPE];
        if (authType == QLatin1String(NM_NOVELLVPN_CONTYPE_GROUPAUTH_STRING)) {
            d->ui.leUserName->setText( d->setting->userName() );
            d->ui.leGroupName->setText( d->setting->userName() );
        } else if (authType == QLatin1String(NM_NOVELLVPN_CONTYPE_X509_STRING)) {
            readX509Auth(dataMap);
        }
    } else if (gwType == QLatin1String(NM_NOVELLVPN_GWTYPE_STDGW_STRING)) {
            readX509Auth(dataMap);
    }

#if 0
    // Optional Settings
    if (dataMap.contains(NM_NOVELLVPN_KEY_PORT)) {
        d->ui.sbCustomPort->setValue(dataMap[NM_NOVELLVPN_KEY_PORT].toUInt());
    } else {
        d->ui.sbCustomPort->setValue(0);
    }
    d->ui.chkUseLZO->setChecked( dataMap[NM_NOVELLVPN_KEY_COMP_LZO] == "yes" );
    d->ui.chkUseTCP->setChecked( dataMap[NM_NOVELLVPN_KEY_PROTO_TCP] == "yes" );
    d->ui.chkUseTAP->setChecked( dataMap[NM_NOVELLVPN_KEY_TAP_DEV] == "yes" );
    // Optional Security Settings
    QString hmacKeyAuth = dataMap[NM_NOVELLVPN_KEY_AUTH];
    if (hmacKeyAuth == QLatin1String(NM_NOVELLVPN_AUTH_NONE)) {
        d->ui.cboHmac->setCurrentIndex(1);
    } else if (hmacKeyAuth == QLatin1String(NM_NOVELLVPN_AUTH_MD5)) {
        d->ui.cboHmac->setCurrentIndex(2);
    } else if (hmacKeyAuth == QLatin1String(NM_NOVELLVPN_AUTH_SHA1)) {
        d->ui.cboHmac->setCurrentIndex(3);
    }
    // ciphers populated above?
    if (d->gotNovellVpnCiphers && dataMap.contains(NM_NOVELLVPN_KEY_CIPHER)) {
        d->ui.cboCipher->setCurrentIndex(d->ui.cboCipher->findText(dataMap[NM_NOVELLVPN_KEY_CIPHER]));
    }

    // Optional TLS
    d->ui.useExtraTlsAuth->setChecked(!dataMap[NM_NOVELLVPN_KEY_TA].isEmpty());
    d->ui.kurlTlsAuthKey->setUrl(KUrl(dataMap[NM_NOVELLVPN_KEY_TA]) );
    if (dataMap.contains(NM_NOVELLVPN_KEY_TA_DIR)) {
        uint tlsAuthDirection = dataMap[NM_NOVELLVPN_KEY_TA_DIR].toUInt();
        d->ui.cboDirection->setCurrentIndex(tlsAuthDirection + 1);
    }
#endif
}

void NovellVpnSettingWidget::readX509Auth(const QStringMap & dataMap)
{
    d->ui.cmbGwType->setCurrentIndex(1);
    d->ui.x509Cert->setUrl(KUrl(dataMap[NM_NOVELLVPN_KEY_CERTIFICATE]));
    //d->ui.x509CertPass->setText(dataMap[NM_NOVELLVPN_KEY_KEY]);
}

void NovellVpnSettingWidget::writeConfig()
{
    kDebug();

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_NOVELLVPN));

    QStringMap data;
    QVariantMap secretData;

    // required settings
    data.insert(NM_NOVELLVPN_KEY_GATEWAY, d->ui.leGateway->text());

    if (d->ui.cmbGwType->currentIndex() == 0) {
        data.insert(QLatin1String(NM_NOVELLVPN_KEY_GWTYPE), QLatin1String(NM_NOVELLVPN_CONTYPE_GROUPAUTH_STRING));

        if (d->ui.cmbAuthType->currentIndex() == 0) {
            data.insert(QLatin1String(NM_NOVELLVPN_KEY_USER_NAME), d->ui.leUserName->text());
            data.insert(QLatin1String(NM_NOVELLVPN_KEY_GROUP_NAME), d->ui.leGroupName->text());
            secretData.insert(QLatin1String(NM_NOVELLVPN_KEY_USER_PWD), d->ui.leUserPass->text());
            secretData.insert(QLatin1String(NM_NOVELLVPN_KEY_GRP_PWD), d->ui.leGroupPass->text());
        } else if (d->ui.cmbAuthType->currentIndex() == 1 ) {
            writeX509Auth(data, secretData);
        }
    } else if (d->ui.cmbGwType->currentIndex() == 1) {
        data.insert(QLatin1String(NM_NOVELLVPN_KEY_GWTYPE), QLatin1String(NM_NOVELLVPN_CONTYPE_X509_STRING));
        writeX509Auth(data, secretData);
    }

    // advanced dialog
}

void NovellVpnSettingWidget::writeX509Auth(QStringMap & data, QVariantMap & secretData)
{

}

void NovellVpnSettingWidget::readSecrets()
{
    QVariantMap secrets = d->setting->vpnSecrets();
    d->ui.x509CertPass->setText(secrets.value(QLatin1String(NM_NOVELLVPN_KEY_CERT_PWD)).toString());
    d->ui.leUserPass->setText(secrets.value(QLatin1String(NM_NOVELLVPN_KEY_USER_PWD)).toString());
    d->ui.leGroupPass->setText(secrets.value(QLatin1String(NM_NOVELLVPN_KEY_GRP_PWD)).toString());
}

void NovellVpnSettingWidget::validate()
{

}

// vim: sw=4 sts=4 et tw=100
