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

#include <QString>

#include "knm_export.h"
#include <QtGui/qwindowdefs.h> // krazy:exclude=includes (for WId)


class KNM_EXPORT SecretStorageHelper
{
public:
    SecretStorageHelper( const QString &connectionName, const QString &settingGroup);
    QString keyForEntry(const QString & entry) const;
    void readSecret(const QString &key, QString &secret);
    void writeSecret(const QString &key, const QString &secret);

    static void setWalletWid( WId wid ) {
        s_walletWId = wid;
    }

    static WId walletWid() {
        return s_walletWId;
    }
private:
    QString m_connectionName;
    QString m_settingGroup;
    static QString s_walletFolderName;
    static WId s_walletWId;
};


