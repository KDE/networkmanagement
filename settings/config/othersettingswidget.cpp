/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010 Sebastian Kügler <sebas@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

Thsis program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <KNotifyConfigWidget>

#include "othersettingswidget.h"
#include "knmserviceprefs.h"
#include "../../plasma_nm_version.h"

class OtherSettingsWidgetPrivate
{
public:
    OtherSettingsWidgetPrivate()
    { }
    Ui_OtherSettings ui;
};

OtherSettingsWidget::OtherSettingsWidget(QWidget * parent)
: QWidget(parent), d_ptr(new OtherSettingsWidgetPrivate)
{
    Q_D(OtherSettingsWidget);
    d->ui.setupUi(this);
    d->ui.lblVersion->setText(i18nc("Version text", "<b>Version %1</b>", plasmaNmVersion));
    d->ui.lblVersion->setTextInteractionFlags(Qt::TextSelectableByMouse);
    d->ui.notificationsButton->setIcon(KIcon("preferences-desktop-notification"));
    connect(d->ui.notificationsButton, SIGNAL(clicked()), SLOT(configureNotifications()));
}

OtherSettingsWidget::~OtherSettingsWidget()
{
    delete d_ptr;
}

void OtherSettingsWidget::configureNotifications()
{
    KNotifyConfigWidget::configure(this, "networkmanagement");
}
// vim: sw=4 sts=4 et tw=100
