/*
    Copyright (c) 2013 Lukas Tinkl <ltinkl@redhat.com>

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

#include "security802-1x.h"
#include "ui_802-1x.h"

Security8021x::Security8021x(NetworkManager::Settings::Security8021xSetting *setting, QWidget *parent) :
    QWidget(parent),
    m_setting(setting),
    m_ui(new Ui::Security8021x)
{
    m_ui->setupUi(this);

    connect(m_ui->cbShowMd5Password, SIGNAL(toggled(bool)), SLOT(setShowMD5Password(bool)));
    connect(m_ui->cbShowTlsPassword, SIGNAL(toggled(bool)), SLOT(setShowTlsPrivateKeyPassword(bool)));
    connect(m_ui->cbShowLeapPassword, SIGNAL(toggled(bool)), SLOT(setShowLeapPassword(bool)));
    connect(m_ui->cbShowFastPassword, SIGNAL(toggled(bool)), SLOT(setShowFastPassword(bool)));
    connect(m_ui->cbShowTtlsPassword, SIGNAL(toggled(bool)), SLOT(setShowTtlsPassword(bool)));
    connect(m_ui->cbShowPeapPassword, SIGNAL(toggled(bool)), SLOT(setShowPeapPassword(bool)));
}

Security8021x::~Security8021x()
{
}

void Security8021x::setShowMD5Password(bool on)
{
    m_ui->md5Password->setPasswordMode(!on);
}

void Security8021x::setShowTlsPrivateKeyPassword(bool on)
{
    m_ui->tlsPrivateKeyPassword->setPasswordMode(!on);
}

void Security8021x::setShowLeapPassword(bool on)
{
    m_ui->leapPassword->setPasswordMode(!on);
}

void Security8021x::setShowFastPassword(bool on)
{
    m_ui->fastPassword->setPasswordMode(!on);
}

void Security8021x::setShowTtlsPassword(bool on)
{
    m_ui->ttlsPassword->setPasswordMode(!on);
}

void Security8021x::setShowPeapPassword(bool on)
{
    m_ui->peapPassword->setPasswordMode(!on);
}
