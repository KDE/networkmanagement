/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#ifndef LEAPAUTHWIDGET_H
#define LEAPAUTHWIDGET_H

#include "securitywidget.h"

namespace Knm
{
    class Connection;
} // namespace Knm
class QFormLayout;

class KNM_EXPORT LeapAuthWidget : public SecurityWidget
{
Q_OBJECT
public:
    LeapAuthWidget(Knm::Connection* connection, QWidget * parent = 0);
    LeapAuthWidget(Knm::Connection* connection, QFormLayout *layout, QWidget * parent = 0);
    virtual ~LeapAuthWidget();
    bool validate() const;
    void readConfig() {}
    void writeConfig();
    void readSecrets();
public Q_SLOTS:
    virtual void setShowPasswords(bool);
private:
    class Private;
    Private * d;
    void setupUi();
};

#endif
