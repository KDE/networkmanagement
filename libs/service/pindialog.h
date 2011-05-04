/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#ifdef COMPILE_MODEM_MANAGER_SUPPORT
#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <QWidget>
#include "ui_pinwidget.h"

#include <KDialog>
#include <KLocale>
#include <KPushButton>
#include <solid/device.h>
#include <solid/control/modemgsmcardinterface.h>

class PinWidget;

class PinDialog : public KDialog
{
Q_OBJECT
public:
    enum Type {Pin, PinPuk};
    enum ErrorCode {PinCodeTooShort, PinCodesDoNotMatch, PukCodeTooShort};

    PinDialog(Solid::Control::ModemGsmCardInterface *modem, const Type type = Pin, QWidget *parent=0);
    ~PinDialog();

    Type type() const;
    QString pin() const;
    QString pin2() const;
    QString puk() const;

public Q_SLOTS:
    void accept();

protected Q_SLOTS:
    void chkShowPassToggled();
    void modemInterfaceRemoved(const QString &udi);

private:
    void showErrorMessage(const PinDialog::ErrorCode);
    Ui::PinWidget * ui;
    QLabel* pixmapLabel;
    QString m_name;
    Type m_type;
    QString m_udi;
};

#endif

#endif
