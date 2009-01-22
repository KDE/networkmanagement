/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>

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


#ifndef SETTINGINTERFACE_H
#define SETTINGINTERFACE_H

#include <QVariant>

#include "knm_export.h"

class ConfigXml;
class KConfig;

namespace Knm
{
    class Connection;
} // namespace Knm

class KNM_EXPORT SettingInterface
{
public:
    SettingInterface(Knm::Connection *);
    virtual ~SettingInterface();
    /**
     * populate the UI from the Connection
     */
    virtual void readConfig();
    /**
     * set the Connection from the UI
     */
    virtual void writeConfig();
    /** 
     * Check that the settings in this widget are valid
     */
    virtual bool validate() const;
    /**
     * Return the appropriate widget
     */
    virtual QWidget* widget() = 0;

private:
    class Private;
    Private * d;
};

#endif // SETTINGINTERFACE_H

