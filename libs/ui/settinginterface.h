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

class ConfigXml;
class KConfig;

#include "knm_export.h"

class KNM_EXPORT SettingInterface
{
public:
    SettingInterface(const QString & connectionId);
    virtual ~SettingInterface();
    /**
     * read in any configuration that ConfigXml can't handle
     */
    virtual void readConfig();
    /**
     * write any configuration that ConfigXml can't handle
     */
    virtual void writeConfig();
    /**
     * read the secrets out of the UI
     */
    virtual QVariantMap secrets() const;
    /**
     * get the name of the settings group configured by the widget
     */
    virtual QString settingName() const = 0;
    /**
     * The configuration management object belonging to this widget
     */
    ConfigXml * configXml() const;
    /** 
     * Check that the settings in this widget are valid
     */
    virtual bool validate() const;
    /**
     * Return the appropriate widget
     */
    virtual QWidget* widget() = 0;

protected:
    /**
     * Setup ConfigXml for this widget
     * Separate from ctor since it depends upon settingsName from concrete subclasses
     */
    void init();
    QString connectionId() const;
private:
    class Private;
    Private * d;
};

#endif // SETTINGINTERFACE_H

