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

#include "othersettingswidget.h"

#include <KNotifyConfigWidget>

#include "knmserviceprefs.h"
#include "detailkeyseditor.h"
#include "../../plasma_nm_version.h"

class OtherSettingsWidgetPrivate
{
public:
    OtherSettingsWidgetPrivate()
    { }
    Ui_OtherSettings ui;
    QStringList detailKeys;
};

OtherSettingsWidget::OtherSettingsWidget(QWidget * parent)
: QWidget(parent), d_ptr(new OtherSettingsWidgetPrivate)
{
    Q_D(OtherSettingsWidget);
    d->ui.setupUi(this);
    d->ui.lblVersion->setText(i18nc("Version text", "<b>Version %1</b>", plasmaNmVersion)); // krazy:exclude=i18ncheckarg
    d->ui.detailsButton->setIcon(KIcon("view-list-details"));
    d->ui.notificationsButton->setIcon(KIcon("preferences-desktop-notification"));
    connect(d->ui.detailsButton, SIGNAL(clicked()), SLOT(configureDetails()));
    connect(d->ui.notificationsButton, SIGNAL(clicked()), SLOT(configureNotifications()));
}

OtherSettingsWidget::~OtherSettingsWidget()
{
    delete d_ptr;
}

void OtherSettingsWidget::configureDetails()
{
    Q_D(OtherSettingsWidget);
    kDebug();

    KNetworkManagerServicePrefs::self()->readConfig();
    KConfigGroup config(KNetworkManagerServicePrefs::self()->config(), QLatin1String("General"));
    d->detailKeys = config.readEntry(QLatin1String("DetailKeys"), QStringList());

    QWeakPointer<KDialog> dialog = new KDialog(this);
    dialog.data()->setCaption(i18nc("@title:window interface details editor",
                                "Interface Details Editor"));
    dialog.data()->setButtons( KDialog::Ok | KDialog::Cancel);
    DetailKeysEditor detailsEditor(d->detailKeys);
    dialog.data()->setMainWidget(&detailsEditor);
    if (dialog.data()->exec() == QDialog::Accepted) {
        d->detailKeys = detailsEditor.currentDetails();
        config.writeEntry(QLatin1String("DetailKeys"), d->detailKeys);
        config.sync();
    }
    if (dialog) {
        dialog.data()->deleteLater();
    }
}

void OtherSettingsWidget::configureNotifications()
{
    KNotifyConfigWidget::configure(this, "networkmanagement");
}
// vim: sw=4 sts=4 et tw=100
