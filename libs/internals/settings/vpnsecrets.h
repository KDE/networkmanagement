/*
Copyright 2011 Lamarque Souza <lamarque@gmail.com>

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

#ifndef KNM_VPNSECRETS_H
#define KNM_VPNSECRETS_H

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "knminternals_export.h"

#include "../types.h"
#include "secrets.h"

namespace Knm {

class VpnSetting;

class KNMINTERNALS_EXPORT VpnSecrets : public Secrets
{
  public:
    VpnSecrets(VpnSetting * setting);
    VpnSecrets(VpnSecrets * secrets, VpnSetting * setting);
    ~VpnSecrets();
    QMap<QString,QString> secretsToMap() const;
    void secretsFromMap(QMap<QString,QString>) const;
    static QStringList variantMapToStringList(const QVariantMap &);
    static QVariantMap variantMapFromStringList(const QStringList & list);
    static QStringMap stringMapFromStringList(const QStringList & list);
    static QStringList stringMapToStringList(const QStringMap & map);
    void secretsToConfig(QMap<QString,QString> secrets, KSharedConfig::Ptr configptr) const;
    QMap<QString,QString> secretsFromConfig(KSharedConfig::Ptr configptr);
    QStringList needSecrets();

  protected:
    static QVariantMap secretsToSave(const QStringMap &, const QStringMap &);
    VpnSetting * m_setting;
};
}

#endif

