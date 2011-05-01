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
#include "pindialog.h"
#include "ui_pinwidget.h"

#include <KDebug>
#include <kwindowsystem.h>

PinDialog::PinDialog(const Type type, QWidget *parent)
    : KDialog(parent), m_type(type)
{
    QWidget *w = new QWidget();
    ui = new Ui::PinWidget();
    ui->setupUi(w);
    ui->pin->setEchoMode(QLineEdit::Password);
    ui->errorMessage->setHidden(true);
    QRect desktop = KGlobalSettings::desktopGeometry(topLevelWidget());
    setMinimumWidth(qMin(1000, qMax(sizeHint().width(), desktop.width() / 4)));

    pixmapLabel = new QLabel( mainWidget() );
    pixmapLabel->setAlignment( Qt::AlignLeft | Qt::AlignTop );
    ui->gridLayout->addWidget( pixmapLabel, 0, 0 );
    pixmapLabel->setPixmap( KIcon("dialog-password").pixmap(KIconLoader::SizeHuge) );

    setButtons( KDialog::Ok | KDialog::Cancel);
    setDefaultButton( KDialog::Ok );
    button(KDialog::Ok)->setText(i18nc("As in 'Unlock cell phone with this pin code'", "Unlock"));
    setMainWidget(w);

    if (m_type == PinPuk) {
        setWindowTitle(i18n("SIM PUK unlock required"));
        ui->title->setText(i18n("SIM PUK Unlock Required"));
        ui->prompt->setText(i18n("The mobile broadband device '%s' requires a SIM PUK code before it can be used."));
        ui->pukLabel->setText(i18n("PUK code:"));
        ui->pinLabel->setText(i18n("New PIN code:"));
        ui->pin2Label->setText(i18n("Re-enter new PIN code:"));
        ui->chkShowPass->setText(i18n("Show PIN/PUK code"));

        ui->puk->setFocus();
        ui->pukLabel->show();
        ui->pin2Label->show();
    } else {
        setWindowTitle(i18n("SIM PIN unlock required"));
        ui->title->setText(i18n("SIM PIN Unlock Required"));
        ui->prompt->setText(i18n("The mobile broadband device '%s' requires a SIM PIN code before it can be used."));
        ui->pinLabel->setText(i18n("PIN code:"));
        ui->chkShowPass->setText(i18n("Show PIN code"));

        ui->pin->setFocus();
        ui->pukLabel->hide();
        ui->pin2Label->hide();
    }

    KWindowSystem::setState( winId(), NET::KeepAbove );
    KWindowSystem::setOnAllDesktops( winId(), true );
    KWindowSystem::activateWindow( winId());

    move((desktop.width() - width()) / 2, (desktop.height() - height()) / 2);
    connect(ui->chkShowPass, SIGNAL(stateChanged(int)), this, SLOT(chkShowPassToggled()));
}

PinDialog::~PinDialog()
{
}

void PinDialog::chkShowPassToggled()
{
    bool on = ui->chkShowPass->isChecked();
    ui->pin->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    ui->pin2->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
    ui->puk->setEchoMode(on ? QLineEdit::Normal : QLineEdit::Password);
}

PinDialog::Type PinDialog::type() const
{
    return m_type;
}

QString PinDialog::pin() const
{
    return ui->pin->text();
}

QString PinDialog::puk() const
{
    return ui->puk->text();
}

#endif
