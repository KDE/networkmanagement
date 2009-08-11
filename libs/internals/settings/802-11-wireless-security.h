// This file is generated by kconfig_compiler from 802-11-wireless-security.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_WIRELESSSECURITYSETTING_H
#define KNM_WIRELESSSECURITYSETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knminternals_export.h"
namespace Knm {

class KNMINTERNALS_EXPORT WirelessSecuritySetting : public Setting
{
  public:
    class EnumSecurityType
    {
      public:
      enum type { None, StaticWep, Leap, DynamicWep, WpaPsk, WpaEap, Wpa2Psk, Wpa2Eap, COUNT };
    };
    class EnumKeymgmt
    {
      public:
      enum type { None, Ieee8021x, WPANone, WPAPSK, WPAEAP, COUNT };
    };
    class EnumAuthalg
    {
      public:
      enum type { none, open, shared, leap, COUNT };
    };

    WirelessSecuritySetting( );
    ~WirelessSecuritySetting();

    QString name() const;

    bool hasSecrets() const;

    /**
      Set Security type
    */
    void setSecurityType( int v )
    {
        mSecurityType = v;
    }

    /**
      Get Security type
    */
    int securityType() const
    {
      return mSecurityType;
    }

    /**
      Set Key management
    */
    void setKeymgmt( int v )
    {
        mKeymgmt = v;
    }

    /**
      Get Key management
    */
    int keymgmt() const
    {
      return mKeymgmt;
    }

    /**
      Set WEP TX key index
    */
    void setWeptxkeyindex( uint v )
    {
        mWeptxkeyindex = v;
    }

    /**
      Get WEP TX key index
    */
    uint weptxkeyindex() const
    {
      return mWeptxkeyindex;
    }

    /**
      Set Authentication algorithm
    */
    void setAuthalg( int v )
    {
        mAuthalg = v;
    }

    /**
      Get Authentication algorithm
    */
    int authalg() const
    {
      return mAuthalg;
    }

    /**
      Set Protocols
    */
    void setProto( const QStringList & v )
    {
        mProto = v;
    }

    /**
      Get Protocols
    */
    QStringList proto() const
    {
      return mProto;
    }

    /**
      Set Pairwise
    */
    void setPairwise( const QStringList & v )
    {
        mPairwise = v;
    }

    /**
      Get Pairwise
    */
    QStringList pairwise() const
    {
      return mPairwise;
    }

    /**
      Set Group
    */
    void setGroup( const QStringList & v )
    {
        mGroup = v;
    }

    /**
      Get Group
    */
    QStringList group() const
    {
      return mGroup;
    }

    /**
      Set LEAP Username
    */
    void setLeapusername( const QString & v )
    {
        mLeapusername = v;
    }

    /**
      Get LEAP Username
    */
    QString leapusername() const
    {
      return mLeapusername;
    }

    /**
      Set WEP key 0
    */
    void setWepkey0( const QString & v )
    {
        mWepkey0 = v;
    }

    /**
      Get WEP key 0
    */
    QString wepkey0() const
    {
      return mWepkey0;
    }

    /**
      Set WEP key 1
    */
    void setWepkey1( const QString & v )
    {
        mWepkey1 = v;
    }

    /**
      Get WEP key 1
    */
    QString wepkey1() const
    {
      return mWepkey1;
    }

    /**
      Set WEP key 2
    */
    void setWepkey2( const QString & v )
    {
        mWepkey2 = v;
    }

    /**
      Get WEP key 2
    */
    QString wepkey2() const
    {
      return mWepkey2;
    }

    /**
      Set WEP key 3
    */
    void setWepkey3( const QString & v )
    {
        mWepkey3 = v;
    }

    /**
      Get WEP key 3
    */
    QString wepkey3() const
    {
      return mWepkey3;
    }

    /**
      Set PSK
    */
    void setPsk( const QString & v )
    {
        mPsk = v;
    }

    /**
      Get PSK
    */
    QString psk() const
    {
      return mPsk;
    }

    /**
      Set LEAP Password
    */
    void setLeappassword( const QString & v )
    {
        mLeappassword = v;
    }

    /**
      Get LEAP Password
    */
    QString leappassword() const
    {
      return mLeappassword;
    }

    /**
      Set WEP Passphrase
    */
    void setWeppassphrase( const QString & v )
    {
        mWeppassphrase = v;
    }

    /**
      Get WEP Passphrase
    */
    QString weppassphrase() const
    {
      return mWeppassphrase;
    }

  protected:

    // 802-11-wireless-security
    int mSecurityType;
    int mKeymgmt;
    uint mWeptxkeyindex;
    int mAuthalg;
    QStringList mProto;
    QStringList mPairwise;
    QStringList mGroup;
    QString mLeapusername;
    QString mWepkey0;
    QString mWepkey1;
    QString mWepkey2;
    QString mWepkey3;
    QString mPsk;
    QString mLeappassword;
    QString mWeppassphrase;

  private:
};

}

#endif

