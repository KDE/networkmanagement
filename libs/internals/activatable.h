/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2012 Lamarque V. Souza <lamarque@kde.org>

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

#ifndef KNM_INTERNALS_ACTIVATABLE_H
#define KNM_INTERNALS_ACTIVATABLE_H

#include "knminternals_export.h"

#include <QtCore/QObject>
#include <QMetaType>
#include <QVariantMap>

namespace Knm {

class KNMINTERNALS_EXPORT Activatable : public QObject
{
Q_OBJECT
Q_PROPERTY(QString deviceUni READ deviceUni)
Q_PROPERTY(QString type READ activatableType)
public:
    /* Keep this list sorted by priority:
     * the first types will appear first in
     * connection list widget.
     */
    enum ActivatableType {
        WirelessInterfaceConnection,
        VpnInterfaceConnection,
        GsmInterfaceConnection,
        InterfaceConnection,
        WirelessNetwork,
        HiddenWirelessInterfaceConnection,
        UnconfiguredInterface
    };

    virtual ~Activatable();

    ActivatableType activatableType() const;
    void setDeviceUni(const QString& deviceUni);
    QString deviceUni() const;
    bool isShared() const;
    void setShared(const bool);
    virtual QVariantMap toMap();
public Q_SLOTS:
    void activate();
Q_SIGNALS:
    void activated();
    void changed();
    void propertiesChanged(const QVariantMap & properties);
protected:
    Activatable(ActivatableType type, const QString &deviceUni, QObject * parent);

private:
    ActivatableType m_type;
    QString m_deviceUni;
    bool m_shared;
};

} // namespace

Q_DECLARE_METATYPE(Knm::Activatable*)

#endif
