// This file is generated by kconfig_compiler from 802-1x.kcfg.
// All changes you do to this file will be lost.

#include "802-1x.h"
#include "802-1xsecrets.h"

using namespace Knm;

Security8021xSetting::Security8021xSetting() : Setting(Setting::Security8021x),
    mPhase1peapver(0), mPhase2auth(0), mPhase2autheap(0), mEnabled(false), mPasswordflags(Setting::AgentOwned), mPrivatekeypasswordflags(Setting::AgentOwned), mPhase2privatekeypasswordflags(Setting::AgentOwned)
{
    m_secretsObject = new Security8021xSecrets(this);
}

Security8021xSetting::Security8021xSetting(Security8021xSetting *setting) : Setting(setting)
{
    m_secretsObject = new Security8021xSecrets(static_cast<Security8021xSecrets*>(setting->getSecretsObject()), this);
    setEap(setting->eap());
    setIdentity(setting->identity());
    setAnonymousidentity(setting->anonymousidentity());
    setCacert(setting->cacert());
    setCapath(setting->capath());
    setSubjectmatch(setting->subjectmatch());
    setAltsubjectmatches(setting->altsubjectmatches());
    setClientcert(setting->clientcert());
    setClientcertpath(setting->clientcertpath());
    setPhase1peapver(setting->phase1peapver());
    setPhase1peaplabel(setting->phase1peaplabel());
    setPhase1fastprovisioning(setting->phase1fastprovisioning());
    setPhase2auth(setting->phase2auth());
    setPhase2autheap(setting->phase2autheap());
    setPhase2cacert(setting->phase2cacert());
    setPhase2capath(setting->phase2capath());
    setPhase2subjectmatch(setting->phase2subjectmatch());
    setPhase2altsubjectmatches(setting->phase2altsubjectmatches());
    setPhase2clientcert(setting->phase2clientcert());
    setPhase2clientcertpath(setting->phase2clientcertpath());
    setPassword(setting->password());
    setPasswordflags(setting->passwordflags());
    setPrivatekey(setting->privatekey());
    setPrivatekeypath(setting->privatekeypath());
    setPrivatekeypassword(setting->privatekeypassword());
    setPrivatekeypasswordflags(setting->privatekeypasswordflags());
    setPhase2privatekey(setting->phase2privatekey());
    setPhase2privatekeypath(setting->phase2privatekeypath());
    setPhase2privatekeypassword(setting->phase2privatekeypassword());
    setPhase2privatekeypasswordflags(setting->phase2privatekeypasswordflags());
    setUseSystemCaCerts(setting->useSystemCaCerts());
    setEnabled(setting->enabled());
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
}

QByteArray Security8021xSetting::getBytes(const QString & fileName)
{
    QByteArray bytes;
    QFile file(fileName);

    if (file.open(QIODevice::ReadOnly)) {
        bytes = file.readAll();
    }
    return bytes;
}
