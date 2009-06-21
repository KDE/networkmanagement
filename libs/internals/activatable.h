/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef KNM_INTERNALS_ACTIVATABLE_H
#define KNM_INTERNALS_ACTIVATABLE_H

#include "knm_export.h"

#include <QtCore/QObject>
#include <QMetaType>

namespace Knm {

class KNM_EXPORT Activatable : public QObject
{
Q_OBJECT
public:
    enum ActivatableType {
        Connection,
        WirelessConnection,
        WirelessNetworkItem
    };

    virtual ~Activatable();

    ActivatableType activatableType() const;
    void setDeviceUni(const QString& deviceUni);
    QString deviceUni() const;
public Q_SLOTS:
    virtual void activate() = 0;
Q_SIGNALS:
    void activated();
    void changed();
protected:
    Activatable(ActivatableType type, const QString &deviceUni, QObject * parent);

private:
    ActivatableType m_type;
    QString m_deviceUni;
};

} // namespace

#endif
