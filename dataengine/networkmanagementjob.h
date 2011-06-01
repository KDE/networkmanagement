/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef NM_JOB_H
#define NM_JOB_H

// plasma
#include <Plasma/ServiceJob>

class KActivityConsumer;

class NetworkManagementJob : public Plasma::ServiceJob
{

    Q_OBJECT

    public:
        NetworkManagementJob(const QString &id, const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent = 0);
        ~NetworkManagementJob();

    protected:
        void start();

    private:
        QString m_id;
};

#endif // NM_JOB_H
