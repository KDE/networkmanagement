// This file is generated by kconfig_compiler from 802-1x.kcfg.
// All changes you do to this file will be lost.

#include "802-1x.h"
#include "802-1xsecrets.h"

#include <QUuid>
#include <kstandarddirs.h>

using namespace Knm;

const QString Security8021xSetting::CERTIFICATE_PERSISTENCE_PATH = QLatin1String("networkmanagement/certificates/");

Security8021xSetting::Security8021xSetting() : Setting(Setting::Security8021x),
    mPhase1peapver(0), mPhase2auth(0), mPhase2autheap(0), mEnabled(false), mUseSystemCaCerts(false), mCertificatesToDelete(0)
{
    m_secretsObject = new Security8021xSecrets(this);
}

Security8021xSetting::~Security8021xSetting()
{
}

QString Security8021xSetting::name() const
{
    return QLatin1String("802-1x");
}

bool Security8021xSetting::hasSecrets() const
{
    return mEnabled;
}

void Security8021xSetting::setSecrets(Setting::secretsTypes types)
{
    if (!mPassword.isEmpty())
        setPasswordflags(types);
    if (!mPrivatekeypassword.isEmpty())
        setPrivatekeypasswordflags(types);
    if (!mPhase2privatekeypassword.isEmpty())
        setPhase2privatekeypasswordflags(types);
}

QByteArray Security8021xSetting::getBytes(const QString & fileName)
{
    QByteArray bytes;
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly)) {
        bytes = file.readAll();
        if (bytes.startsWith("-----BEGIN CERTIFICATE-----")) {
            bytes.remove(0,27);
            bytes = bytes.left(bytes.indexOf("-----END CERTIFICATE-----"));
            bytes = QByteArray::fromBase64(bytes);
        }
    }
    return bytes;
}

QString Security8021xSetting::importCertFromPath(const QString & oldpath, const QString & newpath, bool scope)
{
    if (newpath.isEmpty()) {
        QString certificateId = QUuid::createUuid().toString();
        QString ourCertFile;
        switch (scope)
        {
            case true:
                ourCertFile = KStandardDirs::installPath("data")+ CERTIFICATE_PERSISTENCE_PATH + certificateId;
                break;
            case false:
            default:
                ourCertFile = KStandardDirs::locateLocal("data", CERTIFICATE_PERSISTENCE_PATH + certificateId,true);
                break;
        }
        // try 10 times in case the quuid already exists
        bool success = false;
        for (int i = 0; i < 10 && !success; i++) {
            success = QFile::copy(oldpath, ourCertFile);
        }
        return ourCertFile;
    } else {
        QFile::remove(newpath);
        QFile::copy(oldpath, newpath);
        return newpath;
    }
}

void Security8021xSetting::save(bool scope)
{
    if (certtodelete() & Knm::Security8021xSetting::CACert) {
        QFile::remove(pathFromCert(cacert()));
        setCapath("");
        setCacert(QByteArray());
    }
    if (certtodelete() & Knm::Security8021xSetting::ClientCert) {
        QFile::remove(pathFromCert(clientcert()));
        setClientcertpath("");
        setClientcert(QByteArray());
    }
    if (certtodelete() & Knm::Security8021xSetting::Phase2CACert) {
        QFile::remove(pathFromCert(phase2cacert()));
        setPhase2capath("");
        setPhase2cacert(QByteArray());
    }
    if (certtodelete() & Knm::Security8021xSetting::Phase2ClientCert) {
        QFile::remove(pathFromCert(phase2clientcert()));
        setPhase2clientcertpath("");
        setCacert(QByteArray());
    }
    if (certtodelete() & Knm::Security8021xSetting::PrivateKey)
    {
        QFile::remove(pathFromCert(privatekey()));
        setPrivatekeypath("");
        setPrivatekey(QByteArray());
    }
    if (certtodelete() & Knm::Security8021xSetting::Phase2PrivateKey) {
        QFile::remove(pathFromCert(phase2privatekey()));
        setPhase2privatekeypath("");
        setPhase2privatekey(QByteArray());
    }

    if (!cacerttoimport().isEmpty())
        setCacert( certPathAsByteArray(importCertFromPath(cacerttoimport(),pathFromCert(cacert()),scope)) );
    if (!clientcerttoimport().isEmpty())
        setClientcert( certPathAsByteArray(importCertFromPath(clientcerttoimport(),pathFromCert(clientcert()),scope)) );
    if (!phase2cacerttoimport().isEmpty())
        setPhase2cacert( certPathAsByteArray(importCertFromPath(phase2cacerttoimport(),pathFromCert(phase2cacert()),scope)) );
    if (!phase2clientcerttoimport().isEmpty())
        setPhase2clientcert( certPathAsByteArray(importCertFromPath(phase2clientcerttoimport(),pathFromCert(phase2clientcert()),scope)) );
    if (!privatekeytoimport().isEmpty())
        setPrivatekey( certPathAsByteArray(importCertFromPath(privatekeytoimport(),pathFromCert(privatekey()),scope)) );
    if (!phase2privatekeytoimport().isEmpty())
        setPhase2privatekey( certPathAsByteArray(importCertFromPath(phase2privatekeytoimport(),pathFromCert(phase2privatekey()),scope)) );
}

void Security8021xSetting::remove()
{
    if (!cacert().isEmpty())
        QFile::remove(pathFromCert(cacert()));
    if (!clientcert().isEmpty())
        QFile::remove(pathFromCert(clientcert()));
    if (!phase2cacert().isEmpty())
        QFile::remove(pathFromCert(phase2cacert()));
    if (!phase2clientcert().isEmpty())
        QFile::remove(pathFromCert(phase2clientcert()));
    if (!privatekey().isEmpty())
        QFile::remove(pathFromCert(privatekey()));
    if (!phase2privatekey().isEmpty())
        QFile::remove(pathFromCert(phase2privatekey()));
}
