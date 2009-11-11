/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    Based on code from Dan Williams <dcbw@redhat.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#include <QByteArray>
#include <QCryptographicHash>

#include "wephash.h"

QByteArray wep128PassphraseHash(QByteArray input)
{
    QByteArray md5_data, digest;

    if (input.isEmpty()) {
        return QByteArray();
    }

    md5_data.reserve(65);
    digest.reserve(16);

    // Get at least 64 bytes
    for (int i = 0; i < 64; i++)
        md5_data[i] = input[i % input.length()];

    digest = QCryptographicHash::hash(md5_data, QCryptographicHash::Md5);
    //convert to hex and only pass the first 26 chars for 128-bit encryption
    return digest.toHex().left(26);
}
