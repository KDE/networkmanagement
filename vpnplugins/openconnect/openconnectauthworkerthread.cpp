/*
Copyright 2011 Ilia Kats <ilia-kats@gmx.net>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) version 3, or any
later version accepted by the membership of KDE e.V. (or its
successor approved by the membership of KDE e.V.), which shall
act as a proxy defined in Section 6 of version 3 of the license.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "openconnectauthworkerthread.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <QByteArray>

extern "C"
{
#include <openconnect.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/ossl_typ.h>
#include <errno.h>
}

#include <cstdarg>

class OpenconnectAuthStaticWrapper
{
public:
    static int writeNewConfig(void *obj, char *str, int num)
    {
        if (obj)
            return static_cast<OpenconnectAuthWorkerThread*>(obj)->writeNewConfig(str, num);
        return -1;
    }
    static int validatePeerCert(void *obj, struct x509_st *cert, const char *str)
    {
        if (obj)
            return static_cast<OpenconnectAuthWorkerThread*>(obj)->validatePeerCert(cert, str);
        return -1;
    }
    static int processAuthForm(void *obj, struct oc_auth_form *form)
    {
        if (obj)
            return static_cast<OpenconnectAuthWorkerThread*>(obj)->processAuthFormP(form);
        return -1;
    }
    static void writeProgress(void *obj, int level, const char *str, ...)
    {
        if (obj) {
            va_list argPtr;
            va_start(argPtr, str);
            static_cast<OpenconnectAuthWorkerThread*>(obj)->writeProgress(level, str, argPtr);
            va_end(argPtr);
        }
    }
};


OpenconnectAuthWorkerThread::OpenconnectAuthWorkerThread(QMutex *mutex, QWaitCondition *waitForUserInput, bool *userDecidedToQuit)
: QThread(), m_mutex(mutex), m_waitForUserInput(waitForUserInput), m_userDecidedToQuit(userDecidedToQuit)
{
    m_openconnectInfo = openconnect_vpninfo_new_with_cbdata((char*)"OpenConnect VPN Agent (NetworkManager - running on KDE)",
                                         OpenconnectAuthStaticWrapper::validatePeerCert,
                                         OpenconnectAuthStaticWrapper::writeNewConfig,
                                         OpenconnectAuthStaticWrapper::processAuthForm,
                                         OpenconnectAuthStaticWrapper::writeProgress,
                                         this);
}

OpenconnectAuthWorkerThread::~OpenconnectAuthWorkerThread()
{
    openconnect_vpninfo_free(m_openconnectInfo);
}

void OpenconnectAuthWorkerThread::run()
{
    openconnect_init_openssl();
    int ret = openconnect_obtain_cookie(m_openconnectInfo);
    if (*m_userDecidedToQuit)
        return;
    emit cookieObtained(ret);
}

struct openconnect_info* OpenconnectAuthWorkerThread::getOpenconnectInfo()
{
    return m_openconnectInfo;
}

int OpenconnectAuthWorkerThread::writeNewConfig(char *buf, int buflen)
{
    Q_UNUSED(buflen)
    if (*m_userDecidedToQuit)
        return -EINVAL;
    emit writeNewConfig(QString(QByteArray(buf).toBase64()));
    return 0;
}

int OpenconnectAuthWorkerThread::validatePeerCert(struct x509_st *cert, const char *reason)
{
    if (*m_userDecidedToQuit)
        return -EINVAL;
    char fingerprint[EVP_MAX_MD_SIZE * 2 + 1];
    int ret = 0;

    ret = openconnect_get_cert_sha1(m_openconnectInfo, cert, fingerprint);
    if (ret)
        return ret;

    BIO *bp = BIO_new(BIO_s_mem());
    BUF_MEM *certinfo;
    X509_print_ex(bp, cert, 0, 0);

    BIO_get_mem_ptr(bp, &certinfo);

    bool accepted = false;
    m_mutex->lock();
    QString qFingerprint(fingerprint);
    QString qCertinfo(certinfo->data);
    QString qReason(reason);
    emit validatePeerCert(qFingerprint, qCertinfo, qReason, &accepted);
    m_waitForUserInput->wait(m_mutex);
    m_mutex->unlock();
    BIO_free(bp);
    if (*m_userDecidedToQuit)
        return -EINVAL;

    if (accepted)
        return 0;
    else
        return -EINVAL;

}

int OpenconnectAuthWorkerThread::processAuthFormP(struct oc_auth_form *form)
{
    if (*m_userDecidedToQuit)
        return -1;

    m_mutex->lock();
    emit processAuthForm(form);
    m_waitForUserInput->wait(m_mutex);
    m_mutex->unlock();
    if (*m_userDecidedToQuit)
        return -1;

    return 0;
}

void OpenconnectAuthWorkerThread::writeProgress(int level, const char *fmt, va_list argPtr)
{
    if (*m_userDecidedToQuit)
        return;
    QString msg;
    msg.vsprintf(fmt, argPtr);
    emit updateLog(msg, level);
}

