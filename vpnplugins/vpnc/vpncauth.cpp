/*
Copyright 2010 Andrey Borzenkov <arvidjaar@gmail.com>

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

#include "vpncauth.h"
#include "ui_vpncauth.h"

#include <QString>
#include "nm-vpnc-service.h"

#include "connection.h"

class VpncAuthWidgetPrivate
{
public:
    Ui_VpncAuth ui;
    Knm::VpnSetting * setting;
};

VpncAuthWidget::VpncAuthWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new VpncAuthWidgetPrivate)
{
    Q_D(VpncAuthWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    connect(d->ui.cbShowPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsChanged(bool)));
}

VpncAuthWidget::~VpncAuthWidget()
{
    delete d_ptr;
}

#if 0
void VpncAuthWidget::readConfig()
{
    Q_D(VpncAuthWidget);
}
#endif

void VpncAuthWidget::readSecrets()
{
    Q_D(VpncAuthWidget);
    QStringMap data = d->setting->data();
    QStringMap secrets = d->setting->vpnSecrets();

    //   username
    QString user = data.value(NM_VPNC_KEY_XAUTH_USER);
    if (!user.isEmpty()) {
        d->ui.leUserName->setText(user);
    }
    //   group name
    QString group = data.value(NM_VPNC_KEY_ID);
    if (!group.isEmpty()) {
        d->ui.leGroupName->setText(group);
    }

    bool haveUserPassword = true;
    if (!((Knm::Setting::secretsTypes)data.value(NM_VPNC_KEY_XAUTH_PASSWORD"-flags").toInt()).testFlag(Knm::Setting::NotRequired)) {
        d->ui.leUserPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_XAUTH_PASSWORD)));
    } else {
        d->ui.userNameLabel->setVisible(false);
        d->ui.leUserName->setVisible(false);
        d->ui.userPasswordLabel->setVisible(false);
        d->ui.leUserPassword->setVisible(false);
        haveUserPassword = false;
    }

    if (!((Knm::Setting::secretsTypes)data.value(NM_VPNC_KEY_SECRET"-flags").toInt()).testFlag(Knm::Setting::NotRequired)) {
        d->ui.leGroupPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_SECRET)));
    } else {
        d->ui.groupNameLabel->setVisible(false);
        d->ui.leGroupName->setVisible(false);
        d->ui.groupPasswordLabel->setVisible(false);
        d->ui.leGroupPassword->setVisible(false);
    }

    if (haveUserPassword && d->ui.leUserPassword->text().isEmpty())
        d->ui.leUserPassword->setFocus(Qt::OtherFocusReason);
    else if (d->ui.leGroupPassword->text().isEmpty())
        d->ui.leGroupPassword->setFocus(Qt::OtherFocusReason);
}

void VpncAuthWidget::writeConfig()
{
    Q_D(VpncAuthWidget);

    QStringMap secretData;

    //   user password
    if (!d->ui.leUserPassword->text().isEmpty()) {
        secretData.insert(NM_VPNC_KEY_XAUTH_PASSWORD, d->ui.leUserPassword->text());
    }
    //   group password
    if (!d->ui.leGroupPassword->text().isEmpty()) {
        secretData.insert(NM_VPNC_KEY_SECRET, d->ui.leGroupPassword->text());
    }

    d->setting->setVpnSecrets(secretData);
}

void VpncAuthWidget::showPasswordsChanged(bool show)
{
    Q_D(VpncAuthWidget);
    d->ui.leUserPassword->setPasswordMode(!show);
    d->ui.leGroupPassword->setPasswordMode(!show);
}

// vim: sw=4 sts=4 et tw=100
