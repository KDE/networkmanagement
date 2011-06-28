/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

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

#ifndef OPENCONNECTAUTH_H
#define OPENCONNECTAUTH_H

#include "settingwidget.h"

#include "settings/vpn.h"

#include <QString>

namespace Knm
{
    class Connection;
} // namespace Knm
class QLayout;
struct openconnect_info;
struct oc_auth_form;
struct x509_st;

class OpenconnectAuthWidgetPrivate;

class OpenconnectAuthWidget : public SettingWidget
{
Q_OBJECT
Q_DECLARE_PRIVATE(OpenconnectAuthWidget)
public:
    OpenconnectAuthWidget(Knm::Connection *, QWidget * parent = 0);
    ~OpenconnectAuthWidget();
    void readSecrets();
    void readConfig();
    void writeConfig();

protected Q_SLOTS:
    void validate() {}

private:
    OpenconnectAuthWidgetPrivate * d_ptr;
    void acceptDialog();
    void addFormInfo(const QString &, const QString &);
    void deleteAllFromLayout(QLayout *);

private Q_SLOTS:
    void writeNewConfig(const QString &);
    void validatePeerCert(const QString &, const QString &, const QString &, bool*);
    void processAuthForm(struct oc_auth_form *);
    void updateLog(const QString &, const int &);
    void logLevelChanged(int);
    void formLoginClicked();
    void workerFinished(const int&);
    void viewServerLogToggled(bool);
    void passwordModeToggled(bool);
    void connectHost();
};

#endif // OPENCONNECTAUTH_H
