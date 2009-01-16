/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>

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
#include "ui_security_tls.h"

class TlsWidget::Private
{
public:
    Ui_Tls ui;
    KConfig * config;
};

TlsWidget::TlsWidget(KConfig* config, const QString & connectionId, QWidget * parent)
: EapWidget(connectionId, parent), d(new TlsWidget::Private)
{
    d->ui.setupUi(this);
    d->config = config;
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

}

void TlsWidget::writeConfig()
{

}

QVariantMap TlsWidget::secrets() const
{
    QVariantMap ourSecrets;
    return ourSecrets;
}
// vim: sw=4 sts=4 et tw=100
