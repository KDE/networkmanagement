/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "securitywired8021x.h"

#include <QCheckBox>

#include <connection.h>

#include <settings/802-1x.h>

#include "securityeap.h"

class SecurityWired8021x::Private
{
public:
    QCheckBox * box;
    SecurityEap * securityEap;
    Knm::Security8021xSetting * setting8021x;
};

SecurityWired8021x::SecurityWired8021x(Knm::Connection * connection, QWidget * parent)
    : SettingWidget(connection, parent), d_ptr(new SecurityWired8021x::Private)
{
    setWindowTitle(i18nc("@title:tab wired 802.1x security","802.1x Security"));
    QVBoxLayout * layout = new QVBoxLayout(this);
    setLayout(layout);
    d_ptr->box = new QCheckBox(i18nc("@option:check enables/disables 802.1x auth on wired ethernet", "Use &802.1x authentication"), this);
    layout->addWidget(d_ptr->box);
    d_ptr->securityEap = new SecurityEap(connection, this);
    d_ptr->securityEap->setEnabled(false);
    layout->addWidget(d_ptr->securityEap);

    connect(d_ptr->box, SIGNAL(toggled(bool)), d_ptr->securityEap, SLOT(setEnabled(bool)));

    d_ptr->setting8021x = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));
}

SecurityWired8021x::~SecurityWired8021x()
{
    delete d_ptr;
}

void SecurityWired8021x::readConfig()
{
    if (d_ptr->setting8021x->enabled()) {
        d_ptr->box->setChecked(true);
        d_ptr->securityEap->readConfig();
    } else {
        d_ptr->box->setChecked(false);
    }
}

void SecurityWired8021x::writeConfig()
{
    d_ptr->setting8021x->setEnabled(d_ptr->securityEap->isEnabled());
    d_ptr->securityEap->writeConfig();
}

void SecurityWired8021x::readSecrets()
{
    if (d_ptr->securityEap->isEnabled()) {
        d_ptr->securityEap->readSecrets();
    }
}

void SecurityWired8021x::validate()
{

}
// vim: sw=4 sts=4 et tw=100
