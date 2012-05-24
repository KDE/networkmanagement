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

#ifndef OPENCONNECTAUTHWORKERTHREAD_H
#define OPENCONNECTAUTHWORKERTHREAD_H

#include <QThread>

class QMutex;
class QWaitCondition;
struct openconnect_info;

class OpenconnectAuthWorkerThread : public QThread
{
    Q_OBJECT
    friend class OpenconnectAuthStaticWrapper;
public:
    OpenconnectAuthWorkerThread(QMutex *, QWaitCondition *, bool *, int);
    ~OpenconnectAuthWorkerThread();
    struct openconnect_info* getOpenconnectInfo();

Q_SIGNALS:
    void validatePeerCert(const QString &, const QString &, const QString &, bool*);
    void processAuthForm(struct oc_auth_form *);
    void updateLog(const QString &, const int&);
    void writeNewConfig(const QString &);
    void cookieObtained(const int&);

protected:
    void run();

private:
    int writeNewConfig(char *, int);
    int validatePeerCert(struct x509_st *, const char *);
    int processAuthFormP(struct oc_auth_form *);
    void writeProgress(int level, const char *, va_list);

    QMutex *m_mutex;
    QWaitCondition *m_waitForUserInput;
    bool *m_userDecidedToQuit;
    struct openconnect_info *m_openconnectInfo;
};

#endif
