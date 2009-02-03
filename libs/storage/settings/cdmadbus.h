// This file is generated by kconfig_compiler from cdma.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_CDMADBUS_H
#define KNM_CDMADBUS_H

#include <nm-setting-cdma.h>

#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "settingdbus.h"
#include "knm_export.h"
namespace Knm {

class CdmaSetting;

class KNM_EXPORT CdmaDbus : public SettingDbus
{
  public:
    CdmaDbus( CdmaSetting * setting);
    ~CdmaDbus();
    void fromMap(const QVariantMap&);
    QVariantMap toMap();
    QVariantMap toSecretsMap();
};
}

#endif

