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
      return mData;
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

  protected:

    // vpn
    QString mServiceType;
    QStringMap mData;
    QString mUserName;
    QStringMap mVpnSecrets;
    QString mPluginName;

  private:
};

}

#endif

