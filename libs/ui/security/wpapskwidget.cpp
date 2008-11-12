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

#include <QWidget>

#include <KDebug>

#include "wpapskwidget.h"

#include <nm-setting-wireless-security.h>

#include "802_11_wireless_security_widget.h"
#include "secretstoragehelper.h"
#include "ui_wpapsk.h"

class WpaPskWidget::Private
{
public:
    Ui_WpaPsk ui;
    KConfig * config;
};

WpaPskWidget::WpaPskWidget(KConfig * config, const QString & connectionId, QWidget * parent)
: SecurityWidget(connectionId, parent), d(new WpaPskWidget::Private)
{
    d->config = config;
    d->ui.setupUi(this);
    connect(d->ui.chkShowPass, SIGNAL(toggled(bool)), this, SLOT(chkShowPassToggled(bool)));
    d->ui.psk->setEchoMode(QLineEdit::PasswordEchoOnEdit);
}

WpaPskWidget::~WpaPskWidget()
{
    delete d;
}

void WpaPskWidget::chkShowPassToggled(bool on)
{
    d->ui.psk->setEchoMode(on ? QLineEdit::Normal : QLineEdit::PasswordEchoOnEdit);
}

bool WpaPskWidget::validate() const
{
    return true;
}

void WpaPskWidget::readConfig()
{
    KConfigGroup cg(d->config, NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
    SecretStorageHelper secrets(m_connectionId, cg.name());
    QString fieldName = QLatin1String("psk");
    QString secret;
    secrets.readSecret(fieldName, secret);
    d->ui.psk->setText(secret);
    //d->ui.psk->setEnabled(true);
    d->ui.chkShowPass->setChecked(false);
}

void WpaPskWidget::writeConfig()
{
    KConfigGroup cg(d->config, NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
    cg.writeEntry("keymgmt", Wireless80211SecurityWidget::KEY_MGMT_WPA_PSK);

    SecretStorageHelper secrets(m_connectionId, QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME));
    kDebug() << "PSK is " << d->ui.psk->text();
    secrets.writeSecret("psk", d->ui.psk->text());
}


QVariantMap WpaPskWidget::secrets() const
{
    QVariantMap secrets;
    secrets.insert(QLatin1String(NM_SETTING_WIRELESS_SECURITY_PSK), QVariant(d->ui.psk->text()));
    return secrets;
}

#include "wpapskwidget.moc"
// vim: sw=4 sts=4 et tw=100
