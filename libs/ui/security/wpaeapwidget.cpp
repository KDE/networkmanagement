/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "wpaeapwidget.h"
#include "eapmethodstack_p.h"

#include <QCheckBox>
#include <KDebug>

#include <nm-setting-wireless-security.h>
#include <nm-setting-8021x.h>

//#include "802_11_wireless_security_widget.h"
#include "eapmethodleap.h"
#include "peapwidget.h"
#include "ttlswidget.h"
#include "tlswidget.h"
#include "connection.h"
#include "settings/802-11-wireless-security.h"
#include "settings/802-1x.h"

class WpaEapWidgetPrivate : public EapMethodStackPrivate
{
public:
    Knm::WirelessSecuritySetting * settingSecurity;
    Knm::Security8021xSetting * setting8021x;
    int tlsKey;
    int leapKey;
    int peapKey;
    int ttlsKey;
    QCheckBox * chkShowPassword;
};

WpaEapWidget::WpaEapWidget(Knm::Connection* connection, QWidget * parent)
: EapMethodStack(*new WpaEapWidgetPrivate, connection, parent)
{
    Q_D(WpaEapWidget);
    d->tlsKey = 0;
    d->leapKey = 1;
    d->peapKey = 2;
    d->ttlsKey = 3;
    d->settingSecurity = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->setting8021x = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));

    d->chkShowPassword = new QCheckBox(this);
    d->chkShowPassword->setObjectName(QString::fromUtf8("chkShowPassword"));
    d->chkShowPassword->setText(i18nc("show passwords button", "&Show Passwords"));

    verticalLayout->addWidget(d->chkShowPassword);

    bool isInnerMethod = false;
    registerEapMethod(d->tlsKey, new TlsWidget(isInnerMethod, connection, eapMethods),
            i18nc("TLS auth type", "TLS"));

    registerEapMethod(d->leapKey, new EapMethodLeap(connection, eapMethods),
            i18nc("LEAP auth type", "LEAP"));

    registerEapMethod(d->peapKey, new PeapWidget(connection, eapMethods),
            i18nc("Peap outer auth type", "Protected EAP (PEAP)"));

    registerEapMethod(d->ttlsKey, new TtlsWidget(connection, eapMethods),
            i18nc("TTLS outer auth type", "Tunnelled TLS (TTLS)"));
}

WpaEapWidget::~WpaEapWidget()
{
}

void WpaEapWidget::registerEapMethod(int key, EapMethod * eapMethod, const QString & theLabel)
{
    Q_D(WpaEapWidget);

    connect(d->chkShowPassword, SIGNAL(toggled(bool)), eapMethod, SLOT(setShowPasswords(bool)));

    EapMethodStack::registerEapMethod(key, eapMethod, theLabel);
}

bool WpaEapWidget::validate() const
{
    return true;
}

void WpaEapWidget::readConfig()
{
    Q_D(WpaEapWidget);
    // This is WPA EAP, we have to handle
    // 1. the wireless security setting called by the wireless security top level widget
    // 2. the 802.1x setting called directly by the KCModule
    // As we cannot distinquish the callers just handle both settings

    Knm::Security8021xSetting::EapMethods eap = d->setting8021x->eapFlags();

    // default is peap
    int key = d->peapKey;

    if (eap.testFlag(Knm::Security8021xSetting::tls)) {
        key = d->tlsKey;
    } else if (eap.testFlag(Knm::Security8021xSetting::peap)) {
        key = d->peapKey;
    } else if (eap.testFlag(Knm::Security8021xSetting::ttls)) {
        key = d->ttlsKey;
    } else if (eap.testFlag(Knm::Security8021xSetting::leap)) {
        key = d->leapKey;
    }
    setCurrentEapMethod(key);
    currentEapMethod()->readConfig();
}

void WpaEapWidget::writeConfig()
{
    currentEapMethod()->writeConfig();
}


void WpaEapWidget::readSecrets()
{
    // rely on readConfig before readSecrets!
    currentEapMethod()->readSecrets();
}

// vim: sw=4 sts=4 et tw=100
