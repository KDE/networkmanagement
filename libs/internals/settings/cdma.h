// This file is generated by kconfig_compiler from cdma.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_CDMASETTING_H
#define KNM_CDMASETTING_H

#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knminternals_export.h"
namespace Knm {

class KNMINTERNALS_EXPORT CdmaSetting : public Setting
{
  public:

    CdmaSetting( );
    CdmaSetting(CdmaSetting *);
    ~CdmaSetting();

    QString name() const;

    QMap<QString,QString> secretsToMap() const;
    void secretsFromMap(QMap<QString,QString> secrets);
    QStringList needSecrets() const;
    bool hasPersistentSecrets() const;

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
    Setting::secretsTypes passwordflags() const
    {
        return mPasswordflags;
    }

  protected:

    // cdma
    QString mNumber;
    QString mUsername;
    QString mPassword;
    Setting::secretsTypes mPasswordflags;

  private:
};

}

#endif

