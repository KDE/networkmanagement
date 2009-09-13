/*
    Copyright (C) 2009 Dario Freddi <drf@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef WICDCONNECTIONHANDLER_H
#define WICDCONNECTIONHANDLER_H

#include "activatableobserver.h"
#include "connectionhandler.h"


class WicdConnectionHandler : public ActivatableObserver, public ConnectionHandler
{
  public:
    WicdConnectionHandler();
    
    virtual void handleRemove(Knm::Activatable* );
    virtual void handleUpdate(Knm::Activatable* );
    virtual void handleAdd(Knm::Activatable* );
    virtual void handleRemove(Knm::Connection* );
    virtual void handleUpdate(Knm::Connection* );
    virtual void handleAdd(Knm::Connection* );
};

#endif // WICDCONNECTIONHANDLER_H
