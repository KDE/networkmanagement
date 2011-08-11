/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

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

#ifndef SECURITYWIDGET_H
#define SECURITYWIDGET_H

#include "settingwidget.h"

#include "knm_export.h"

class SecurityWidgetPrivate;

class KNM_EXPORT SecurityWidget : public QWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(SecurityWidget)
public:
    SecurityWidget(Knm::Connection* connection, QWidget * parent = 0 );
    virtual ~SecurityWidget();
    virtual void readConfig() = 0;
    virtual void writeConfig() = 0;
    virtual void readSecrets() = 0;
    virtual bool validate() const = 0;
/**
 * Emit this signal upon security validations. It is connected to the slot
 * ConnectionPreferences::valid, which will enable/disable OK button
 * of connection editor window
 */
Q_SIGNALS:
    void valid(bool);
public Q_SLOTS:
    virtual void setShowPasswords(bool) {}
protected:
    SecurityWidget(SecurityWidgetPrivate &dd, Knm::Connection* connection, QWidget * parent = 0 );
    SecurityWidgetPrivate * d_ptr;
};

#endif
