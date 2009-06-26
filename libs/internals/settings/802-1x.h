// This file is generated by kconfig_compiler from 802-1x.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_SECURITY8021XSETTING_H
#define KNM_SECURITY8021XSETTING_H

#include <QFile>
#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "knminternals_export.h"
namespace Knm {

class KNMINTERNALS_EXPORT Security8021xSetting : public Setting
{
  public:
    class EnumPhase1peapver
    {
      public:
      enum type { zero, one, COUNT };
    };
    class EnumPhase1peaplabel
    {
      public:
      enum type { zero, one, COUNT };
    };
    class EnumPhase2autheap
    {
      public:
      enum type { pap, mschap, mschapv2, chap, COUNT };
    };

    Security8021xSetting( );
    ~Security8021xSetting();

    QString name() const;

    bool hasSecrets() const;

    /**
      Set EAP
    */
    void setEap( const QStringList & v )
    {
        mEap = v;
    }

    /**
      Get EAP
    */
    QStringList eap() const
    {
      return mEap;
    }

    /**
      Set Identity
    */
    void setIdentity( const QString & v )
    {
        mIdentity = v;
    }

    /**
      Get Identity
    */
    QString identity() const
    {
      return mIdentity;
    }

    /**
      Set Anonymous identity
    */
    void setAnonymousidentity( const QString & v )
    {
        mAnonymousidentity = v;
    }

    /**
      Get Anonymous identity
    */
    QString anonymousidentity() const
    {
      return mAnonymousidentity;
    }

    /**
      Set CA Cert
    */
    void setCacert( const QByteArray & v )
    {
        mCacert = v;
    }

    /**
      Get CA Cert
    */
    QByteArray cacert() const
    {
      return mCacert;
    }

    /**
      Set CA Path
    */
    void setCapath( const QString & v )
    {
        mCapath = v;

        // also update the ca cert blob
        QFile ca_cert(v);

        if (ca_cert.open(QIODevice::ReadOnly)) {
           QByteArray bytes = ca_cert.readAll();

           // FIXME: verify that the ca_cert is a X509 cert
           // (see libnm-util/nm-setting-8021x.c function nm_setting_802_1x_set_ca_cert_from_file)

           setCacert( bytes );
        }
    }

    /**
      Get CA Path
    */
    QString capath() const
    {
      return mCapath;
    }

    /**
      Set Client Cert
    */
    void setClientcert( const QByteArray & v )
    {
        mClientcert = v;
    }

    /**
      Get Client Cert
    */
    QByteArray clientcert() const
    {
      return mClientcert;
    }

    /**
      Set Phase 1 PEAP version
    */
    void setPhase1peapver( int v )
    {
        mPhase1peapver = v;
    }

    /**
      Get Phase 1 PEAP version
    */
    int phase1peapver() const
    {
      return mPhase1peapver;
    }

    /**
      Set Phase 1 PEAP label
    */
    void setPhase1peaplabel( const QString & v )
    {
        mPhase1peaplabel = v;
    }

    /**
      Get Phase 1 PEAP label
    */
    QString phase1peaplabel() const
    {
      return mPhase1peaplabel;
    }

    /**
      Set Phase 1 fast provisioning
    */
    void setPhase1fastprovisioning( const QString & v )
    {
        mPhase1fastprovisioning = v;
    }

    /**
      Get Phase 1 fast provisioning
    */
    QString phase1fastprovisioning() const
    {
      return mPhase1fastprovisioning;
    }

    /**
      Set Phase 2 auth
    */
    void setPhase2auth( const QString & v )
    {
        mPhase2auth = v;
    }

    /**
      Get Phase 2 auth
    */
    QString phase2auth() const
    {
      return mPhase2auth;
    }

    /**
      Set Phase 2 auth eap
    */
    void setPhase2autheap( int v )
    {
        mPhase2autheap = v;
    }

    /**
      Get Phase 2 auth eap
    */
    int phase2autheap() const
    {
      return mPhase2autheap;
    }

    /**
      Set Phase 2 CA Cert
    */
    void setPhase2cacert( const QByteArray & v )
    {
        mPhase2cacert = v;
    }

    /**
      Get Phase 2 CA Cert
    */
    QByteArray phase2cacert() const
    {
      return mPhase2cacert;
    }

    /**
      Set Phase 2 CA Path
    */
    void setPhase2capath( const QString & v )
    {
        mPhase2capath = v;
    }

    /**
      Get Phase 2 CA Path
    */
    QString phase2capath() const
    {
      return mPhase2capath;
    }

    /**
      Set Phase 2 client cert
    */
    void setPhase2clientcert( const QByteArray & v )
    {
        mPhase2clientcert = v;
    }

    /**
      Get Phase 2 client cert
    */
    QByteArray phase2clientcert() const
    {
      return mPhase2clientcert;
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
      Set Private key
    */
    void setPrivatekey( const QByteArray & v )
    {
        mPrivatekey = v;
    }

    /**
      Get Private key
    */
    QByteArray privatekey() const
    {
      return mPrivatekey;
    }

    /**
      Set Phase 2 private key
    */
    void setPhase2privatekey( const QByteArray & v )
    {
        mPhase2privatekey = v;
    }

    /**
      Get Phase 2 private key
    */
    QByteArray phase2privatekey() const
    {
      return mPhase2privatekey;
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
      Set Connection uses 802.1x
    */
    void setEnabled( bool v )
    {
        mEnabled = v;
    }

    /**
      Get Connection uses 802.1x
    */
    bool enabled() const
    {
      return mEnabled;
    }

    enum EapMethod
    {
        ttls = 1,
        peap = 2,
        tls  = 4
    };
    Q_DECLARE_FLAGS(EapMethods, EapMethod)

    void setEapFlags( const EapMethods& methods )
    {
        QStringList eap;
        if (methods.testFlag(ttls))
            eap.append("ttls");
        if (methods.testFlag(tls))
            eap.append("tls");
        if (methods.testFlag(peap))
            eap.append("peap");
kDebug() << eap;
        setEap(eap);
    }

    EapMethods eapFlags() const
    {
        QStringList eaps = eap();
        EapMethods eapFlags;
        if (eaps.contains("ttls"))
            eapFlags = eapFlags | ttls;
        if (eaps.contains("tls"))
            eapFlags = eapFlags | tls;
        if (eaps.contains("peap"))
            eapFlags = eapFlags | peap;
        return eapFlags;
    }



  protected:

    // 802-1x
    QStringList mEap;
    QString mIdentity;
    QString mAnonymousidentity;
    QByteArray mCacert;
    QString mCapath;
    QByteArray mClientcert;
    int mPhase1peapver;
    QString mPhase1peaplabel;
    QString mPhase1fastprovisioning;
    QString mPhase2auth;
    int mPhase2autheap;
    QByteArray mPhase2cacert;
    QString mPhase2capath;
    QByteArray mPhase2clientcert;
    QString mPassword;
    QByteArray mPrivatekey;
    QByteArray mPhase2privatekey;
    QString mPin;
    QString mPsk;
    bool mEnabled;

  private:
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Security8021xSetting::EapMethods)

}

#endif

