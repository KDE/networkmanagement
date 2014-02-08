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

#include "openswanauth.h"
#include "ui_openswanauth.h"

#include <nm-setting-vpn.h>

#include <QString>
#include <KAcceleratorManager>

#include "nm-openswan-service.h"
#include "connection.h"

class OpenswanAuthDialogPrivate
{
public:
    Ui_OpenswanAuth ui;
    Knm::VpnSetting * setting;
};

OpenswanAuthDialog::OpenswanAuthDialog(Knm::Connection *connection, QWidget *parent)
    : SettingWidget(connection, parent), d_ptr(new OpenswanAuthDialogPrivate)
{
    Q_D(OpenswanAuthDialog);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    connect(d->ui.cbShowPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsChanged(bool)));

    readSecrets();

    KAcceleratorManager::manage(this);
}

OpenswanAuthDialog::~OpenswanAuthDialog()
{
    delete d_ptr;
}

void OpenswanAuthDialog::readSecrets()
{
    Q_D(OpenswanAuthDialog);
    const QStringMap data = d->setting->data();
    const QStringMap secrets = d->setting->vpnSecrets();

    const QString groupName = data.value(NM_OPENSWAN_LEFTID);
    if (!groupName.isEmpty()) {
        d->ui.leGroupName->setText(groupName);
    }

    bool haveUserPassword = true;
    if (data.value(NM_OPENSWAN_XAUTH_PASSWORD_INPUT_MODES) != NM_OPENSWAN_PW_TYPE_UNUSED) {
        d->ui.leUserPassword->setText(secrets.value(NM_OPENSWAN_XAUTH_PASSWORD));
    } else {
        d->ui.leUserPassword->setVisible(false);
        d->ui.userPasswordLabel->setVisible(false);
        haveUserPassword = false;
    }

    bool haveGroupPassword = true;
    if (data.value(NM_OPENSWAN_PSK_INPUT_MODES) != NM_OPENSWAN_PW_TYPE_UNUSED) {
        d->ui.leGroupPassword->setText(secrets.value(NM_OPENSWAN_PSK_VALUE));
    } else {
        d->ui.leGroupPassword->setVisible(false);
        d->ui.groupPasswordLabel->setVisible(false);
        haveGroupPassword = false;
    }

    if (haveUserPassword && d->ui.leUserPassword->text().isEmpty()) {
        d->ui.leUserPassword->setFocus(Qt::OtherFocusReason);
    } else if (haveGroupPassword && d->ui.leGroupPassword->text().isEmpty()) {
        d->ui.leGroupPassword->setFocus(Qt::OtherFocusReason);
    }
}

void OpenswanAuthDialog::writeConfig()
{
    Q_D(OpenswanAuthDialog);

    QStringMap secrets;

    if (!d->ui.leUserPassword->text().isEmpty()) {
        secrets.insert(NM_OPENSWAN_XAUTH_PASSWORD, d->ui.leUserPassword->text());
    }

    if (!d->ui.leGroupPassword->text().isEmpty()) {
        secrets.insert(NM_OPENSWAN_PSK_VALUE, d->ui.leGroupPassword->text());
    }

    d->setting->setVpnSecrets(secrets);
}

void OpenswanAuthDialog::showPasswordsChanged(bool show)
{
    Q_D(OpenswanAuthDialog);
    d->ui.leUserPassword->setPasswordMode(!show);
    d->ui.leGroupPassword->setPasswordMode(!show);
}
