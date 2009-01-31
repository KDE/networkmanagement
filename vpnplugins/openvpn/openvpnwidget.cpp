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

OpenVpnSettingWidget::OpenVpnSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent)
{
    m_ui.setupUi(this);
}

OpenVpnSettingWidget::~OpenVpnSettingWidget()
{

}

void OpenVpnSettingWidget::readConfig()
{
#if 0
    // PORTME
    kDebug();
    KConfigGroup group(configXml()->config(), settingName());

    QStringList data = group.readEntry( NM_SETTING_VPN_DATA, QStringList() );
    QMap<QString, QString> dataMap;

    for ( int i = 0; i < data.count(); i += 2 )
        dataMap.insert( data[i], data[i+1] );

    m_ui.chkUseLZO->setChecked( dataMap[NM_OPENVPN_KEY_COMP_LZO] == "yes" );
    SecretStorageHelper secrets(connectionId(), group.name());

    QString cType = dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE];

    if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_PASSWORD_TLS ) )
    {
        m_ui.cmbConnectionType->setCurrentIndex( 3 );
        m_ui.x509PassUsername->setText( dataMap[NM_OPENVPN_KEY_USERNAME] );
        m_ui.x509PassCaFile->setPath( dataMap[NM_OPENVPN_KEY_CA] );
        m_ui.x509PassCert->setPath( dataMap[NM_OPENVPN_KEY_CERT] );
        m_ui.x509PassKey->setPath( dataMap[NM_OPENVPN_KEY_KEY] );
        QString password;
        secrets.readSecret(NM_OPENVPN_KEY_PASSWORD, password);
        m_ui.x509PassPassword->setText( password );
    } else if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_STATIC_KEY ) )
        m_ui.cmbConnectionType->setCurrentIndex( 1 );
    else if ( cType == QLatin1String( NM_OPENVPN_CONTYPE_PASSWORD ) )
    {
        m_ui.passUserName->setText( dataMap[NM_OPENVPN_KEY_USERNAME] );
        QString password;
        secrets.readSecret(NM_OPENVPN_KEY_PASSWORD, password);
        m_ui.passPassword->setText( password );
        m_ui.cmbConnectionType->setCurrentIndex( 2 );
    } else { // default
        m_ui.cmbConnectionType->setCurrentIndex( 0 );
        m_ui.x509CaFile->setPath( dataMap[NM_OPENVPN_KEY_CA] );
        m_ui.x509Cert->setPath( dataMap[NM_OPENVPN_KEY_CERT] );
        m_ui.x509Key->setPath( dataMap[NM_OPENVPN_KEY_CIPHER] );
    }

    //dataMap[NM_OPENVPN_KEY_TAP_DEV,
    //dataMap[NM_OPENVPN_KEY_LOCAL_IP,
    QString port = dataMap[NM_OPENVPN_KEY_PORT];
    m_ui.chkDefaultPort->setChecked(port.isEmpty());
    m_ui.port->setText( port );
    m_ui.gateway->setText( dataMap[NM_OPENVPN_KEY_REMOTE] );
#endif
}

void OpenVpnSettingWidget::writeConfig()
{
// PORT ME
#if 0
    kDebug();
    KConfigGroup group(configXml()->config(), settingName());

    group.writeEntry( "servicetype", "org.freedesktop.NetworkManager.openvpn" );

    QStringList data;
    data.push_back( NM_OPENVPN_KEY_REMOTE );
    data.push_back( m_ui.gateway->text() );

    data.push_back( NM_OPENVPN_KEY_COMP_LZO );
    data.push_back( m_ui.chkUseLZO->isChecked() ? "yes" : "no" );

    const char *contype = NM_OPENVPN_CONTYPE_TLS;

    SecretStorageHelper secrets(connectionId(), group.name());

    switch ( m_ui.cmbConnectionType->currentIndex() )
    {
    case 0:
        contype = NM_OPENVPN_CONTYPE_TLS;
        data.push_back( NM_OPENVPN_KEY_CA );
        data.push_back( m_ui.x509CaFile->url().path() );
        data.push_back( NM_OPENVPN_KEY_CERT );
        data.push_back( m_ui.x509Cert->url().path() );
        data.push_back( NM_OPENVPN_KEY_KEY );
        data.push_back( m_ui.x509Key->url().path() );
        break;
    case 1:
        contype = NM_OPENVPN_CONTYPE_STATIC_KEY;
        break;
    case 2:
        contype = NM_OPENVPN_CONTYPE_PASSWORD;
        data.push_back( NM_OPENVPN_KEY_USERNAME );
        data.push_back( m_ui.passUserName->text() );
        group.writeEntry( NM_SETTING_VPN_USER_NAME, m_ui.passUserName->text() );
        secrets.writeSecret( QLatin1String( NM_OPENVPN_KEY_PASSWORD ), m_ui.passPassword->text() );
        break;
    case 3:
        contype = NM_OPENVPN_CONTYPE_PASSWORD_TLS;
        data.push_back( NM_OPENVPN_KEY_USERNAME );
        data.push_back( m_ui.x509PassUsername->text() );
        group.writeEntry( NM_SETTING_VPN_USER_NAME, m_ui.x509PassUsername->text() );
        data.push_back( NM_OPENVPN_KEY_CA );
        data.push_back( m_ui.x509PassCaFile->url().path() );
        data.push_back( NM_OPENVPN_KEY_CERT );
        data.push_back( m_ui.x509PassCert->url().path() );
        data.push_back( NM_OPENVPN_KEY_KEY );
        data.push_back( m_ui.x509PassKey->url().path() );
        secrets.writeSecret( NM_OPENVPN_KEY_PASSWORD, m_ui.x509PassPassword->text() );
        break;
    }
    data.push_back( NM_OPENVPN_KEY_CONNECTION_TYPE );
    data.push_back( contype );

    //group.writeEntry( NM_OPENVPN_KEY_TAP_DEV,
    //group.writeEntry( NM_OPENVPN_KEY_KEY,
    //group.writeEntry( NM_OPENVPN_KEY_LOCAL_IP,
    data.push_back( NM_OPENVPN_KEY_PROTO_TCP );
    data.push_back( m_ui.chkUseTCP->isChecked() ? "yes" : "no" );
    if ( !m_ui.chkDefaultPort->isChecked() )
    {
        data.push_back( NM_OPENVPN_KEY_PORT );
        data.push_back( m_ui.port->text() );
    }

    //group.writeEntry( NM_OPENVPN_KEY_REMOTE_IP,
    //group.writeEntry( NM_OPENVPN_KEY_STATIC_KEY,
    //group.writeEntry( NM_OPENVPN_KEY_STATIC_KEY_DIRECTION,
    //group.writeEntry( NM_OPENVPN_KEY_TA,
    //group.writeEntry( NM_OPENVPN_KEY_TA_DIR,

    group.writeEntry( NM_SETTING_VPN_DATA, data );
#endif
}

void OpenVpnSettingWidget::readSecrets()
{
}
// vim: sw=4 sts=4 et tw=100
