/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>

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

#ifndef CONNECTABLE_H
#define CONNECTABLE_H

#include <QtCore/QObject>
#include <QMetaType>

namespace Knm {

class Connectable :public QObject
{
    Q_OBJECT
public:
    Connectable();
    
    virtual ~Connectable() {}

    void setConnectionType(const QString& type);
    QString connectionType();
    void setDeviceUni(const QString& deviceUni);
    QString deviceUni();

private:
    QString m_type;
    QString m_deviceUni;

};
} // namespace


#endif
