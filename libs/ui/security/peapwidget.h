/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#ifndef PEAPWIDGET_H
#define PEAPWIDGET_H

#include "eapmethodinnerauth.h"
#include "ui_eapmethodpeapbase.h"

class PeapWidgetPrivate;

class PeapWidget : public EapMethodInnerAuth, public Ui_EapMethodPeapBase
{
Q_OBJECT
Q_DECLARE_PRIVATE(PeapWidget)
public:
    PeapWidget(Knm::Connection* connection, QWidget * parent = 0 );
    virtual ~PeapWidget();

    // implementation of EapWidget methods
    bool validate() const;
    void readConfig();
    void writeConfig();
    void readSecrets();
    void syncWidgetData(const QPair<QString, QString> &widgetData);
    QPair<QString, QString> widgetData();
private Q_SLOTS:
    void emitValid();
    void showAltSubjectMatchesEditor();
    void showServersEditor();
};

#endif
