// This file is generated by kconfig_compiler from ppp.kcfg.
// All changes you do to this file will be lost.
#ifndef PPPSETTING_H
#define PPPSETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knm_export.h"
class KNM_EXPORT PppSetting : public Setting
{
  public:

    PppSetting( );
    ~PppSetting();

    QString name() const;

    bool hasSecrets() const;

    /**
      Set No Auth
    */
    void setNoauth( bool v )
    {
        mNoauth = v;
    }

    /**
      Get No Auth
    */
    bool noauth() const
    {
      return mNoauth;
    }

    /**
      Set Refuse EAP
    */
    void setRefuseeap( bool v )
    {
        mRefuseeap = v;
    }

    /**
      Get Refuse EAP
    */
    bool refuseeap() const
    {
      return mRefuseeap;
    }

    /**
      Set Refuse PAP
    */
    void setRefusepap( bool v )
    {
        mRefusepap = v;
    }

    /**
      Get Refuse PAP
    */
    bool refusepap() const
    {
      return mRefusepap;
    }

    /**
      Set Refuse CHAP
    */
    void setRefusechap( bool v )
    {
        mRefusechap = v;
    }

    /**
      Get Refuse CHAP
    */
    bool refusechap() const
    {
      return mRefusechap;
    }

    /**
      Set Refuse MS CHAP
    */
    void setRefusemschap( bool v )
    {
        mRefusemschap = v;
    }

    /**
      Get Refuse MS CHAP
    */
    bool refusemschap() const
    {
      return mRefusemschap;
    }

    /**
      Set Refuse MS CHAP V2
    */
    void setRefusemschapv2( bool v )
    {
        mRefusemschapv2 = v;
    }

    /**
      Get Refuse MS CHAP V2
    */
    bool refusemschapv2() const
    {
      return mRefusemschapv2;
    }

    /**
      Set No BSD comp.
    */
    void setNobsdcomp( bool v )
    {
        mNobsdcomp = v;
    }

    /**
      Get No BSD comp.
    */
    bool nobsdcomp() const
    {
      return mNobsdcomp;
    }

    /**
      Set No deflate
    */
    void setNodeflate( bool v )
    {
        mNodeflate = v;
    }

    /**
      Get No deflate
    */
    bool nodeflate() const
    {
      return mNodeflate;
    }

    /**
      Set No VJ comp.
    */
    void setNovjcomp( bool v )
    {
        mNovjcomp = v;
    }

    /**
      Get No VJ comp.
    */
    bool novjcomp() const
    {
      return mNovjcomp;
    }

    /**
      Set Require MPPE
    */
    void setRequiremppe( bool v )
    {
        mRequiremppe = v;
    }

    /**
      Get Require MPPE
    */
    bool requiremppe() const
    {
      return mRequiremppe;
    }

    /**
      Set Require MPPE 128
    */
    void setRequiremppe128( bool v )
    {
        mRequiremppe128 = v;
    }

    /**
      Get Require MPPE 128
    */
    bool requiremppe128() const
    {
      return mRequiremppe128;
    }

    /**
      Set MPPE Stateful
    */
    void setMppestateful( bool v )
    {
        mMppestateful = v;
    }

    /**
      Get MPPE Stateful
    */
    bool mppestateful() const
    {
      return mMppestateful;
    }

    /**
      Set CRTSCTS
    */
    void setCrtscts( bool v )
    {
        mCrtscts = v;
    }

    /**
      Get CRTSCTS
    */
    bool crtscts() const
    {
      return mCrtscts;
    }

    /**
      Set Baud
    */
    void setBaud( uint v )
    {
        mBaud = v;
    }

    /**
      Get Baud
    */
    uint baud() const
    {
      return mBaud;
    }

    /**
      Set MRU
    */
    void setMru( uint v )
    {
        mMru = v;
    }

    /**
      Get MRU
    */
    uint mru() const
    {
      return mMru;
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
      Set LCP echo Failure
    */
    void setLcpechofailure( uint v )
    {
        mLcpechofailure = v;
    }

    /**
      Get LCP echo Failure
    */
    uint lcpechofailure() const
    {
      return mLcpechofailure;
    }

    /**
      Set LCP echo interval
    */
    void setLcpechointerval( uint v )
    {
        mLcpechointerval = v;
    }

    /**
      Get LCP echo interval
    */
    uint lcpechointerval() const
    {
      return mLcpechointerval;
    }

  protected:

    // ppp
    bool mNoauth;
    bool mRefuseeap;
    bool mRefusepap;
    bool mRefusechap;
    bool mRefusemschap;
    bool mRefusemschapv2;
    bool mNobsdcomp;
    bool mNodeflate;
    bool mNovjcomp;
    bool mRequiremppe;
    bool mRequiremppe128;
    bool mMppestateful;
    bool mCrtscts;
    uint mBaud;
    uint mMru;
    uint mMtu;
    uint mLcpechofailure;
    uint mLcpechointerval;

  private:
};

#endif

