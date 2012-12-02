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

#ifndef TLSWIDGET_H
#define TLSWIDGET_H

#include "eapmethod.h"
#include "ui_eapmethodtlsbase.h"

class TlsWidgetPrivate;

/**
 * TLS may be used as an outer or as an inner auth method
 * This is controlled by the isInnerMethod argument to the ctor
 * If inner, it saves its settings to the 'phase2' versions of the
 * relevant Security8021xSetting methods
 */
class TlsWidget : public EapMethod, public Ui_EapMethodTlsBase
{
Q_OBJECT
Q_DECLARE_PRIVATE(TlsWidget)
public:
    /**
     * @param is TLS being used for inner or outer auth?
     */
    TlsWidget(bool isInnerMethod, Knm::Connection* connection, QWidget * parent = 0 );
    virtual ~TlsWidget();

    bool validate() const;
    void readConfig();
    void writeConfig();
    void readSecrets();
    void syncWidgetData(const QPair<QString, QString> &widgetData);
    QPair<QString, QString> widgetData();
public Q_SLOTS:
    void setShowPasswords(bool b = true);
private Q_SLOTS:
    void privateKeyPasswordStorageChanged(int);
    void showAltSubjectMatchesEditor();
    void showServersEditor();
    void emitValid();
};

#endif
