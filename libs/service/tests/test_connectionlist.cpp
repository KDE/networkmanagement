/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include <QtTest>
#include <QtCore>

#include <KDebug>

#include "connection.h"

#include "connectionlist.h"
#include "connectionhandler.h"

class DummyConnectionHandler : public ConnectionHandler
{
    public:
        void handleAdd(Knm::Connection *)
        {
            kDebug();
            handled = true;
        }
        void handleUpdate(Knm::Connection *)
        {
            kDebug();
            handled = true;
        }
        void handleRemove(Knm::Connection *)
        {
            kDebug();
            handled = true;
        }
    bool handled;
};

class TestConnectionList : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void testAddConnection();
    void testFindConnection();
    void testUpdateConnection();
    void testRemoveConnection();
    void testUnregisterConnectionHandler();
private:
    DummyConnectionHandler *dummy;
    ConnectionList * list;
};

void TestConnectionList::initTestCase()
{
    dummy = new DummyConnectionHandler;
    dummy->handled = false;
    list = new ConnectionList(this);
    list->registerConnectionHandler(dummy);
}

void TestConnectionList::cleanupTestCase()
{
    delete dummy;
}

void TestConnectionList::testAddConnection()
{
    Knm::Connection * conn = new Knm::Connection(QLatin1String("test add connection"), Knm::Connection::Wired);
    QString uuid = conn->uuid();
    list->addConnection(conn);
    QVERIFY(list->connections().contains(uuid));
}

void TestConnectionList::testFindConnection()
{
    Knm::Connection * conn = new Knm::Connection(QLatin1String("test find connection"), Knm::Connection::Wired);
    QString uuid = conn->uuid();
    list->addConnection(conn);
    QVERIFY(list->connections().contains(uuid));
    Knm::Connection * retrieved = list->findConnection(uuid);
    QCOMPARE(conn, retrieved);
}

void TestConnectionList::testUpdateConnection()
{
    Knm::Connection * conn = new Knm::Connection(QLatin1String("test update connection"), Knm::Connection::Wired);
    list->addConnection(conn);
    Knm::Connection * updated = new Knm::Connection(conn->uuid(), Knm::Connection::Wired);
    updated->setName(QLatin1String("test update connection UPDATED"));

    list->updateConnection(updated);
    Knm::Connection * retrieved = list->findConnection(updated->uuid());
    QVERIFY(retrieved != 0);
    QVERIFY(updated == retrieved);
    QVERIFY(conn != retrieved);
}

void TestConnectionList::testRemoveConnection() {
    Knm::Connection * conn = new Knm::Connection(QLatin1String("test remove connection"), Knm::Connection::Wired);
    QString uuid = conn->uuid();
    list->addConnection(conn);
    list->removeConnection(conn);
    Knm::Connection * retrieved = list->findConnection(uuid);
    QVERIFY(retrieved == 0);
}

void TestConnectionList::testUnregisterConnectionHandler()
{
    dummy->handled = false;
    list->unregisterConnectionHandler(dummy);
}

QTEST_MAIN(TestConnectionList)

#include "test_connectionlist.moc"
