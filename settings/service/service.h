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

#ifndef KNM07_KDED_SERVICE_H
#define KNM07_KDED_SERVICE_H

#include <QVariant>

#include <KDEDModule>

class NetworkSettings;

class KNetworkManagerService : public KDEDModule {
Q_OBJECT
public:
    KNetworkManagerService(QObject * parent, const QVariantList&);
    virtual ~KNetworkManagerService();
    int status( const QString & network );
    static const QString SERVICE_USER_SETTINGS;
private Q_SLOTS:
    void serviceOwnerChanged( const QString& service,const QString& oldOwner, const QString& newOwner );
    void serviceRegistered(const QString&);
    void serviceUnregistered(const QString&);
private:
    void registerService();
    NetworkSettings * mNetworkSettings;
    bool m_active;
};

#endif
