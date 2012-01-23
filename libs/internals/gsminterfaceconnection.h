/*
Copyright 2008 Frederik Gladhorn <gladhorn@kde.org>
Copyright 2009 Will Stephenson <wstephenson@kde.org>
Copyright 2010-2012 Lamarque Souza <lamarque@kde.org>

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

#ifndef KNM_EXTERNALS_GSMINTERFACECONNECTION_H
#define KNM_EXTERNALS_GSMINTERFACECONNECTION_H

#include <solid/control/networkmodeminterface.h>

#include "interfaceconnection.h"

#include "knminternals_export.h"

namespace Knm {

class KNMINTERNALS_EXPORT GsmInterfaceConnection : public InterfaceConnection
{
Q_OBJECT
Q_PROPERTY(int quality READ getSignalQuality)
Q_PROPERTY(int tech READ getAccessTechnology)

public:
    friend class GsmInterfaceConnectionBuilder;

    // To create an object of GsmInterfaceConnection class please use
    // Knm::GsmInterfaceConnectionHelpers::build(..) function.
    virtual ~GsmInterfaceConnection();
    int getSignalQuality() const;
    int getAccessTechnology() const;
    QVariantMap toMap();
Q_SIGNALS:
    void signalQualityChanged(int);
    void accessTechnologyChanged(const int);
    void enabledChanged(const bool enabled);
    void gsmPropertiesChanged(const QVariantMap & properties);
public Q_SLOTS:
    void setSignalQuality(uint signalQuality);
    void setAccessTechnology(const Solid::Control::ModemInterface::AccessTechnology accessTechnology);
    void setEnabled(const bool enabled);
    void connectMMSignals();
protected:
    GsmInterfaceConnection(ActivatableType type, const QString & deviceUni, QObject * parent);
    int m_signalQuality;
    int m_accessTechnology;
    bool m_enabled;
};
} // namespace

#endif
