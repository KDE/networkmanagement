/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>
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

#include "tlswidget.h"

#include "ui_eapmethodtlsbase.h"
#include "connection.h"
#include "settings/802-1x.h"

TlsWidget::TlsWidget(Knm::Connection* connection, QWidget * parent)
: EapMethod(connection, parent)
{
    setupUi(this);
}

TlsWidget::~TlsWidget()
{
}

bool TlsWidget::validate() const
{
    return true;
}

void TlsWidget::readConfig()
{
    kDebug() << "TODO:: Implement";
}

void TlsWidget::writeConfig()
{
    kDebug() << "TODO:: Implement";

}

void TlsWidget::readSecrets()
{
    kDebug() << "TODO:: Implement";
}

void TlsWidget::setPasswordMode(bool on)
{
    lePrivateKeyPassword->setPasswordMode(on);
}

// vim: sw=4 sts=4 et tw=100
