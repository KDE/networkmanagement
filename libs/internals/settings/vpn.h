// This file is generated by kconfig_compiler from vpn.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_VPNSETTING_H
#define KNM_VPNSETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knminternals_export.h"
#include "../types.h"

#define NM_VPN_PW_TYPE_SAVE   "save"
#define NM_VPN_PW_TYPE_ASK    "ask"
#define NM_VPN_PW_TYPE_UNUSED "unused"

namespace Knm {

class KNMINTERNALS_EXPORT VpnSetting : public Setting
{
  public:

    VpnSetting( );
    VpnSetting(VpnSetting *);
    ~VpnSetting();

    QString name() const;

    bool hasSecrets() const;

    /**
      Set Service Type
    */
    void setServiceType( const QString & v )
    {
        mServiceType = v;
    }

    /**
      Get Service Type
    */
    QString serviceType() const
    {
      return mServiceType;
    }

    /**
      Set Data bits
    */
    void setData( const QStringMap & v )
    {
        mData = v;
    }

    /**
      Get Data bits
    */
    QStringMap data() const
    {
      QStringMap r = mData;
      // Add secrets flags.
      foreach (QString key, mSecretsStorageType.keys()) {
          r.insert(key + "-flags", QString::number(storageTypeToSecretsType(mSecretsStorageType.value(key))));

          // TODO: remove this when all vpnplugins/*/*widget.* are converted to use Settings::secretsTypes.
          r.insert(key.replace(' ', "-").toLower() + "-type", mSecretsStorageType.value(key));
      }

      return r;
    }

    static secretsType storageTypeToSecretsType(const QString & storageType)
    {
      if (storageType == NM_VPN_PW_TYPE_SAVE) {
        return None;
      }
      if (storageType == NM_VPN_PW_TYPE_ASK) {
          return NotSaved;
      }
      if (storageType == NM_VPN_PW_TYPE_UNUSED) {
          return NotRequired;
      }
      return AgentOwned;
    }

    /**
      Set Username
    */
    void setUserName( const QString & v )
    {
        mUserName = v;
    }

    /**
      Get Username
    */
    QString userName() const
    {
      return mUserName;
    }

    /**
      Set Vpnsecrets
    */
    void setVpnSecrets( const QStringMap & v )
    {
        mVpnSecrets = v;
    }

    /**
      Get Vpnsecrets
    */
    QStringMap vpnSecrets() const
    {
      return mVpnSecrets;
    }

    /**
      Set VPN Plugin Name
    */
    void setPluginName( const QString & v )
    {
        mPluginName = v;
    }

    /**
      Get VPN Plugin Name
    */
    QString pluginName() const
    {
      return mPluginName;
    }

    /**
      Set secret storage type
    */
    void setSecretsStorageType( const QStringMap & v )
    {
      mSecretsStorageType = v;
    }

    /**
      Get secret storage type
    */
    QStringMap secretsStorageType() const
    {
      return mSecretsStorageType;
    }

    bool hasVolatileSecrets() const;

  protected:

    // vpn
    QString mServiceType;
    QStringMap mData;
    QString mUserName;
    QStringMap mVpnSecrets;
    QString mPluginName;
    QStringMap mSecretsStorageType;

  private:
};

}

#endif

