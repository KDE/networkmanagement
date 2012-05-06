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

#include "settingwidget_p.h"

#include <KDebug>
#include <KLocale>

#include <nm-setting-vpn.h>
#include "settings/vpn.h"
#include "connection.h"

#include "ui_novellvpnprop.h"
#include "ui_novellvpnadvancedprop.h"

class NovellVpnSettingWidget::Private
{
public:
    Ui_NovellVpnWidget ui;
    Knm::VpnSetting * setting;
    Ui_NovellVpnAdvancedWidget advUi;
    KDialog * advancedDialog;
};


NovellVpnSettingWidget::NovellVpnSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d(new Private)
{
    setValid(false);
    d->ui.setupUi(this);
    d->ui.x509Cert->setMode(KFile::LocalOnly);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));

    connect(d->ui.leGateway, SIGNAL(textChanged(QString)), this, SLOT(validate()));
    connect(d->ui.cbShowPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsChanged(bool)));

    connect(d->ui.cmbGwType, SIGNAL(currentIndexChanged(int)), this, SLOT(gatewayTypeChanged(int)));

    connect(d->ui.btnAdvanced, SIGNAL(clicked()), this, SLOT(advancedClicked()));

    d->advancedDialog = new KDialog(this);
    d->advancedDialog->setButtons(KDialog::Ok);
    d->advancedDialog->setCaption(i18nc("@window:title NovellVPN advanced connection options", "NovellVPN advanced options"));
    QWidget * advWid = new QWidget(d->advancedDialog);
    d->advUi.setupUi(advWid);
    d->advancedDialog->setMainWidget(advWid);
}

NovellVpnSettingWidget::~NovellVpnSettingWidget()
{
    delete d;
}

void NovellVpnSettingWidget::advancedClicked()
{
    d->advancedDialog->exec();
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
            d->ui.leUserName->setText(dataMap[NM_NOVELLVPN_KEY_USER_NAME]);
            d->ui.leGroupName->setText(dataMap[NM_NOVELLVPN_KEY_GROUP_NAME]);
        } else if (authType == QLatin1String(NM_NOVELLVPN_CONTYPE_X509_STRING)) {
            readX509Auth(dataMap);
        }
    } else if (gwType == QLatin1String(NM_NOVELLVPN_GWTYPE_STDGW_STRING)) {
            readX509Auth(dataMap);
    }

    // advanced
    QString dhGroup = dataMap[QLatin1String(NM_NOVELLVPN_KEY_DHGROUP)];
    if (dhGroup.toUInt() == 1) {
        d->advUi.rbDh2->setChecked(true);
    }

    QString pfsGroup = dataMap[QLatin1String(NM_NOVELLVPN_KEY_PFSGROUP)];
    if (pfsGroup.toUInt() == PFSGROUP_PFS1) {
        d->advUi.rbPf1->setChecked(true);
    } else if (pfsGroup.toUInt() == PFSGROUP_PFS2) {
        d->advUi.rbPf2->setChecked(true);
    }

    if (dataMap[QLatin1String(NM_NOVELLVPN_KEY_NOSPLITTUNNEL)] == QLatin1String("yes")) {
        d->advUi.cbDisableSplit->setChecked(true);
    }
}

void NovellVpnSettingWidget::readX509Auth(const QStringMap & dataMap)
{
    d->ui.cmbGwType->setCurrentIndex(1);
    d->ui.x509Cert->setUrl(KUrl(dataMap[NM_NOVELLVPN_KEY_CERTIFICATE]));
}

void NovellVpnSettingWidget::writeConfig()
{
    kDebug();

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_NOVELLVPN));

    QStringMap data;
    QStringMap secretData;

    // required settings
    data.insert(NM_NOVELLVPN_KEY_GATEWAY, d->ui.leGateway->text());

    if (d->ui.cmbGwType->currentIndex() == 0) {
        data.insert(QLatin1String(NM_NOVELLVPN_KEY_GWTYPE), QLatin1String(NM_NOVELLVPN_GWTYPE_NORTEL_STRING));

        if (d->ui.cmbAuthType->currentIndex() == 0) {
            data.insert(QLatin1String(NM_NOVELLVPN_KEY_AUTHTYPE), QLatin1String(NM_NOVELLVPN_CONTYPE_GROUPAUTH_STRING));

            data.insert(QLatin1String(NM_NOVELLVPN_KEY_USER_NAME), d->ui.leUserName->text());
            data.insert(QLatin1String(NM_NOVELLVPN_KEY_GROUP_NAME), d->ui.leGroupName->text());
            secretData.insert(QLatin1String(NM_NOVELLVPN_KEY_USER_PWD), d->ui.leUserPass->text());
            secretData.insert(QLatin1String(NM_NOVELLVPN_KEY_GRP_PWD), d->ui.leGroupPass->text());
        } else {
            writeX509Auth(data, secretData);
        }
    } else {
        data.insert(QLatin1String(NM_NOVELLVPN_KEY_GWTYPE), QLatin1String(NM_NOVELLVPN_GWTYPE_STDGW_STRING));
        writeX509Auth(data, secretData);
    }

    // advanced dialog
    data.insert(QLatin1String(NM_NOVELLVPN_KEY_DHGROUP),
            (d->advUi.rbDh1->isChecked()
             ? QString::number(DHGROUP_DH1)
             : QString::number(DHGROUP_DH2)));

    if (d->advUi.rbPfOff->isChecked()) {
        data.insert(QLatin1String(NM_NOVELLVPN_KEY_PFSGROUP), QString::number(PFSGROUP_OFF));
    } else if (d->advUi.rbPf1->isChecked()) {
        data.insert(QLatin1String(NM_NOVELLVPN_KEY_PFSGROUP), QString::number(PFSGROUP_PFS1));
    } else {
        data.insert(QLatin1String(NM_NOVELLVPN_KEY_PFSGROUP), QString::number(PFSGROUP_PFS2));
    }

    data.insert(QLatin1String(NM_NOVELLVPN_KEY_NOSPLITTUNNEL), d->advUi.cbDisableSplit->isChecked() ? QLatin1String("yes") : QLatin1String("no"));

    d->setting->setData(data);
    d->setting->setVpnSecrets(secretData);
}

void NovellVpnSettingWidget::writeX509Auth(QStringMap & data, QStringMap & secretData)
{
    data.insert(QLatin1String(NM_NOVELLVPN_KEY_AUTHTYPE), QLatin1String(NM_NOVELLVPN_CONTYPE_X509_STRING));
    data.insert(NM_NOVELLVPN_KEY_CERTIFICATE, d->ui.x509Cert->url().path());
    secretData.insert(QLatin1String(NM_NOVELLVPN_KEY_CERT_PWD), d->ui.x509CertPass->text());
}

void NovellVpnSettingWidget::readSecrets()
{
    QStringMap secrets = d->setting->vpnSecrets();
    kDebug() << "Value of" << NM_NOVELLVPN_KEY_CERT_PWD << secrets.value(QLatin1String(NM_NOVELLVPN_KEY_CERT_PWD));
    d->ui.x509CertPass->setText(secrets.value(QLatin1String(NM_NOVELLVPN_KEY_CERT_PWD)));
    d->ui.leUserPass->setText(secrets.value(QLatin1String(NM_NOVELLVPN_KEY_USER_PWD)));
    d->ui.leGroupPass->setText(secrets.value(QLatin1String(NM_NOVELLVPN_KEY_GRP_PWD)));
}

void NovellVpnSettingWidget::validate()
{
    setValid(!d->ui.leGateway->text().isEmpty());
    emit valid(isValid());
}

// vim: sw=4 sts=4 et tw=100
