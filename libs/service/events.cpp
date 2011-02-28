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

#include "events.h"

const QString Event::HwAdded = QLatin1String("hwadded");
const QString Event::HwRemoved = QLatin1String("hwremoved");
const QString Event::NetworkAppeared = QLatin1String("networkappeared");
const QString Event::NetworkDisappeared = QLatin1String("networkdisappeared");
const QString Event::Connecting = QLatin1String("connecting");
const QString Event::Disconnected = QLatin1String("disconnected");
const QString Event::Connected = QLatin1String("connected");
const QString Event::ConnectFailed = QLatin1String("connfailed"); // TODO: implement
const QString Event::RfOff = QLatin1String("rfoff");
const QString Event::RfOn = QLatin1String("rfon");
const QString Event::LowSignal = QLatin1String("lowsignal"); // TODO: implement
const QString Event::NetworkingDisabled = QLatin1String("networkingdisabled");
const QString Event::InterfaceStateChange = QLatin1String("ifacestatechange");
const QString Event::AlreadyRunning = QLatin1String("alreadyrunning");
