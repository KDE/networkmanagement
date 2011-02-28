/*
Copyright 2008,2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef KNM_EVENTS_H
#define KNM_EVENTS_H

#include <QString>

namespace Event
{
extern const QString HwAdded;
extern const QString HwRemoved;
extern const QString NetworkAppeared;
extern const QString NetworkDisappeared;
extern const QString Connecting;
extern const QString Disconnected;
extern const QString Connected;
extern const QString ConnectFailed;
extern const QString RfOff;
extern const QString RfOn;
extern const QString LowSignal;
extern const QString NetworkingDisabled;
extern const QString InterfaceStateChange;
extern const QString AlreadyRunning;
} // namespace Event
#endif

