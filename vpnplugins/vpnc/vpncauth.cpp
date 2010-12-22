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

#include <nm-setting-vpn.h>

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
}

VpncAuthWidget::~VpncAuthWidget()
{

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
    QVariantMap secrets = d->setting->vpnSecrets();
    QStringMap secretsType = d->setting->secretsStorageType();
    QString userType;
    QString groupType;

    /*
     * First time "old" settings are loaded secretsType map is empty, so
     * try to read from data as fallback
     */
    userType = secretsType.value(NM_VPNC_KEY_XAUTH_PASSWORD_TYPE);
    if (userType.isNull()) {
        userType = d->setting->data().value(NM_VPNC_KEY_XAUTH_PASSWORD_TYPE);
    }
    if (userType == QLatin1String(NM_VPN_PW_TYPE_SAVE)) {
        d->ui.leUserPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_XAUTH_PASSWORD)).toString());
        d->ui.leUserPassword->setEnabled(false);
    }

    groupType = secretsType.value(NM_VPNC_KEY_SECRET_TYPE);
    if (userType.isNull()) {
        groupType = d->setting->data().value(NM_VPNC_KEY_SECRET_TYPE);
    }
    if (groupType == QLatin1String(NM_VPN_PW_TYPE_SAVE)) {
        d->ui.leGroupPassword->setText(secrets.value(QLatin1String(NM_VPNC_KEY_SECRET)).toString());
        d->ui.leGroupPassword->setEnabled(false);
    }
}

void VpncAuthWidget::writeConfig()
{
    Q_D(VpncAuthWidget);

    QVariantMap secretData;

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

// vim: sw=4 sts=4 et tw=100
