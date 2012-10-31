/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>
Copyright 2011-2012 Rajeesh K Nambiar <rajeeshknambiar@gmail.com>
Copyright 2011-2012 Lamarque V. Souza <lamarque@kde.org>

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

#ifndef KNM4_VPNC_H
#define KNM4_VPNC_H

#include "vpnuiplugin.h"

#include <QVariant>

#include <KProcess>

namespace Knm
{
    class Connection;
} // namespace Knm

class VpncUiPluginPrivate: public QObject
{
Q_OBJECT
public:
    VpncUiPluginPrivate();
    ~VpncUiPluginPrivate();
    QString readStringKeyValue(const KConfigGroup & configGroup, const QString & key);
    KProcess * ciscoDecrypt;
    QString decryptedPasswd;

protected Q_SLOTS:
    void gotciscoDecryptOutput();
    void ciscoDecryptError(QProcess::ProcessError pError);
    void ciscoDecryptFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

class VpncUiPlugin : public VpnUiPlugin
{
Q_OBJECT
public:
    VpncUiPlugin(QObject * parent = 0, const QVariantList& = QVariantList());
    virtual ~VpncUiPlugin();
    SettingWidget * widget(Knm::Connection *, QWidget * parent = 0);
    SettingWidget * askUser(Knm::Connection *, QWidget * parent = 0);
    QString suggestedFileName(Knm::Connection *connection) const;
    QString supportedFileExtensions() const;
    QVariantList importConnectionSettings(const QString &fileName);
    bool exportConnectionSettings(Knm::Connection * connection, const QString &fileName);
};

#endif //  KNM4_VPNC_H
