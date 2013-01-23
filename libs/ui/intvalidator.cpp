/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#include <KDebug>
#include <QStringList>

#include "intvalidator.h"

IntValidator::IntValidator(QObject *parent)
 : QValidator(parent), m_boundary(false)
{
    m_signed = false;
}

IntValidator::IntValidator(bool isSigned, QObject *parent)
:QValidator(parent), m_signed(isSigned), m_boundary(false)
{
}

IntValidator::IntValidator(int min, int max, QObject *parent)
:QValidator(parent), m_min(min), m_max(max), m_boundary(true)
{
}

IntValidator::~IntValidator()
{
}

QValidator::State IntValidator::validate(QString &value, int &pos) const
{
    if (m_boundary)
    {
        int num = value.toInt();
        if (num < m_min || num > m_max)
            return QValidator::Invalid;
    }
    QRegExpValidator *v;
    switch (m_signed)
    {
        case false:
            v = new QRegExpValidator(QRegExp("^[0-9]*$"), 0);
            break;
        case true:
        default:
            v = new QRegExpValidator(QRegExp("^-?[0-9]*$"), 0);
            break;
    }

    return v->validate(value,pos);
}
