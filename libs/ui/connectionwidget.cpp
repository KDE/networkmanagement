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

    d->ui.pushButtonChooseIcon->setToolTip(i18nc("@info:tooltip user action", "Choose a connection icon"));
    d->ui.pushButtonPermissions->setIcon(KIcon("preferences-desktop-user"));
    // it doesn't make a sense to set up an icon here, lets do it in readConfig
    connect(d->ui.pushButtonChooseIcon, SIGNAL(clicked()), this, SLOT(buttonChooseIconClicked()));
    connect(d->ui.pushButtonPermissions, SIGNAL(clicked()), this, SLOT(buttonPermissionsClicked()));
    connect(d->ui.id, SIGNAL(textChanged(const QString&)), this, SLOT(validate()));
}

ConnectionWidget::~ConnectionWidget()
{
}

void ConnectionWidget::setConnection(Knm::Connection * connection)
{
    kDebug() << "Connection id is " << connection->uuid();
    d_ptr->connection = connection;
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
    d->ui.pushButtonChooseIcon->setIcon(KIcon(connection()->iconName()));
    d->ui.system->setChecked(d->permissions.isEmpty());
}

void ConnectionWidget::writeConfig()
{
    Q_D(ConnectionWidget);
    connection()->setName(d->ui.id->text());
    connection()->setAutoConnect(d->ui.autoconnect->isChecked());
    if (!d->ui.system->isChecked())
        connection()->setPermissions(d->permissions);
    else
        connection()->setPermissions(QHash<QString,QString>());
    // connection()->setIconName(..) is already called from buttonChooseIconClicked()
}

void ConnectionWidget::validate()
{
    Q_D(ConnectionWidget);
    d->valid = !d->ui.id->text().isEmpty();
    emit valid(d->valid);
}

void ConnectionWidget::buttonChooseIconClicked()
{
    Q_D(ConnectionWidget);
    KIconDialog dlg(this);

    // set customLocation to kdedir/share/apps/networkmanagement/icons
    QString customLocation(KStandardDirs::locate("data", QLatin1String("networkmanagement/icons/")));
    //qDebug() << "Custom location: " << customLocation;

    dlg.setCustomLocation(customLocation);

    dlg.setup(KIconLoader::NoGroup, KIconLoader::Any, false, 0, true, false, false);

    QString iconName = dlg.openDialog();
    if (!iconName.isEmpty())
    {
        //qDebug() << "Icon name: " << iconName;
        d->ui.pushButtonChooseIcon->setIcon(KIcon(iconName));
        connection()->setIconName(iconName);
    }
}

void ConnectionWidget::buttonPermissionsClicked()
{
    Q_D(ConnectionWidget);
    kDebug() << "advanced permissions dialog clicked";
    KDialog dialog(this);
    dialog.setCaption(i18nc("@title:window advanced permissions editor",
                                "Advanced Permissions Editor"));
    dialog.setButtons( KDialog::Ok | KDialog::Cancel);
    AdvancedPermissionsWidget permissionsWid(d->permissions);
    dialog.setMainWidget(&permissionsWid);
    if (dialog.exec() == QDialog::Accepted) {
        d->permissions = permissionsWid.currentUsers();
    }
}

// vim: sw=4 sts=4 et tw=100
