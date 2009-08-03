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

#include "vpncwidget.h"

#include <nm-setting-vpn.h>

#include <QString>
#include "nm-vpnc-service.h"

#include "connection.h"

class VpncSettingWidgetPrivate
{
public:
    Ui_VpncProp ui;
    Knm::VpnSetting * setting;
    uint dpdTimeout;
};

VpncSettingWidget::VpncSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new VpncSettingWidgetPrivate)
{
    Q_D(VpncSettingWidget);
    d->dpdTimeout = 0;
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    connect(d->ui.cboUserPassOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(userPasswordTypeChanged(int)));
    connect(d->ui.cboGroupPassOptions, SIGNAL(currentIndexChanged(int)), this, SLOT(groupPasswordTypeChanged(int)));
}

VpncSettingWidget::~VpncSettingWidget()
{

}

void VpncSettingWidget::userPasswordTypeChanged(int index)
{
    Q_D(VpncSettingWidget);
    d->ui.leUserPassword->setEnabled(index == 1);
}

void VpncSettingWidget::groupPasswordTypeChanged(int index)
{
    Q_D(VpncSettingWidget);
    d->ui.leGroupPassword->setEnabled(index == 1);
}

void VpncSettingWidget::readConfig()
{
    Q_D(VpncSettingWidget);
    // General settings
    QStringMap dataMap = d->setting->data();
    //   gateway
    QString gateway = dataMap[NM_VPNC_KEY_GATEWAY];
    if (!gateway.isEmpty()) {
        d->ui.leGateway->setText(gateway);
    }
    //   group name
    QString group = dataMap[NM_VPNC_KEY_ID];
    if (!group.isEmpty()) {
        d->ui.leGroupName->setText(group);
    }
    // password storage type is set in readSecrets

    // Optional settings
    //   username
    QString user = dataMap[NM_VPNC_KEY_XAUTH_USER];
    if (!user.isEmpty()) {
        d->ui.leUserName->setText(user);
    }

    //   domain
    QString domain = dataMap[NM_VPNC_KEY_DOMAIN];
    if (!domain.isEmpty()) {
        d->ui.leDomain->setText(domain);
    }

    //   encryption
    if (dataMap[NM_VPNC_KEY_SINGLE_DES] == QLatin1String("yes")) {
        d->ui.cboEncryptionMethod->setCurrentIndex(1);
    } else if (dataMap[NM_VPNC_KEY_NO_ENCRYPTION] == QLatin1String("yes")) {
        d->ui.cboEncryptionMethod->setCurrentIndex(2);
    }

    //   nat traversal
    if (dataMap[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_NATT) {
        d->ui.cboNatTraversal->setCurrentIndex(1);
    } else if (dataMap[NM_VPNC_KEY_NAT_TRAVERSAL_MODE] == NM_VPNC_NATT_MODE_NONE) {
        d->ui.cboNatTraversal->setCurrentIndex(2);
    }
    //   dead peer detection
    if (dataMap.contains(NM_VPNC_KEY_DPD_IDLE_TIMEOUT)) {
        uint dpdTimeout = dataMap.value(NM_VPNC_KEY_DPD_IDLE_TIMEOUT).toUInt();
        if (dpdTimeout == 0) {
            d->ui.chkDeadPeerDetection->setChecked(false);
            d->dpdTimeout = dpdTimeout;
        }
    }
}

void VpncSettingWidget::fillOnePasswordCombo(QComboBox * combo, const QString & key, const QStringMap & data, bool hasPassword)
{
    if (data.contains(key)) {
        if (data.value(key) == NM_VPNC_PW_TYPE_SAVE) {
            combo->setCurrentIndex(1);
        } else if (data.value(key) == NM_VPNC_PW_TYPE_UNUSED) {
            combo->setCurrentIndex(2);
        }
    } else if (!hasPassword) {
        combo->setCurrentIndex(1);
    }
}

void VpncSettingWidget::writeConfig()
{
    Q_D(VpncSettingWidget);
    kDebug();

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_VPNC));

    QStringMap data;
    QVariantMap secretData;

    // General settings
    //   gateway
    if (!d->ui.leGateway->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_GATEWAY, d->ui.leGateway->text());
    }

    //   group name
    if (!d->ui.leGroupName->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_ID, d->ui.leGroupName->text());
    }

    //   user password
    if (!d->ui.leUserPassword->text().isEmpty() && d->ui.cboUserPassOptions->currentIndex() == 0) {
        secretData.insert(NM_VPNC_KEY_XAUTH_PASSWORD, d->ui.leUserPassword->text());
    }
    //   group password
    if (!d->ui.leGroupPassword->text().isEmpty() && d->ui.cboGroupPassOptions->currentIndex() == 0) {
        secretData.insert(NM_VPNC_KEY_SECRET, d->ui.leGroupPassword->text());
    }
    handleOnePasswordType(d->ui.cboUserPassOptions, NM_VPNC_KEY_XAUTH_PASSWORD_TYPE, data);
    handleOnePasswordType(d->ui.cboGroupPassOptions, NM_VPNC_KEY_SECRET_TYPE, data);

    // Optional settings
    //   username
    if (!d->ui.leUserName->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_XAUTH_USER, d->ui.leUserName->text());
    }

    //   domain
    if (!d->ui.leDomain->text().isEmpty()) {
        data.insert(NM_VPNC_KEY_DOMAIN, d->ui.leDomain->text());
    }

    //   encryption
    switch (d->ui.cboEncryptionMethod->currentIndex()) {
        case 1:
            data.insert(NM_VPNC_KEY_SINGLE_DES, QLatin1String("yes"));
            break;
        case 2:
            data.insert(NM_VPNC_KEY_NO_ENCRYPTION, QLatin1String("yes"));
            break;
        default:
            break;
    }

    // nat traversal
    switch (d->ui.cboNatTraversal->currentIndex()) {
        case 1:
            data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NATT));
            break;
        case 2:
            data.insert(NM_VPNC_KEY_NAT_TRAVERSAL_MODE, QLatin1String(NM_VPNC_NATT_MODE_NONE));
            break;
        default:
            break;
    }

    // dead peer detection
    if (d->ui.chkDeadPeerDetection->isChecked()) {
        if (d->dpdTimeout > 0) {
            data.insert(NM_VPNC_KEY_DPD_IDLE_TIMEOUT, QString::number(d->dpdTimeout));
        }
    } else {
        data.insert(NM_VPNC_KEY_DPD_IDLE_TIMEOUT, QString::number(0));
    }

    d->setting->setUserName(d->ui.leUserName->text());
    d->setting->setData(data);
    d->setting->setVpnSecrets(secretData);
}

uint VpncSettingWidget::handleOnePasswordType(const QComboBox * combo, const QString & key, QStringMap & data)
{
    uint type = combo->currentIndex();
    switch (type) {
        case 0:
            data.insert(key, NM_VPNC_PW_TYPE_ASK);
            break;
        case 1:
            data.insert(key, NM_VPNC_PW_TYPE_SAVE);
            break;
        case 2:
            data.insert(key, NM_VPNC_PW_TYPE_UNUSED);
            break;
    }
    return type;
}

void VpncSettingWidget::readSecrets()
{
    Q_D(VpncSettingWidget);
    QVariantMap secrets = d->setting->vpnSecrets();
    if (d->setting->data().value(NM_VPNC_KEY_XAUTH_PASSWORD_TYPE) == QLatin1String(NM_VPNC_PW_TYPE_SAVE)) {
        d->ui.leUserPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_XAUTH_PASSWORD)).toString());
    }
    fillOnePasswordCombo(d->ui.cboUserPassOptions, NM_VPNC_KEY_XAUTH_PASSWORD_TYPE, d->setting->data(), !d->ui.leUserName->text().isEmpty());

    if (d->setting->data().value(NM_VPNC_KEY_SECRET_TYPE) == QLatin1String(NM_VPNC_PW_TYPE_SAVE)) {
        d->ui.leGroupPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_SECRET)).toString());
    }
    fillOnePasswordCombo(d->ui.cboGroupPassOptions, NM_VPNC_KEY_SECRET_TYPE, d->setting->data(), !d->ui.leGroupPassword->text().isEmpty());
}

// vim: sw=4 sts=4 et tw=100
