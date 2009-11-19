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


#ifndef SETTINGWIDGET_H
#define SETTINGWIDGET_H

#include <QWidget>

#include "knm_export.h"

namespace Knm
{
    class Connection;
} // namespace Knm

class SettingWidgetPrivate;

class KNM_EXPORT SettingWidget : public QWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(SettingWidget)
public:
    SettingWidget(QWidget * parent = 0);
    SettingWidget(Knm::Connection * connection, QWidget * parent = 0);
    virtual ~SettingWidget();

    void setConnection(Knm::Connection *);
    Knm::Connection * connection() const;
    /**
     * populate the UI from the Connection
     */
    virtual void readConfig() = 0;
    /**
     * set the Connection from the UI
     */
    virtual void writeConfig() = 0;
    /**
     * Populate the UI with any secrets from the Setting.
     * Separate from readConfig because secrets are fetched
     * asynchronously.
     */
    virtual void readSecrets();
    /** 
     * Check that the settings in this widget are valid
     */
    bool isValid() const;
signals:
    void valid(bool);
protected Q_SLOTS:
    virtual void validate() = 0;
protected:
    void setValid(bool);
    SettingWidget(SettingWidgetPrivate &dd, QWidget * parent = 0);
    SettingWidget(SettingWidgetPrivate &dd, Knm::Connection * connection, QWidget * parent = 0);
    SettingWidgetPrivate * d_ptr;
};

#endif // SETTINGWIDGET_H

