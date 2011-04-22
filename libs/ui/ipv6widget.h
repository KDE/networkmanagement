/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>, based on work by Will Stephenson <wstephenson@kde.org>

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

#ifndef IPV6WIDGET_H
#define IPV6WIDGET_H

#include "settingwidget.h"

#include "knm_export.h"

class IpV6WidgetPrivate;
class KNM_EXPORT IpV6Widget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(IpV6Widget)
public:
    IpV6Widget(Knm::Connection * connection, QWidget * parent = 0 );
    virtual ~IpV6Widget();
    void readConfig();
    void writeConfig();
    void setDns(const QList<QVariant> dnsList);
protected Q_SLOTS:
    /**
     * Update state of widgets dependent on IP setting method
     */
    void methodChanged(int);
    void addressEditingFinished();
    void dnsEdited(QStringList addresses);
    void dnsSearchEdited(QStringList addresses);
    void showDnsEditor();
    void showDnsSearchEditor();
    void validate();
};

#endif
