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

#include "openvpnwidget.h"
#include "nm-openvpn-service.h"

#include <KDebug>
#include <nm-setting-vpn.h>
#include "settings/vpn.h"
#include "connection.h"

class OpenVpnSettingWidget::Private
{
public:
    Ui_OpenVPNProp ui;
    Knm::VpnSetting * setting;
};


OpenVpnSettingWidget::OpenVpnSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent)
{
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
}

OpenVpnSettingWidget::~OpenVpnSettingWidget()
{
    delete d;
}

void OpenVpnSettingWidget::readConfig()
{
    kDebug();

    QStringMap dataMap = d->setting->data();

    d->ui.chkUseLZO->setChecked( dataMap[NM_OPENVPN_KEY_COMP_LZO] == "yes" );

    QString cType = dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE];

    if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_PASSWORD_TLS ) )
    {
        d->ui.cmbConnectionType->setCurrentIndex( 3 );
        d->ui.x509PassUsername->setText( dataMap[NM_OPENVPN_KEY_USERNAME] );
        d->ui.x509PassCaFile->setPath( dataMap[NM_OPENVPN_KEY_CA] );
        d->ui.x509PassCert->setPath( dataMap[NM_OPENVPN_KEY_CERT] );
        d->ui.x509PassKey->setPath( dataMap[NM_OPENVPN_KEY_KEY] );
    } else if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_STATIC_KEY ) ) {
        d->ui.cmbConnectionType->setCurrentIndex( 1 );
    } else if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_PASSWORD ) ) {
        d->ui.passUserName->setText( dataMap[NM_OPENVPN_KEY_USERNAME] );
        d->ui.cmbConnectionType->setCurrentIndex( 2 );
    } else { // default
        d->ui.cmbConnectionType->setCurrentIndex( 0 );
        d->ui.x509CaFile->setPath( dataMap[NM_OPENVPN_KEY_CA] );
        d->ui.x509Cert->setPath( dataMap[NM_OPENVPN_KEY_CERT] );
        d->ui.x509Key->setPath( dataMap[NM_OPENVPN_KEY_CIPHER] );
    }

    //dataMap[NM_OPENVPN_KEY_TAP_DEV,
    //dataMap[NM_OPENVPN_KEY_LOCAL_IP,
    QString port = dataMap[NM_OPENVPN_KEY_PORT];
    d->ui.chkDefaultPort->setChecked(port.isEmpty());
    d->ui.port->setText( port );
    d->ui.gateway->setText( dataMap[NM_OPENVPN_KEY_REMOTE] );
}

void OpenVpnSettingWidget::writeConfig()
{
    kDebug();

    d->setting->setServiceType(QLatin1String("org.freedesktop.NetworkManager.openvpn"));

    QStringMap data;
    QStringMap secretData;
    data.insert(NM_OPENVPN_KEY_REMOTE, d->ui.gateway->text());

    data.insert( NM_OPENVPN_KEY_COMP_LZO, d->ui.chkUseLZO->isChecked() ? "yes" : "no" );

    const char *contype = NM_OPENVPN_CONTYPE_TLS;

    switch ( d->ui.cmbConnectionType->currentIndex())
    {
    case 0:
        contype = NM_OPENVPN_CONTYPE_TLS;
        data.insert( NM_OPENVPN_KEY_CA, d->ui.x509CaFile->url().path());
        data.insert( NM_OPENVPN_KEY_CERT, d->ui.x509Cert->url().path());
        data.insert( NM_OPENVPN_KEY_KEY, d->ui.x509Key->url().path());
        break;
    case 1:
        contype = NM_OPENVPN_CONTYPE_STATIC_KEY;
        break;
    case 2:
        contype = NM_OPENVPN_CONTYPE_PASSWORD;
        data.insert( NM_OPENVPN_KEY_USERNAME, d->ui.passUserName->text());
        d->setting->setUserName(d->ui.passUserName->text());
        secretData.insert(QLatin1String( NM_OPENVPN_KEY_PASSWORD ), d->ui.passPassword->text());
        break;
    case 3:
        contype = NM_OPENVPN_CONTYPE_PASSWORD_TLS;
        data.insert(NM_OPENVPN_KEY_USERNAME, d->ui.x509PassUsername->text());
        d->setting->setUserName(d->ui.x509PassUsername->text());
        data.insert(NM_OPENVPN_KEY_CA, d->ui.x509PassCaFile->url().path());
        data.insert(NM_OPENVPN_KEY_CERT, d->ui.x509PassCert->url().path());
        data.insert(NM_OPENVPN_KEY_KEY, d->ui.x509PassKey->url().path());
        secretData.insert(NM_OPENVPN_KEY_PASSWORD, d->ui.x509PassPassword->text());
        break;
    }
    data.insert( NM_OPENVPN_KEY_CONNECTION_TYPE, contype);

    //group.writeEntry( NM_OPENVPN_KEY_TAP_DEV,
    //group.writeEntry( NM_OPENVPN_KEY_KEY,
    //group.writeEntry( NM_OPENVPN_KEY_LOCAL_IP,
    data.insert( NM_OPENVPN_KEY_PROTO_TCP, d->ui.chkUseTCP->isChecked() ? "yes" : "no" );
    if ( !d->ui.chkDefaultPort->isChecked() )
    {
        data.insert(NM_OPENVPN_KEY_PORT, d->ui.port->text());
    }

    //group.writeEntry( NM_OPENVPN_KEY_REMOTE_IP,
    //group.writeEntry( NM_OPENVPN_KEY_STATIC_KEY,
    //group.writeEntry( NM_OPENVPN_KEY_STATIC_KEY_DIRECTION,
    //group.writeEntry( NM_OPENVPN_KEY_TA,
    //group.writeEntry( NM_OPENVPN_KEY_TA_DIR,

    d->setting->setData(data);
    d->setting->setVpnSecrets(secretData);
}

void OpenVpnSettingWidget::readSecrets()
{
    QStringMap secrets = d->setting->vpnSecrets();
    d->ui.x509PassPassword->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_PASSWORD)));
    d->ui.passPassword->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_PASSWORD)));
}

// vim: sw=4 sts=4 et tw=100
