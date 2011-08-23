// This file is generated by kconfig_compiler from pppoe.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_PPPOESETTING_H
#define KNM_PPPOESETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knminternals_export.h"
namespace Knm {

class KNMINTERNALS_EXPORT PppoeSetting : public Setting
{
  public:

    PppoeSetting( );
    PppoeSetting(PppoeSetting *);
    ~PppoeSetting();

    QString name() const;

    bool hasSecrets() const;

    QMap<QString,QString> secretsToMap();
    void secretsFromMap(QMap<QString,QString> secrets);
    QStringList needSecrets();

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

  protected:

    // pppoe
    QString mService;
    QString mUsername;
    QString mPassword;
    Setting::secretsTypes mPasswordflags;

  private:
};

}

#endif

