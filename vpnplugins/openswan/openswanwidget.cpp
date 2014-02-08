/*
    Copyright 2013 Jan Grulich <jgrulich@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "openswanwidget.h"
#include "ui_openswan.h"
#include "nm-openswan-service.h"

#include <QDBusMetaType>
#include <QDebug>
#include <KAcceleratorManager>

#include "connection.h"

class OpenswanWidgetPrivate
{
public:
    Ui::OpenswanWidget ui;
    Knm::VpnSetting * setting;
    enum AuthType {PrivateKey = 0, SshAgent, Smartcard, Eap};
};


OpenswanWidget::OpenswanWidget(Knm::Connection *connection, QWidget *parent):
    SettingWidget(connection, parent), d_ptr(new OpenswanWidgetPrivate)
{
    Q_D(OpenswanWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    connect(d->ui.cbUsernamePasswordMode, SIGNAL(currentIndexChanged(int)), SLOT(userPasswordTypeChanged(int)));
    connect(d->ui.cbGroupPasswordMode, SIGNAL(currentIndexChanged(int)), SLOT(groupPasswordTypeChanged(int)));

    connect(d->ui.ckShowPasswords, SIGNAL(toggled(bool)), SLOT(showPassword(bool)));

    connect(d->ui.gateway, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()));
    connect(d->ui.groupname, SIGNAL(textChanged(QString)), SLOT(slotWidgetChanged()));

    KAcceleratorManager::manage(this);
}

OpenswanWidget::~OpenswanWidget()
{
    delete d_ptr;
}

void OpenswanWidget::readConfig()
{
    Q_D(OpenswanWidget);

    const QStringMap data = d->setting->data();

    const QString gateway = data.value(NM_OPENSWAN_RIGHT);
    if (!gateway.isEmpty()) {
        d->ui.gateway->setText(gateway);
    }

    const QString groupName = data.value(NM_OPENSWAN_LEFTID);
    if (!groupName.isEmpty()) {
        d->ui.groupname->setText(groupName);
    }

    const QString userPasswordMode = data.value(NM_OPENSWAN_XAUTH_PASSWORD_INPUT_MODES);
    if (userPasswordMode == NM_OPENSWAN_PW_TYPE_SAVE) {
        d->ui.cbUsernamePasswordMode->setCurrentIndex(0);
    } else if (userPasswordMode == NM_OPENSWAN_PW_TYPE_ASK) {
        d->ui.cbUsernamePasswordMode->setCurrentIndex(1);
    } else if (userPasswordMode == NM_OPENSWAN_PW_TYPE_UNUSED) {
        d->ui.cbUsernamePasswordMode->setCurrentIndex(2);
    }

    const QString groupPasswordMode = data.value(NM_OPENSWAN_PSK_INPUT_MODES);
    if (groupPasswordMode == NM_OPENSWAN_PW_TYPE_SAVE) {
        d->ui.cbGroupPasswordMode->setCurrentIndex(0);
    } else if (groupPasswordMode == NM_OPENSWAN_PW_TYPE_ASK) {
        d->ui.cbGroupPasswordMode->setCurrentIndex(1);
    } else if (groupPasswordMode == NM_OPENSWAN_PW_TYPE_UNUSED) {
        d->ui.cbGroupPasswordMode->setCurrentIndex(2);
    }

    const QString username = data.value(NM_OPENSWAN_LEFTXAUTHUSER);
    if (!username.isEmpty()) {
        d->ui.username->setText(username);
    }

    const QString phase1 = data.value(NM_OPENSWAN_IKE);
    if (!phase1.isEmpty()) {
        d->ui.phase1->setText(phase1);
    }

    const QString phase2 = data.value(NM_OPENSWAN_ESP);
    if (!phase2.isEmpty()) {
        d->ui.phase2->setText(phase2);
    }

    const QString domain = data.value(NM_OPENSWAN_DOMAIN);
    if (!domain.isEmpty()) {
        d->ui.domain->setText(domain);
    }
}

void OpenswanWidget::readSecrets()
{
    Q_D(OpenswanWidget);
    QStringMap secrets = d->setting->vpnSecrets();

    const QString userPassword = secrets.value(NM_OPENSWAN_XAUTH_PASSWORD);
    if (!userPassword.isEmpty()) {
        d->ui.userPassword->setText(userPassword);
    }

    const QString groupPassword = secrets.value(NM_OPENSWAN_PSK_VALUE);
    if (!groupPassword.isEmpty()) {
        d->ui.groupPassword->setText(groupPassword);
    }
}

void OpenswanWidget::writeConfig()
{
    Q_D(OpenswanWidget);

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_OPENSWAN));
    QStringMap data;
    QStringMap secrets;

    if (!d->ui.gateway->text().isEmpty()) {
        data.insert(NM_OPENSWAN_RIGHT, d->ui.gateway->text());
    }

    if (!d->ui.groupname->text().isEmpty()) {
        data.insert(NM_OPENSWAN_LEFTID, d->ui.groupname->text());
    }

    if (!d->ui.userPassword->text().isEmpty()) {
        secrets.insert(NM_OPENSWAN_XAUTH_PASSWORD, d->ui.userPassword->text());
    }

    const int usernamePasswordMode = d->ui.cbUsernamePasswordMode->currentIndex();
    if (usernamePasswordMode == 0) {
        data.insert(NM_OPENSWAN_XAUTH_PASSWORD_INPUT_MODES, NM_OPENSWAN_PW_TYPE_SAVE);
        data.insert(NM_OPENSWAN_XAUTH_PASSWORD"-flags", QString::number(Knm::Setting::AgentOwned));
    } else if (usernamePasswordMode == 1) {
        data.insert(NM_OPENSWAN_XAUTH_PASSWORD_INPUT_MODES, NM_OPENSWAN_PW_TYPE_ASK);
        data.insert(NM_OPENSWAN_XAUTH_PASSWORD"-flags", QString::number(Knm::Setting::NotSaved));
    } else {
        data.insert(NM_OPENSWAN_XAUTH_PASSWORD_INPUT_MODES, NM_OPENSWAN_PW_TYPE_UNUSED);
        data.insert(NM_OPENSWAN_XAUTH_PASSWORD"-flags", QString::number(Knm::Setting::NotRequired));
    }

    if (!d->ui.groupPassword->text().isEmpty()) {
        secrets.insert(NM_OPENSWAN_PSK_VALUE, d->ui.groupPassword->text());
    }

    const int groupPasswordMode = d->ui.cbGroupPasswordMode->currentIndex();
    if (groupPasswordMode == Store) {
        data.insert(NM_OPENSWAN_PSK_INPUT_MODES, NM_OPENSWAN_PW_TYPE_SAVE);
        data.insert(NM_OPENSWAN_PSK_VALUE"-flags", QString::number(Knm::Setting::AgentOwned));
    } else if (groupPasswordMode == AlwaysAsk) {
        data.insert(NM_OPENSWAN_PSK_INPUT_MODES, NM_OPENSWAN_PW_TYPE_ASK);
        data.insert(NM_OPENSWAN_PSK_VALUE"-flags", QString::number(Knm::Setting::NotSaved));
    } else if (groupPasswordMode == NotRequired) {
        data.insert(NM_OPENSWAN_PSK_INPUT_MODES, NM_OPENSWAN_PW_TYPE_UNUSED);
        data.insert(NM_OPENSWAN_PSK_VALUE"-flags", QString::number(Knm::Setting::NotRequired));
    }

    if (!d->ui.username->text().isEmpty()) {
        data.insert(NM_OPENSWAN_LEFTXAUTHUSER, d->ui.username->text());
    }

    if (!d->ui.phase1->text().isEmpty()) {
        data.insert(NM_OPENSWAN_IKE, d->ui.phase1->text());
    }

    if (!d->ui.phase2->text().isEmpty()) {
        data.insert(NM_OPENSWAN_ESP, d->ui.phase2->text());
    }

    if (!d->ui.domain->text().isEmpty()) {
        data.insert(NM_OPENSWAN_DOMAIN, d->ui.domain->text());
    }

    d->setting->setData(data);
    d->setting->setVpnSecrets(secrets);
}

void OpenswanWidget::userPasswordTypeChanged(int index)
{
    Q_D(OpenswanWidget);
    if (index == AlwaysAsk || index == NotRequired) {
        d->ui.userPassword->setEnabled(false);
    } else {
        d->ui.userPassword->setEnabled(true);
    }
}

void OpenswanWidget::groupPasswordTypeChanged(int index)
{
    Q_D(OpenswanWidget);
    if (index == AlwaysAsk || index == NotRequired) {
        d->ui.groupPassword->setEnabled(false);
    } else {
        d->ui.groupPassword->setEnabled(true);
    }
}

void OpenswanWidget::showPassword(bool show)
{
    Q_D(OpenswanWidget);
    d->ui.userPassword->setPasswordMode(!show);
    d->ui.groupPassword->setPasswordMode(!show);
}
