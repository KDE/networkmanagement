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

#include "connectionwidget.h"
#include "settingwidget_p.h"

#include <KDebug>

#include <kicondialog.h>
#include <kstandarddirs.h>

#include "ui_connection.h"
#include "connection.h"

#include "advancedpermissionswidget.h"

class ConnectionWidgetPrivate : public SettingWidgetPrivate
{
public:
    Ui_ConnectionSettings ui;
    QString defaultName;
    QHash<QString, QString> permissions;
};

ConnectionWidget::ConnectionWidget(QWidget * parent)
    : SettingWidget(*new ConnectionWidgetPrivate, parent)
{
    Q_D(ConnectionWidget);
    d->ui.setupUi(this);
    // Avoid double margins in dialog
    layout()->setMargin(0);
    d->valid = false; // valid on creation because connection name (id) is empty

    d->ui.pushButtonPermissions->setIcon(KIcon("preferences-desktop-user"));
    connect(d->ui.pushButtonPermissions, SIGNAL(clicked()), this, SLOT(buttonPermissionsClicked()));
    connect(d->ui.id, SIGNAL(textChanged(const QString&)), this, SLOT(validate()));
}

ConnectionWidget::~ConnectionWidget()
{
}

void ConnectionWidget::setDefaultName(const QString & defaultName)
{
    Q_D(ConnectionWidget);

    if (connection()->name() == i18n("New Wireless Connection"))
        d->ui.id->setText(defaultName);

    d->defaultName = defaultName;
}

QTabWidget * ConnectionWidget::connectionSettingsWidget()
{
    Q_D(ConnectionWidget);
    return d->ui.tabwidget;
}

void ConnectionWidget::readConfig()
{
    Q_D(ConnectionWidget);
    d->permissions = connection()->permissions();

    if (connection()->name().isEmpty()) {
        connection()->setName(d->defaultName);
    }

    d->ui.id->setText(connection()->name());
    d->ui.autoconnect->setChecked(connection()->autoConnect());
    d->ui.connectionIcon->setPixmap(KIcon(connection()->iconName()).pixmap(QSize(48,48)));
    d->ui.system->setChecked(d->permissions.isEmpty());
}

void ConnectionWidget::writeConfig()
{
    Q_D(ConnectionWidget);
    connection()->setName(d->ui.id->text());
    connection()->setAutoConnect(d->ui.autoconnect->isChecked());
    if (!d->ui.system->isChecked()) {
        if (d->permissions.isEmpty()) {
            connection()->addToPermissions(KUser().loginName(),QString());
            d->permissions = connection()->permissions();
        } else {
            connection()->setPermissions(d->permissions);
        }
    } else
        connection()->setPermissions(QHash<QString,QString>());
}

void ConnectionWidget::validate()
{
    Q_D(ConnectionWidget);
    d->valid = !d->ui.id->text().isEmpty();
    emit valid(d->valid);
}

void ConnectionWidget::buttonPermissionsClicked()
{
    Q_D(ConnectionWidget);
    kDebug() << "advanced permissions dialog clicked";
    QWeakPointer<KDialog> dialog = new KDialog(this);
    dialog.data()->setCaption(i18nc("@title:window advanced permissions editor",
                                "Advanced Permissions Editor"));
    dialog.data()->setButtons( KDialog::Ok | KDialog::Cancel);
    AdvancedPermissionsWidget permissionsWid(d->permissions);
    dialog.data()->setMainWidget(&permissionsWid);
    if (dialog.data()->exec() == QDialog::Accepted) {
        d->permissions = permissionsWid.currentUsers();
    }
    delete dialog.data();
}

// vim: sw=4 sts=4 et tw=100
