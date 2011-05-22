// This file is generated by kconfig_compiler from gsm.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_GSMSETTING_H
#define KNM_GSMSETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knminternals_export.h"
namespace Knm {

class KNMINTERNALS_EXPORT GsmSetting : public Setting
{
  public:

    GsmSetting( );
    ~GsmSetting();

    QString name() const;

    bool hasSecrets() const;

    void setSecrets(Setting::secretsTypes);

    /**
      Set Number
    */
    void setNumber( const QString & v )
    {
        mNumber = v;
    }

    /**
      Get Number
    */
    QString number() const
    {
      return mNumber;
    }

    /**
      Set Username
    */
    void setUsername( const QString & v )
    {
        mUsername = v;
    }

    /**
      Get Username
    */
    QString username() const
    {
      return mUsername;
    }

    /**
      Set Password
    */
    void setPassword( const QString & v )
    {
        mPassword = v;
    }

    /**
      Get Password
    */
    QString password() const
    {
      return mPassword;
    }

    /**
     * Set Password flags
     */
    void setPasswordflags( Setting::secretsTypes types )
    {
        mPasswordflags = types;
    }

    /**
     * Get Password flags
     */
    Setting::secretsTypes passwordflags()
    {
        return mPasswordflags;
    }

    /**
      Set APN
    */
    void setApn( const QString & v )
    {
        mApn = v;
    }

    /**
      Get APN
    */
    QString apn() const
    {
      return mApn;
    }

    /**
      Set Network ID
    */
    void setNetworkid( const QString & v )
    {
        /* TODO: change this checks to verify if the colon separated list of mmc-mnc in v is valid.
         * Since NetworkManager-0.8 does not use this list yet we do not need to hurry.*/
        if (v.length() == 5 || v.length() == 6) {
            mNetworkid = v;
        }
        mNetworkid = v;
    }

    /**
      Get Network ID
    */
    QString networkid() const
    {
      return mNetworkid;
    }

    /**
      Set Network Type
    */
    void setNetworktype( int v )
    {
        mNetworktype = v;
    }

    /**
      Get Network Type
    */
    int networktype() const
    {
      return mNetworktype;
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
      Set PIN
    */
    void setPin( const QString & v )
    {
        mPin = v;
    }

    /**
      Get PIN
    */
    QString pin() const
    {
      return mPin;
    }

    /**
     * Set PIN flags
     */
    void setPinflags( Setting::secretsTypes types )
    {
        mPinflags = types;
    }

    /**
     * Get PIN flags
     */
    Setting::secretsTypes pinflags()
    {
        return mPinflags;
    }

  protected:

    // gsm
    QString mNumber;
    QString mUsername;
    QString mPassword;
    QString mApn;
    QString mNetworkid;
    int mNetworktype;
    int mBand;
    QString mPin;
    QString mPuk;
    Setting::secretsTypes mPasswordflags;
    Setting::secretsTypes mPinflags;

  private:
};

}

#endif

