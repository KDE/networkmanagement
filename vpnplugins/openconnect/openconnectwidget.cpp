/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.de>

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

#include "openconnectwidget.h"
#include <KDialog>

#include <nm-setting-vpn.h>

#include "ui_openconnectprop.h"

#include <QString>
#include "nm-openconnect-service.h"

#include "connection.h"

class OpenconnectSettingWidgetPrivate
{
public:
    Ui_OpenconnectProp ui;
    Knm::VpnSetting * setting;
};

OpenconnectSettingWidget::OpenconnectSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new OpenconnectSettingWidgetPrivate)
{
    Q_D(OpenconnectSettingWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
}

OpenconnectSettingWidget::~OpenconnectSettingWidget()
{
    delete d_ptr;
}

void OpenconnectSettingWidget::readConfig()
{
    Q_D(OpenconnectSettingWidget);
    // General settings
    QStringMap dataMap = d->setting->data();
    QStringMap vpnSecretsMap = d->setting->vpnSecrets();

    d->ui.leGateway->setText(dataMap[NM_OPENCONNECT_KEY_GATEWAY]);
    d->ui.leCaCertificate->setUrl(KUrl(dataMap[NM_OPENCONNECT_KEY_CACERT]));
    d->ui.leProxy->setText(dataMap[NM_OPENCONNECT_KEY_PROXY]);
    d->ui.chkAllowTrojan->setChecked(dataMap[NM_OPENCONNECT_KEY_CSD_ENABLE] == "yes");
    d->ui.leCsdWrapperScript->setUrl(KUrl(dataMap[NM_OPENCONNECT_KEY_CSD_WRAPPER]));
    d->ui.leUserCert->setUrl(KUrl(dataMap[NM_OPENCONNECT_KEY_USERCERT]));
    d->ui.leUserPrivateKey->setUrl(KUrl(dataMap[NM_OPENCONNECT_KEY_PRIVKEY]));
    d->ui.chkUseFsid->setChecked(dataMap[NM_OPENCONNECT_KEY_PEM_PASSPHRASE_FSID] == "yes");
}

void OpenconnectSettingWidget::writeConfig()
{
    Q_D(OpenconnectSettingWidget);

    d->setting->setServiceType(QLatin1String(NM_DBUS_SERVICE_OPENCONNECT));

    QStringMap data;
    QStringMap secretData;

    data.insert(QLatin1String(NM_OPENCONNECT_KEY_GATEWAY), d->ui.leGateway->text().toUtf8());
    if (!d->ui.leCaCertificate->url().isEmpty())
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_CACERT), d->ui.leCaCertificate->url().path());
    if (!d->ui.leProxy->text().isEmpty())
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_PROXY), d->ui.leProxy->text().toUtf8());
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_CSD_ENABLE), d->ui.chkAllowTrojan->isChecked() ? "yes" : "no");
    if (!d->ui.leCsdWrapperScript->url().isEmpty())
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_CSD_WRAPPER), d->ui.leCsdWrapperScript->url().path());
    if (!d->ui.leUserCert->url().isEmpty())
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_USERCERT), d->ui.leUserCert->url().path());
    if (!d->ui.leUserPrivateKey->url().isEmpty())
        data.insert(QLatin1String(NM_OPENCONNECT_KEY_PRIVKEY), d->ui.leUserPrivateKey->url().path());
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_PEM_PASSPHRASE_FSID), d->ui.chkUseFsid->isChecked() ? "yes" : "no");

    /* These are different for every login session, and should not be stored */
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_COOKIE"-flags"), QString::number(Knm::Setting::NotSaved));
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_GWCERT"-flags"), QString::number(Knm::Setting::NotSaved));
    data.insert(QLatin1String(NM_OPENCONNECT_KEY_GATEWAY"-flags"), QString::number(Knm::Setting::NotSaved));

    d->setting->setData(data);

    //VpnSecrets empty so far, they will be acquired by the auth dialog
    d->setting->setVpnSecrets(secretData);
}

void OpenconnectSettingWidget::readSecrets()
{
}

void OpenconnectSettingWidget::validate()
{
}
