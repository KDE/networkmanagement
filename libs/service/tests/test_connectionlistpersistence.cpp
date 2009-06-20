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

#include <QtCore>
#include <QtTest>

#include "connectionlist.h"
#include "connectionlistpersistence.h"

// TODO pass this path in from CMake
static const QString TEST_DATA_PATH = "/space/kde/sources/trunk/playground/base/plasma/applets/networkmanager/libs/service/tests/data/";

class TestConnectionListPersistence : public QObject
{
Q_OBJECT
private slots:
    void initTestCase();
    void testInit();
    void testAdd();
    void testUpdate();
    void testRemove();
    void cleanupTestCase();
private:
    void updateConfigFileTo(const QString & updateWith);
    ConnectionList * list;
    ConnectionListPersistence * listPersistence;
};

void TestConnectionListPersistence::updateConfigFileTo(const QString & to)
{
    QString configFilePath = TEST_DATA_PATH + "networkmanagementrc";
    QFile origFile(configFilePath);
    origFile.remove();
    QFile newFile(TEST_DATA_PATH + to);
    if (newFile.exists()) {
        QFile::copy(newFile.fileName(), configFilePath);
    }
}

void TestConnectionListPersistence::initTestCase()
{
    // this should be cleaned up on exit but the test case may crash
    updateConfigFileTo("networkmanagementrc_init");

    QString changedConnection = TEST_DATA_PATH + "{9999b154-a9b2-484f-b09a-6249f1f64460}";
    QFile origFile(changedConnection);
    QFile newFile(changedConnection + "_orig");
    if (newFile.exists()) {
        origFile.remove();
        QFile::copy(newFile.fileName(), changedConnection);
    }

    list = new ConnectionList(0);
    listPersistence = new ConnectionListPersistence(list,
            TEST_DATA_PATH + "networkmanagementrc",
            TEST_DATA_PATH);
}

void TestConnectionListPersistence::testInit()
{
    listPersistence->init();
    QCOMPARE(list->connections().first(), QLatin1String("{9999b154-a9b2-484f-b09a-6249f1f64460}"));
}

void TestConnectionListPersistence::testAdd()
{
    updateConfigFileTo("networkmanagementrc_add");

    listPersistence->configure(QStringList());
    QVERIFY(list->connections().contains(QLatin1String("{99990901-9cc1-4420-872f-8e303f37f97b}")));
}

void TestConnectionListPersistence::testUpdate()
{
    // update config file
    updateConfigFileTo("networkmanagementrc_update");
    // change the connection file
    QString changedConnection = TEST_DATA_PATH + "{9999b154-a9b2-484f-b09a-6249f1f64460}";
    QFile origFile(changedConnection);
    QFile newFile(changedConnection + "_updated");
    if (newFile.exists()) {
        origFile.remove();
        QFile::copy(newFile.fileName(), changedConnection);
    }

    listPersistence->configure(QStringList("{9999b154-a9b2-484f-b09a-6249f1f64460}"));
    QVERIFY(list->connections().contains(QLatin1String("{99990901-9cc1-4420-872f-8e303f37f97b}")) && list->connections().contains(QLatin1String("{9999b154-a9b2-484f-b09a-6249f1f64460}")));
    Knm::Connection * changed = list->findConnection("{9999b154-a9b2-484f-b09a-6249f1f64460}");
    QVERIFY(changed != 0);
    QCOMPARE(changed->name(), QLatin1String("changed name!"));
}

void TestConnectionListPersistence::testRemove()
{
    updateConfigFileTo("networkmanagementrc_remove");

    listPersistence->configure(QStringList());
    QCOMPARE(list->connections().first(), QLatin1String("{9999b154-a9b2-484f-b09a-6249f1f64460}"));
    QVERIFY(!(list->connections().contains(QLatin1String("{99990901-9cc1-4420-872f-8e303f37f97b}"))));
}

void TestConnectionListPersistence::cleanupTestCase()
{
    updateConfigFileTo("networkmanagementrc_init");

    delete listPersistence;
    delete list;
}

QTEST_MAIN(TestConnectionListPersistence)

#include "test_connectionlistpersistence.moc"
