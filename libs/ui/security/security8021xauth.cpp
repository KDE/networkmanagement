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

#include "security8021xauth.h"
#include "settingwidget_p.h"

#include <connection.h>
#include <settings/802-1x.h>

#include <QFormLayout>
#include <QLabel>
#include <KLineEdit>
#include <QCheckBox>
#include <KLocale>

class Security8021xAuthWidgetPrivate : public SettingWidgetPrivate
{
public:
    Knm::Security8021xSetting *setting;
    QFormLayout *layout;
};

Security8021xAuthWidget::Security8021xAuthWidget(Knm::Connection* connection,  QWidget * parent)
: SettingWidget(*new Security8021xAuthWidgetPrivate, parent)
{
    Q_UNUSED(connection)
}

Security8021xAuthWidget::Security8021xAuthWidget(Knm::Connection* connection, const QStringList &secrets, QWidget * parent)
: SettingWidget(*new Security8021xAuthWidgetPrivate, parent)
{
    Q_D(Security8021xAuthWidget);
    d->connection = connection;
    d->setting = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));

    d->layout = new QFormLayout(this);
    this->setLayout(d->layout);

    if (secrets.contains(QLatin1String("password"))) {
        QLabel *label = new QLabel(this);
        label->setText(i18n("Password:"));
        KLineEdit *lineEdit = new KLineEdit(this);
        lineEdit->setPasswordMode(true);
        lineEdit->setProperty("setting", "password");
        d->layout->addRow(label, lineEdit);
    }
    if (secrets.contains(QLatin1String("private-key-password"))) {
        QLabel *label = new QLabel(this);
        label->setText(i18n("Private Key Password:"));
        KLineEdit *lineEdit = new KLineEdit(this);
        lineEdit->setPasswordMode(true);
        lineEdit->setProperty("setting", "private-key-password");
        d->layout->addRow(label, lineEdit);
    }
    if (secrets.contains(QLatin1String("phase2-private-key-password"))) {
        QLabel *label = new QLabel(this);
        label->setText(i18n("Phase 2 Private Key Password:"));
        KLineEdit *lineEdit = new KLineEdit(this);
        lineEdit->setPasswordMode(true);
        lineEdit->setProperty("setting", "phase2-private-key-password");
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
    d->layout->setWidget(d->layout->rowCount(), QFormLayout::FieldRole, showPasswords);
    connect(showPasswords, SIGNAL(toggled(bool)), this, SLOT(showPasswordsToggled(bool)));
    d->layout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
}

Security8021xAuthWidget::~Security8021xAuthWidget()
{
}

void Security8021xAuthWidget::showPasswordsToggled(bool toggled)
{
    Q_D(Security8021xAuthWidget);
    for (int i = 0; i < d->layout->rowCount() - 1; i++)
    {
        KLineEdit *le = qobject_cast<KLineEdit*>(d->layout->itemAt(i, QFormLayout::FieldRole)->widget());
        if (le) {
            le->setPasswordMode(!toggled);
        }
    }
}

void Security8021xAuthWidget::writeConfig()
{
    Q_D(Security8021xAuthWidget);

    for (int i = 0; i < d->layout->rowCount() - 1; i++)
    {
        KLineEdit *le = qobject_cast<KLineEdit*>(d->layout->itemAt(i, QFormLayout::FieldRole)->widget());
        if (le) {
            QString setting = le->property("setting").toString();
            if (setting == QLatin1String("password")) {
                d->setting->setPassword(le->text());
            } else if (setting == QLatin1String("private-key-password")) {
                d->setting->setPrivatekeypassword(le->text());
            } else if (setting == QLatin1String("phase2-private-key-password")) {
                d->setting->setPhase2privatekeypassword(le->text());
            }
        }
    }
}
