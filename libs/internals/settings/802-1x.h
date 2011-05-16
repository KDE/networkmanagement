// This file is generated by kconfig_compiler from 802-1x.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_SECURITY8021XSETTING_H
#define KNM_SECURITY8021XSETTING_H

#include <QFile>
#include <QFlags>
#include <kglobal.h>
#include <kdebug.h>
#include <kcoreconfigskeleton.h>
#include "setting.h"
#include "connection.h"
#include "knminternals_export.h"
namespace Knm {

class KNMINTERNALS_EXPORT Security8021xSetting : public Setting
{
  Q_FLAGS(Certificates)
  public:
    class EnumPhase1peapver
    {
      public:
      enum type { automatic, zero, one, COUNT };
    };
    class EnumPhase1peaplabel
    {
      public:
      enum type { automatic, zero, one, COUNT };
    };
    class EnumPhase2auth
    {
      public:
      enum type { none, pap, mschap, mschapv2, chap, md5, gtc, otp, COUNT };
    };
    class EnumPhase2autheap
    {
      public:
      enum type { none, md5, gtc, otp, mschapv2, tls, COUNT };
    };

    Security8021xSetting( );
    ~Security8021xSetting();

    enum Certificate {CACert = 0x1, ClientCert = 0x2, Phase2CACert = 0x4, Phase2ClientCert = 0x8, PrivateKey = 0x10, Phase2PrivateKey = 0x20};
    Q_DECLARE_FLAGS(Certificates, Certificate)

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
    }

    /**
      Get CA Path
    */
    QString capath() const
    {
      return mCapath;
    }

    void setCacertFromPath( const QString & v )
    {
        setCapath(v);
        setCacert(getBytes(v));
    }

    /**
    Get CA Cert to import
    */
    QString cacerttoimport() const
    {
        return mCacertToImport;
    }

    /**
    Set CA Cert to import
    */
    void setCacerttoimport( const QString & v )
    {
        mCacertToImport = v;
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
      Set Client Cert Path
    */
    void setClientcertpath( const QString & v )
    {
        mClientcertpath = v;
    }

    /**
      Get Client Cert Path
    */
    QString clientcertpath() const
    {
      return mClientcertpath;
    }

    void setClientcertFromPath( const QString & v )
    {
        setClientcertpath(v);
        setClientcert(getBytes(v));
    }

    /**
    Get client Cert to import
    */
    QString clientcerttoimport() const
    {
        return mClientcertToImport;
    }

    /**
    Set client Cert to import
    */
    void setClientcerttoimport( const QString & v )
    {
        mClientcertToImport = v;
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
    void setPhase2auth( int v )
    {
        mPhase2auth = v;
    }

    /**
      Get Phase 2 auth
    */
    int phase2auth() const
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

    void setPhase2cacertFromPath( const QString & v)
    {
        setPhase2capath(v);
        setPhase2cacert(getBytes(v));
    }

    /**
    Get Phase2CA Cert to import
    */
    QString phase2cacerttoimport() const
    {
        return mPhase2cacertToImport;
    }

    /**
    Set Phase2CA Cert to import
    */
    void setPhase2cacerttoimport( const QString & v )
    {
        mPhase2cacertToImport = v;
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
      Set Phase 2 client cert path
    */
    void setPhase2clientcertpath( const QString & v )
    {
        mPhase2clientcertpath = v;
    }

    /**
      Get Phase 2 client cert path
    */
    QString phase2clientcertpath() const
    {
      return mPhase2clientcertpath;
    }

    void setPhase2clientcertFromPath( const QString & v)
    {
        setPhase2clientcertpath(v);
        setPhase2clientcert(getBytes(v));
    }

    /**
    Get Phase2 client Cert to import
    */
    QString phase2clientcerttoimport() const
    {
        return mPhase2clientcertToImport;
    }

    /**
    Set Phase2 client Cert to import
    */
    void setPhase2clientcerttoimport( const QString & v )
    {
        mPhase2clientcertToImport = v;
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
      Set Private key Path
    */
    void setPrivatekeypath( const QString & v )
    {
        mPrivatekeypath = v;
    }

    /**
      Get Private key Path
    */
    QString privatekeypath() const
    {
      return mPrivatekeypath;
    }

    void setPrivatekeyFromPath( const QString & v )
    {
        setPrivatekeypath(v);
        setPrivatekey(getBytes(v));
    }

    /**
    Get private key to import
    */
    QString privatekeytoimport() const
    {
        return mPrivatekeyToImport;
    }

    /**
    Set private key to import
    */
    void setPrivatekeytoimport( const QString & v )
    {
        mPrivatekeyToImport = v;
    }

    /**
      Set Private key password
    */
    void setPrivatekeypassword( const QString & v )
    {
        mPrivatekeypassword = v;
    }

    /**
      Get Private key password
    */
    QString privatekeypassword() const
    {
      return mPrivatekeypassword;
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
      Set Phase 2 Private key Path
    */
    void setPhase2privatekeypath( const QString & v )
    {
        mPhase2privatekeypath = v;
    }

    /**
      Get Phase 2 Private key Path
    */
    QString phase2privatekeypath() const
    {
      return mPhase2privatekeypath;
    }

    void setPhase2privatekeyFromPath( const QString & v )
    {
        setPhase2privatekeypath(v);
        setPhase2privatekey(getBytes(v));
    }

    /**
    Get Phase2private key to import
    */
    QString phase2privatekeytoimport() const
    {
        return mPhase2privatekeyToImport;
    }

    /**
    Set Phase2private key to import
    */
    void setPhase2privatekeytoimport( const QString & v )
    {
        mPhase2privatekeyToImport = v;
    }

    /**
      Set Phase 2 private key password
    */
    void setPhase2privatekeypassword( const QString & v )
    {
        mPhase2privatekeypassword = v;
    }

    /**
      Get Phase 2 private key password
    */
    QString phase2privatekeypassword() const
    {
      return mPhase2privatekeypassword;
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
      Set Use System CA Certs
    */
    void setUseSystemCaCerts( bool v )
    {
        mUseSystemCaCerts = v;
    }

    /**
      Get Use System CA Certs
    */
    bool useSystemCaCerts() const
    {
      return mUseSystemCaCerts;
    }

    /**
      Set Connection uses 802.1x
    */
    void setEnabled( bool v )
    {
        mEnabled = v;
        if (!mEnabled)
            remove();
    }

    /**
      Get Connection uses 802.1x
    */
    bool enabled() const
    {
      return mEnabled;
    }

    /**
    Get Certs to delete
    */
    bool certtodelete() const
    {
        return mCertificatesToDelete;
    }

    /**
    Set Certificates to delete
    */
    void addToCertToDelete( Certificate v )
    {
        mCertificatesToDelete |= v;
    }

    /**
    Remove Certificates to delete
    */
    void removeFromCertToDelete( Certificate v )
    {
        mCertificatesToDelete &= !v;
    }

    QString pathFromCert(const QByteArray & cert)
    {
        int len =  cert.size() - 7;
        if (len < 0)
            return QString();
        QString path = QString(cert.right(len));
        return path;
    }

    QByteArray certPathAsByteArray(const QString & path)
    {
        QByteArray arr = QByteArray("file://" + path.toUtf8());
        arr.append('\0');
        return arr;
    }

    enum EapMethod
    {
        ttls = 1,
        peap = 2,
        tls  = 4,
        leap = 8
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
        if (methods.testFlag(leap))
            eap.append("leap");
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
        if (eaps.contains("leap"))
            eapFlags = eapFlags | leap;
        return eapFlags;
    }

    QString importCertFromPath( const QString & oldpath, const QString & newpath = QString(), bool scope = true );
    void save(bool scope);
    void remove();

  protected:

    // 802-1x
    QStringList mEap;
    QString mIdentity;
    QString mAnonymousidentity;
    QByteArray mCacert;
    QString mCapath;
    QString mCacertToImport;
    QByteArray mClientcert;
    QString mClientcertpath;
    QString mClientcertToImport;
    int mPhase1peapver;
    QString mPhase1peaplabel;
    QString mPhase1fastprovisioning;
    int mPhase2auth;
    int mPhase2autheap;
    QByteArray mPhase2cacert;
    QString mPhase2capath;
    QString mPhase2cacertToImport;
    QByteArray mPhase2clientcert;
    QString mPhase2clientcertpath;
    QString mPhase2clientcertToImport;
    QString mPassword;
    QByteArray mPrivatekey;
    QString mPrivatekeypath;
    QString mPrivatekeyToImport;
    QString mPrivatekeypassword;
    QByteArray mPhase2privatekey;
    QString mPhase2privatekeypath;
    QString mPhase2privatekeyToImport;
    QString mPhase2privatekeypassword;
    QString mPin;
    QString mPsk;
    bool mEnabled;
    bool mUseSystemCaCerts;
    Certificates mCertificatesToDelete;

  private:
    static const QString CERTIFICATE_PERSISTENCE_PATH;
    QByteArray getBytes(const QString & fileName);
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Security8021xSetting::EapMethods)

}

#endif

