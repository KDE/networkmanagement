// This file is generated by kconfig_compiler from 802-11-wireless.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_WIRELESSSETTING_H
#define KNM_WIRELESSSETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knm_export.h"
namespace Knm {

class KNM_EXPORT WirelessSetting : public Setting
{
  public:
    class EnumMode
    {
      public:
      enum type { infrastructure, adhoc, COUNT };
    };
    class EnumBand
    {
      public:
      enum type { a, bg, COUNT };
    };

    WirelessSetting( );
    ~WirelessSetting();

    QString name() const;

    bool hasSecrets() const;

    /**
      Set SSID
    */
    void setSsid( const QByteArray & v )
    {
        mSsid = v;
    }

    /**
      Get SSID
    */
    QByteArray ssid() const
    {
      return mSsid;
    }

    /**
      Set Mode
    */
    void setMode( int v )
    {
        mMode = v;
    }

    /**
      Get Mode
    */
    int mode() const
    {
      return mMode;
    }

    /**
      Set Band
    */
    void setBand( int v )
    {
        mBand = v;
    }

    /**
      Get Band
    */
    int band() const
    {
      return mBand;
    }

    /**
      Set Channel
    */
    void setChannel( uint v )
    {
        mChannel = v;
    }

    /**
      Get Channel
    */
    uint channel() const
    {
      return mChannel;
    }

    /**
      Set BSSID
    */
    void setBssid( const QByteArray & v )
    {
        mBssid = v;
    }

    /**
      Get BSSID
    */
    QByteArray bssid() const
    {
      return mBssid;
    }

    /**
      Set Data rate
    */
    void setRate( uint v )
    {
        mRate = v;
    }

    /**
      Get Data rate
    */
    uint rate() const
    {
      return mRate;
    }

    /**
      Set Transmit power
    */
    void setTxpower( uint v )
    {
        mTxpower = v;
    }

    /**
      Get Transmit power
    */
    uint txpower() const
    {
      return mTxpower;
    }

    /**
      Set MAC Address
    */
    void setMacaddress( const QByteArray & v )
    {
        mMacaddress = v;
    }

    /**
      Get MAC Address
    */
    QByteArray macaddress() const
    {
      return mMacaddress;
    }

    /**
      Set MTU
    */
    void setMtu( uint v )
    {
        mMtu = v;
    }

    /**
      Get MTU
    */
    uint mtu() const
    {
      return mMtu;
    }

    /**
      Set Seen BSSIDs
    */
    void setSeenbssids( const QStringList & v )
    {
        mSeenbssids = v;
    }

    /**
      Get Seen BSSIDs
    */
    QStringList seenbssids() const
    {
      return mSeenbssids;
    }

    /**
      Set security
    */
    void setSecurity( const QString & v )
    {
        mSecurity = v;
    }

    /**
      Get security
    */
    QString security() const
    {
      return mSecurity;
    }

  protected:

    // 802-11-wireless
    QByteArray mSsid;
    int mMode;
    int mBand;
    uint mChannel;
    QByteArray mBssid;
    uint mRate;
    uint mTxpower;
    QByteArray mMacaddress;
    uint mMtu;
    QStringList mSeenbssids;
    QString mSecurity;

  private:
};

}

#endif

