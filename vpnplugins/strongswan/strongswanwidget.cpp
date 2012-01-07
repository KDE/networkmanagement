/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Maurus Rohrer <maurus.rohrer@gmail.com>

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

#include "strongswanwidget.h"

#include <nm-setting-vpn.h>

#include "ui_strongswanprop.h"

#include <QString>
#include "nm-strongswan-service.h"

#include "connection.h"
#include "knmserviceprefs.h"

class StrongswanSettingWidgetPrivate
{
public:
    Ui_StrongswanProp ui;
    Knm::VpnSetting * setting;
    enum AuthType {PrivateKey = 0, SshAgent, Smartcard, Eap};
};

StrongswanSettingWidget::StrongswanSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new StrongswanSettingWidgetPrivate)
{
    Q_D(StrongswanSettingWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    connect(d->ui.cboUserPassOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(userPasswordTypeChanged(int)));
    connect(d->ui.cboPrivateKeyPassOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(privateKeyPasswordTypeChanged(int)));
    connect(d->ui.cboPinOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(pinTypeChanged(int)));
    connect(d->ui.cmbMethod, SIGNAL(currentIndexChanged(int)), this, SLOT(methodChanged(int)));
    connect(d->ui.cbShowPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsChanged(bool)));
}

StrongswanSettingWidget::~StrongswanSettingWidget()
{
    delete d_ptr;
}

void StrongswanSettingWidget::userPasswordTypeChanged(int index)
{
    Q_D(StrongswanSettingWidget);
    d->ui.leUserPassword->setEnabled(index == 1);
}

void StrongswanSettingWidget::privateKeyPasswordTypeChanged(int index)
{
    Q_D(StrongswanSettingWidget);
    d->ui.lePrivateKeyPassword->setEnabled(index == 1);
}

void StrongswanSettingWidget::pinTypeChanged(int index)
{
    Q_D(StrongswanSettingWidget);
    d->ui.lePin->setEnabled(index == 1);
}

void StrongswanSettingWidget::methodChanged(int index)
{
    Q_D(StrongswanSettingWidget);
    d->ui.cbShowPasswords->setEnabled(index != StrongswanSettingWidgetPrivate::SshAgent);
}


void StrongswanSettingWidget::readConfig()
{

   kDebug();

    Q_D(StrongswanSettingWidget);
    // General settings
    QStringMap dataMap = d->setting->data();
    // Gateway Address
    QString gateway = dataMap[NM_STRONGSWAN_GATEWAY];
    if (!gateway.isEmpty()) {
        d->ui.leGateway->setText(gateway);
    }
    // Certificate
    d->ui.leGatewayCertificate->setUrl(KUrl(dataMap[NM_STRONGSWAN_CERTIFICATE]) );

    // Authentication
    QString method = dataMap[NM_STRONGSWAN_METHOD];
    if (method == QLatin1String(NM_STRONGSWAN_AUTH_KEY)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::PrivateKey);
        d->ui.leAuthPrivatekeyCertificate->setUrl(KUrl(dataMap[NM_STRONGSWAN_USERCERT]));
        d->ui.leAuthPrivatekeyKey->setUrl(KUrl(dataMap[NM_STRONGSWAN_USERKEY]));
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_AGENT)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::SshAgent);
        d->ui.leAuthSshCertificate->setUrl(KUrl(dataMap[NM_STRONGSWAN_USERCERT]));
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_SMARTCARD)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::Smartcard);
    } else if (method == QLatin1String(NM_STRONGSWAN_AUTH_EAP)) {
        d->ui.cmbMethod->setCurrentIndex(StrongswanSettingWidgetPrivate::Eap);
        d->ui.leUserName->setText(dataMap[NM_STRONGSWAN_USER]);
    }

    // Settings
    d->ui.innerIP->setChecked(dataMap[NM_STRONGSWAN_INNERIP] == "yes");
    d->ui.udpEncap->setChecked(dataMap[NM_STRONGSWAN_ENCAP] == "yes");
    d->ui.ipComp->setChecked(dataMap[NM_STRONGSWAN_IPCOMP] == "yes");

}

void StrongswanSettingWidget::fillOnePasswordCombo(QComboBox * combo, const QString & key, const QStringMap & data, bool hasPassword)
{
    if (data.contains(key)) {
        if (data.value(key) == NM_STRONGSWAN_PW_TYPE_SAVE) {
            combo->setCurrentIndex(1);
        } else if (data.value(key) == NM_STRONGSWAN_PW_TYPE_UNUSED) {
            combo->setCurrentIndex(2);
        }
    } else if (!hasPassword) {
        combo->setCurrentIndex(1);
    }
}

void StrongswanSettingWidget::writeConfig()
{
    Q_D(StrongswanSettingWidget);
    kDebug();
    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_STRONGSWAN));

    QStringMap data;
    QStringMap secretData;

    // General settings
    // Gateway
    if (!d->ui.leGateway->text().isEmpty()) {
        data.insert(NM_STRONGSWAN_GATEWAY, d->ui.leGateway->text());
    }
    data.insert( NM_STRONGSWAN_CERTIFICATE, d->ui.leGatewayCertificate->url().path());

    // Authentication
    switch (d->ui.cmbMethod->currentIndex())
    {
        case StrongswanSettingWidgetPrivate::PrivateKey:
            data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_KEY);
            data.insert(NM_STRONGSWAN_USERCERT, d->ui.leAuthPrivatekeyCertificate->url().path());
            data.insert(NM_STRONGSWAN_USERKEY, d->ui.leAuthPrivatekeyKey->url().path());
            break;
        case StrongswanSettingWidgetPrivate::SshAgent:
            data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_AGENT);
            data.insert(NM_STRONGSWAN_USERCERT, d->ui.leAuthSshCertificate->url().path());
            break;
        case StrongswanSettingWidgetPrivate::Smartcard:
            data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_SMARTCARD);
            break;
        case StrongswanSettingWidgetPrivate::Eap:
            data.insert(NM_STRONGSWAN_METHOD, NM_STRONGSWAN_AUTH_EAP);
            if (!d->ui.leUserName->text().isEmpty())
                data.insert(NM_STRONGSWAN_USER, d->ui.leUserName->text());
            //StrongSwan-nm 1.2 does not appear to be able to save secrets, the must be entered through the auth dialog
            if (!d->ui.leUserPassword->text().isEmpty() && d->ui.cboUserPassOptions->currentIndex() == 1)
                secretData.insert(NM_STRONGSWAN_SECRET, d->ui.leUserPassword->text());
            handleOnePasswordType(d->ui.cboUserPassOptions, NM_STRONGSWAN_SECRET_TYPE, data);
            break;
    }

    //Options
    data.insert(NM_STRONGSWAN_INNERIP, d->ui.innerIP->isChecked() ? "yes" : "no");
    data.insert(NM_STRONGSWAN_ENCAP, d->ui.udpEncap->isChecked() ? "yes" : "no");
    data.insert(NM_STRONGSWAN_IPCOMP, d->ui.ipComp->isChecked() ? "yes" : "no");

   // d->setting->setSecretsAvailable(false);
    d->setting->setData(data);
    d->setting->setVpnSecrets(secretData);
}

uint StrongswanSettingWidget::handleOnePasswordType(const QComboBox * combo, const QString & key, QStringMap & data)
{
    uint type = combo->currentIndex();
    switch (type) {
        case 0:
            data.insert(key, NM_STRONGSWAN_PW_TYPE_ASK);
            break;
        case 1:
            data.insert(key, NM_STRONGSWAN_PW_TYPE_SAVE);
            break;
        case 2:
            data.insert(key, NM_STRONGSWAN_PW_TYPE_UNUSED);
            break;
    }
    return type;
}

void StrongswanSettingWidget::readSecrets()
{
    Q_D(StrongswanSettingWidget);
    QStringMap secrets = d->setting->vpnSecrets();
    if (d->setting->data().value(NM_STRONGSWAN_SECRET_TYPE) == QLatin1String(NM_STRONGSWAN_PW_TYPE_SAVE)) {
        switch (d->ui.cmbMethod->currentIndex())
        {
            case StrongswanSettingWidgetPrivate::PrivateKey:
                d->ui.lePrivateKeyPassword->setText(secrets.value(QLatin1String(NM_STRONGSWAN_SECRET)));
                fillOnePasswordCombo(d->ui.cboPrivateKeyPassOptions, NM_STRONGSWAN_SECRET_TYPE, d->setting->data(), !d->ui.leAuthPrivatekeyKey->url().isEmpty());
                break;
            case StrongswanSettingWidgetPrivate::Smartcard:
                d->ui.lePin->setText(secrets.value(QLatin1String(NM_STRONGSWAN_SECRET)));
                fillOnePasswordCombo(d->ui.cboPinOptions, NM_STRONGSWAN_SECRET_TYPE, d->setting->data(), true);
                break;
            case StrongswanSettingWidgetPrivate::Eap:
                d->ui.leUserPassword->setText(secrets.value(QLatin1String(NM_STRONGSWAN_SECRET)));
                fillOnePasswordCombo(d->ui.cboUserPassOptions, NM_STRONGSWAN_SECRET_TYPE, d->setting->data(), !d->ui.leUserName->text().isEmpty());
                break;
            default:
                break;
        }
    }
}

void StrongswanSettingWidget::validate()
{

}

void StrongswanSettingWidget::showPasswordsChanged(bool show)
{
    Q_D(StrongswanSettingWidget);
    d->ui.lePrivateKeyPassword->setPasswordMode(!show);
    d->ui.leUserPassword->setPasswordMode(!show);
}
// vim: sw=4 sts=4 et tw=100
