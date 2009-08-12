/*
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

#include "pptpwidget.h"
#include <KDialog>

#include <nm-setting-vpn.h>

#include "ui_pptpprop.h"
#include "ui_pptpadvanced.h"

#include <QString>
#include "nm-pptp-service.h"

#include "connection.h" 

class PptpSettingWidgetPrivate
{
public:
    Ui_PptpProp ui;
    Ui_PptpAdvanced advUi;
    Knm::VpnSetting * setting;
    KDialog * advancedDlg;
    QWidget * advancedWid;
};

PptpSettingWidget::PptpSettingWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d_ptr(new PptpSettingWidgetPrivate)
{
    Q_D(PptpSettingWidget);
    connect(d->ui.btnAdvanced, SIGNAL(clicked()), this, SLOT(doAdvancedDialog()));
    d->advancedDlg = new KDialog(this);
    d->advancedWid = new QWidget(this);
    d->advUi.setupUi(d->advancedWid);
    d->advancedDlg->setMainWidget(d->advancedWid);
}

PptpSettingWidget::~PptpSettingWidget()
{

}

void PptpSettingWidget::doAdvancedDialog()
{
    Q_D(PptpSettingWidget);
    d->advancedDlg->exec();
    // Check return value and mark some flag indicating that the dialog is dirty
}

void PptpSettingWidget::readConfig()
{
    Q_D(PptpSettingWidget);
    // General settings
    QStringMap dataMap = d->setting->data();
}

void PptpSettingWidget::writeConfig()
{
    // save the main dialog's data in the setting
    // if the advanced dialog is dirty, save its data in the vpn setting too
}

void PptpSettingWidget::readSecrets()
{
    Q_D(PptpSettingWidget);
    QVariantMap secrets = d->setting->vpnSecrets();
}

// vim: sw=4 sts=4 et tw=100
