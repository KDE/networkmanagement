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

#ifndef WIRELESSITEM_H
#define WIRELESSITEM_H

#include <QString>
#include <solid/control/wirelessaccesspoint.h>

#include "knm_export.h"

namespace Knm
{
class KNM_EXPORT WirelessItem
{
public:
    WirelessItem(const QString & ssid, int strength, Solid::Control::AccessPoint::WpaFlags wpaFlags, Solid::Control::AccessPoint::WpaFlags rsnFlags);
    virtual ~WirelessItem();
    QString ssid() const;
    int strength() const;
    virtual void setStrength(int strength);
    Solid::Control::AccessPoint::WpaFlags wpaFlags() const;
    Solid::Control::AccessPoint::WpaFlags rsnFlags() const;
protected:
    QString m_ssid;
    int m_strength;
    Solid::Control::AccessPoint::WpaFlags m_wpaFlags;
    Solid::Control::AccessPoint::WpaFlags m_rsnFlags;
};

} // namespace Knm

#endif // WIRELESSITEM_H
