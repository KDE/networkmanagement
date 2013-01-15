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

#include "wpapskwidget.h"
#include "securitywidget_p.h"

#include <QWidget>

#include <nm-setting-wireless-security.h>
#include <nm-setting-connection.h>
#include <nm-setting-wireless.h>
#include <nm-setting-8021x.h>

#include <KDebug>
#include <wpasecretidentifier.h>

//#include "802_11_wireless_security_widget.h"
#include "ui_wpapsk.h"
#include "settings/802-11-wireless.h"
#include "settings/802-11-wireless-security.h"
#include "connection.h"


class WpaPskWidgetPrivate : public SecurityWidgetPrivate
{
public:
    Ui_WpaPsk ui;
    Knm::WirelessSetting* wsetting;
    Knm::WirelessSecuritySetting* setting;
};

WpaPskWidget::WpaPskWidget(Knm::Connection* connection, QWidget * parent)
: SecurityWidget(*new WpaPskWidgetPrivate, connection, parent)
{
    Q_D(WpaPskWidget);
    d->ui.setupUi(this);
    d->setting = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->wsetting = static_cast<Knm::WirelessSetting *>(connection->setting(Knm::Setting::Wireless));

    connect(d->ui.psk, SIGNAL(textChanged(QString)), SLOT(emitValid()));
    connect(d->ui.chkShowPass, SIGNAL(stateChanged(int)), this, SLOT(chkShowPassToggled()));
    d->ui.psk->setEchoMode(QLineEdit::Password);
}

WpaPskWidget::~WpaPskWidget()
{
}

void WpaPskWidget::chkShowPassToggled()
{
    Q_D(WpaPskWidget);
    bool on = d->ui.chkShowPass->isChecked();
    d->ui.psk->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

void WpaPskWidget::emitValid()
{
    emit valid(validate());
}

bool WpaPskWidget::validate() const
{
    Q_D(const WpaPskWidget);
    WpaSecretIdentifier::WpaSecretType secretType = WpaSecretIdentifier::identify(d->ui.psk->text());
    return (secretType == WpaSecretIdentifier::Passphrase || secretType == WpaSecretIdentifier::PreSharedKey);
}

void WpaPskWidget::readConfig()
{
    Q_D(WpaPskWidget);
    d->ui.chkShowPass->setChecked(false);
    chkShowPassToggled();
}

void WpaPskWidget::writeConfig()
{
    Q_D(WpaPskWidget);
    d->setting->setPsk(d->ui.psk->text());
    if (!d->connection->permissions().isEmpty()) {
        d->setting->setPskflags(Knm::Setting::AgentOwned);
    } else {
        d->setting->setPskflags(Knm::Setting::None);
    }
    d->wsetting->setSecurity(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
}

void WpaPskWidget::readSecrets()
{
    Q_D(WpaPskWidget);
    d->ui.psk->setText(d->setting->psk());
    emitValid();
}

// vim: sw=4 sts=4 et tw=100
