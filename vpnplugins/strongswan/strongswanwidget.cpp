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
    uint dpdTimeout;
};

StrongswanSettingWidget::StrongswanSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new StrongswanSettingWidgetPrivate)
{
    Q_D(StrongswanSettingWidget);
    d->dpdTimeout = 0;
    d->ui.setupUi(this);
    d->ui.leCertificate->setMode(KFile::LocalOnly);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));



    connect(d->ui.cboUserPassOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(userPasswordTypeChanged(int)));
    QList<const KUrlRequester *> requesters;
    requesters << d->ui.leCertificate;

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


void StrongswanSettingWidget::readConfig()
{
  
   kDebug();

    Q_D(StrongswanSettingWidget);
    // General settings
    QStringMap dataMap = d->setting->data();
    // Gateway Adress
    QString gateway = dataMap[NM_STRONGSWAN_GATEWAY];
    if (!gateway.isEmpty()) {
        d->ui.leGateway->setText(gateway);
    }
    // Certificate
    d->ui.leCertificate->setUrl(KUrl(dataMap[NM_STRONGSWAN_CERTIFICATE]) );

    //  Username
    QString user = dataMap[NM_STRONGSWAN_USER];
    if (!user.isEmpty()) {
        d->ui.leUserName->setText(user);
    }

    // password storage type is set in readSecrets



    // Method (always EAP, for the moment)
        d->ui.leMethod->setCurrentIndex(0);

    // Settings
    // Virtual IP
    QString innerIP = dataMap[NM_STRONGSWAN_INNERIP];
    if(!innerIP.isEmpty()){
        if(innerIP==QLatin1String("yes")){
            d->ui.innerIP->setChecked(true);
        }
        else{
            d->ui.innerIP->setChecked(false);
        }
    }

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

    // Certificate
    data.insert( NM_STRONGSWAN_CERTIFICATE, d->ui.leCertificate->url().path().toUtf8());

    // Username
    if (!d->ui.leUserName->text().isEmpty()) {
        data.insert(NM_STRONGSWAN_USER, d->ui.leUserName->text());
    }

    // User password
    if (!d->ui.leUserPassword->text().isEmpty() && d->ui.cboUserPassOptions->currentIndex() == 1) {
        secretData.insert(NM_STRONGSWAN_SECRET, d->ui.leUserPassword->text());
    }
    handleOnePasswordType(d->ui.cboUserPassOptions, NM_STRONGSWAN_SECRET_TYPE, data);


    // Method (always EAP, for the moment)
    data.insert(NM_STRONGSWAN_EAP, QLatin1String("yes"));

    //Options
    // Virtual IP
    if(d->ui.innerIP->isChecked()){
        data.insert(NM_STRONGSWAN_INNERIP, QLatin1String("yes"));
    }
    else{
        data.insert(NM_STRONGSWAN_INNERIP, QLatin1String("no"));
    }



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
        d->ui.leUserPassword->setText(secrets.value(QLatin1String(NM_STRONGSWAN_SECRET)));
    }
    fillOnePasswordCombo(d->ui.cboUserPassOptions, NM_STRONGSWAN_SECRET_TYPE, d->setting->data(), !d->ui.leUserName->text().isEmpty());
}

void StrongswanSettingWidget::validate()
{

}
// vim: sw=4 sts=4 et tw=100
