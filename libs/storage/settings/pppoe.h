// This file is generated by kconfig_compiler from pppoe.kcfg.
// All changes you do to this file will be lost.
#ifndef KNMINTERNALS_PPPOESETTING_H
#define KNMINTERNALS_PPPOESETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knm_export.h"
namespace KnmInternals {

class KNM_EXPORT PppoeSetting : public Setting
{
  public:

    PppoeSetting( );
    ~PppoeSetting();

    QString name() const;

    bool hasSecrets() const;

    /**
      Set Service
    */
    void setService( const QString & v )
    {
        mService = v;
    }

    /**
      Get Service
    */
    QString service() const
    {
      return mService;
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

  protected:

    // pppoe
    QString mService;
    QString mUsername;
    QString mPassword;

  private:
};

}

#endif

