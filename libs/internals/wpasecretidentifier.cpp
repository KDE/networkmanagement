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

#include "wpasecretidentifier.h"

WpaSecretIdentifier::WpaSecretType WpaSecretIdentifier::identify(const QString & secret)
{
    bool secretIsPsk = true;
    bool secretIsPassphrase = true;
    QByteArray secretBytes = secret.toAscii();
    for (int i = 0; i < secretBytes.size(); ++i) {
        char current = secretBytes.at(i);
        if (!(current >= (char)0x20 && current <= (char)0x7e)) {
            secretIsPassphrase = false;
        }
        if (! (( current >= (char)0x30 && current <= (char)0x39)
                || ( current >= (char)0x41 && current <= (char)0x46)
                || ( current >= (char)0x61 && current <= (char)0x66))) {
            secretIsPsk = false;
        }
    }
    if (secretBytes.size() < 8 || secretBytes.size() > 63) {
        secretIsPassphrase = false;
    }
    if (secretBytes.size() != 64) {
        secretIsPsk = false;
    }

    WpaSecretType type = Invalid;

    if (secretIsPsk) {
        type = PreSharedKey;
    } else if (secretIsPassphrase) {
        type = Passphrase;
    }

    return type;
}

// vim: sw=4 sts=4 et tw=100
