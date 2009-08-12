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

#include "802_1x_security_widget.h"

#include "ui_802_1x.h"

class Wired8021xSecurityWidget::Private
{
public:
    Ui_Wired8021xSecurity ui;
};

Wired8021xSecurityWidget::Wired8021xSecurityWidget(Knm::Connection * connection, QWidget * parent)
: SettingWidget(connection, parent), d(new Wired8021xSecurityWidget::Private)
{
    d->ui.setupUi(this);
    //connect(d->ui.type, SIGNAL(currentIndexChanged(int)), this, SLOT(securityTypeChanged(int)));
}

Wired8021xSecurityWidget::~Wired8021xSecurityWidget()
{
    delete d;
}

void Wired8021xSecurityWidget::securityTypeChanged(int index)
{
    // None 0 -> 0
    // Wep 1,2,3 -> 1
    // Leap 4 -> 2
    // Dynamic WEP 5 -> 4
    // WPA PSK 6 ->  3
    // WPA EAP 7 -> 4
#if 0
    switch (index) {
        case 0: // None
            d->ui.stackedWidget->setCurrentIndex(0);
            break;
        case 1: // WEP
        case 2:
        case 3:
            d->ui.stackedWidget->setCurrentIndex(1);
            break;
        case 4: // LEAP
            d->ui.stackedWidget->setCurrentIndex(2);
            break;
        case 5:
            d->ui.stackedWidget->setCurrentIndex(4);
            break;
        case 6:
            d->ui.stackedWidget->setCurrentIndex(3);
            break;
        case 7:
            d->ui.stackedWidget->setCurrentIndex(4);
            break;
        default:
            break;
    }
#endif
}

// vim: sw=4 sts=4 et tw=100
