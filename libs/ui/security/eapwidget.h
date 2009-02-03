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

#ifndef EAPWIDGET_H
#define EAPWIDGET_H

#include <QWidget>
#include <QVariant>

namespace Knm
{
    class Connection;
}

/**
 * Abstract base class for EAP configuration widgets
 */
class EapWidget : public QWidget
{
Q_OBJECT
public:
    EapWidget(Knm::Connection* connection, QWidget * parent = 0 );
    virtual ~EapWidget();

    virtual void readConfig() = 0;
    virtual void writeConfig() = 0;
    virtual void readSecrets() = 0;
    virtual bool validate() const = 0;

protected:
    Knm::Connection* m_connection;
};

#endif
