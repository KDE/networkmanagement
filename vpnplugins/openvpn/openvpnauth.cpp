/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#include "openvpnauth.h"

#include <QString>
#include <QFormLayout>
#include <QLabel>
#include <QCheckBox>
#include <KLineEdit>
#include <KLocale>
#include "nm-openvpn-service.h"

#include "connection.h"

class OpenVpnAuthWidgetPrivate
{
public:
    Knm::VpnSetting * setting;
    QFormLayout *layout;
};

OpenVpnAuthWidget::OpenVpnAuthWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new OpenVpnAuthWidgetPrivate)
{
    Q_D(OpenVpnAuthWidget);
    d->setting = static_cast<Knm::VpnSetting *>(connection->setting(Knm::Setting::Vpn));
    d->layout = new QFormLayout(this);
    this->setLayout(d->layout);
}

OpenVpnAuthWidget::~OpenVpnAuthWidget()
{
    delete d_ptr;
}

void OpenVpnAuthWidget::readSecrets()
{
    Q_D(OpenVpnAuthWidget);
    QStringMap secrets = d->setting->vpnSecrets();
    QStringMap dataMap = d->setting->data();
    QString cType = dataMap[NM_OPENVPN_KEY_CONNECTION_TYPE];
    QLabel *label;
    KLineEdit *lineEdit;

    Knm::Setting::secretsTypes certType = (Knm::Setting::secretsTypes)dataMap[NM_OPENVPN_KEY_CERTPASS"-flags"].toInt();
    Knm::Setting::secretsTypes passType = (Knm::Setting::secretsTypes)dataMap[NM_OPENVPN_KEY_PASSWORD"-flags"].toInt();
    Knm::Setting::secretsTypes proxyType = (Knm::Setting::secretsTypes)dataMap[NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD"-flags"].toInt();

    if (cType == QLatin1String(NM_OPENVPN_CONTYPE_TLS) && !(certType & Knm::Setting::NotRequired)) {
        label = new QLabel(this);
        label->setText(i18n("Key Password:"));
        lineEdit = new KLineEdit(this);
        lineEdit->setPasswordMode(true);
        lineEdit->setProperty("nm_secrets_key", QLatin1String(NM_OPENVPN_KEY_CERTPASS));
        lineEdit->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_CERTPASS)));
        d->layout->addRow(label, lineEdit);
    } else if (cType == QLatin1String(NM_OPENVPN_CONTYPE_PASSWORD) && !(passType & Knm::Setting::NotRequired)) {
        label = new QLabel(this);
        label->setText(i18n("Password:"));
        lineEdit = new KLineEdit(this);
        lineEdit->setPasswordMode(true);
        lineEdit->setProperty("nm_secrets_key", QLatin1String(NM_OPENVPN_KEY_PASSWORD));
        lineEdit->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_PASSWORD)));
        d->layout->addRow(label, lineEdit);
    } else if (cType == QLatin1String(NM_OPENVPN_CONTYPE_PASSWORD_TLS)) {
        if (!(passType.testFlag(Knm::Setting::NotRequired))) {
            label = new QLabel(this);
            label->setText(i18n("Password:"));
            lineEdit = new KLineEdit(this);
            lineEdit->setPasswordMode(true);
            lineEdit->setProperty("nm_secrets_key", QLatin1String(NM_OPENVPN_KEY_PASSWORD));
            lineEdit->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_PASSWORD)));
            d->layout->addRow(label, lineEdit);
        }
        if (!(certType.testFlag(Knm::Setting::NotRequired))) {
            label = new QLabel(this);
            label->setText(i18n("Key Password:"));
            lineEdit = new KLineEdit(this);
            lineEdit->setPasswordMode(true);
            lineEdit->setProperty("nm_secrets_key", QLatin1String(NM_OPENVPN_KEY_CERTPASS));
            lineEdit->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_CERTPASS)));
            d->layout->addRow(label, lineEdit);
        }
    }

    if (!(proxyType & Knm::Setting::NotRequired)) {
        label = new QLabel(this);
        label->setText(i18n("Proxy Password:"));
        lineEdit = new KLineEdit(this);
        lineEdit->setPasswordMode(true);
        lineEdit->setProperty("nm_secrets_key", QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD));
        lineEdit->setText(secrets.value(QLatin1String(NM_OPENVPN_KEY_HTTP_PROXY_PASSWORD)));
        d->layout->addRow(label, lineEdit);
    }

    for (int i = 0; i < d->layout->rowCount(); i++)
    {
        KLineEdit *le = qobject_cast<KLineEdit*>(d->layout->itemAt(i, QFormLayout::FieldRole)->widget());
        if (le && le->text().isEmpty()) {
            le->setFocus(Qt::OtherFocusReason);
            break;
        }
    }

    QCheckBox *showPasswords = new QCheckBox(this);
    showPasswords->setText(i18n("&Show password"));
    d->layout->addRow(showPasswords);
    connect(showPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsToggled(bool)));
}

void OpenVpnAuthWidget::writeConfig()
{
    Q_D(OpenVpnAuthWidget);

    QStringMap secretData;
    for (int i = 0; i < d->layout->rowCount() - 1; i++)
    {
        KLineEdit *le = qobject_cast<KLineEdit*>(d->layout->itemAt(i, QFormLayout::FieldRole)->widget());
        if (le && !le->text().isEmpty()) {
            QString key = le->property("nm_secrets_key").toString();
            secretData.insert(key, le->text());
        }
    }

    d->setting->setVpnSecrets(secretData);
}

void OpenVpnAuthWidget::showPasswordsToggled(bool toggled)
{
    Q_D(OpenVpnAuthWidget);
    for (int i = 0; i < d->layout->rowCount() - 1; i++)
    {
        KLineEdit *le = qobject_cast<KLineEdit*>(d->layout->itemAt(i, QFormLayout::FieldRole)->widget());
        if (le) {
            le->setPasswordMode(!toggled);
        }
    }
}
