/*
Copyright 2009 Andrey Batyiev <batyiev@gmail.com>

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
#include "listvalidator.h"

#include <KDebug>

#include <QStringList>

class ListValidator::Private
{
public:
    Private() : inner(0)
    {
    }

    QValidator *inner;
};

ListValidator::ListValidator(QObject *parent)
 : QValidator(parent), d(new ListValidator::Private)
{
}

ListValidator::~ListValidator()
{
}

QValidator::State ListValidator::validate(QString &text, int &pos) const
{
    QStringList strings = text.split(',');
    int trash;
    QValidator::State state = Acceptable;
    foreach (QString string, strings) {
        QValidator::State current = d->inner->validate(string, trash);
        if (current == Invalid)
            return Invalid;
        if (current == Intermediate)
            state = Intermediate;
    }
    return state;
}

void ListValidator::setInnerValidator(QValidator *validator)
{
    d->inner = validator; 
}

const QValidator* ListValidator::innerValidator() const
{
    return d->inner;
}

