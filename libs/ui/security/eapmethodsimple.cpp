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

#include "eapmethodsimple.h"

#include "eapmethod_p.h"

#include <settings/802-1x.h>
EapMethodSimple::EapMethodSimple(Type type, Knm::Connection * connection, QWidget * parent)
    : EapMethod(connection, parent), m_type(type)
{
    setupUi(this);
}

EapMethodSimple::~EapMethodSimple()
{

}

void EapMethodSimple::setShowPasswords(bool on)
{
    lePassword->setPasswordMode(!on);
}

bool EapMethodSimple::validate() const
{
    return !(leUserName->text().isEmpty() || lePassword->text().isEmpty());
}

void EapMethodSimple::readConfig()
{
    Q_D(EapMethod);
    leUserName->setText(d->setting->identity());
}

void EapMethodSimple::writeConfig()
{
    Q_D(EapMethod);
    switch (m_type) {
        case Pap:
            d->setting->setPhase2auth(Knm::Security8021xSetting::EnumPhase2auth::pap);
            break;
        case MsChap:
            d->setting->setPhase2auth(Knm::Security8021xSetting::EnumPhase2auth::mschap);
            break;
        case MsChapV2:
            d->setting->setPhase2auth(Knm::Security8021xSetting::EnumPhase2auth::mschapv2);
            break;
        case MD5:
            d->setting->setPhase2auth(Knm::Security8021xSetting::EnumPhase2auth::md5);
            break;
        case Chap:
            d->setting->setPhase2auth(Knm::Security8021xSetting::EnumPhase2auth::chap);
            break;
    }
    d->setting->setIdentity(leUserName->text());
    d->setting->setPassword(lePassword->text());
}

void EapMethodSimple::readSecrets()
{
    Q_D(EapMethod);
    lePassword->setText(d->setting->password());
}

// vim: sw=4 sts=4 et tw=100
