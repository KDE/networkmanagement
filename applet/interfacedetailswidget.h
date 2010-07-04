/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INTERFACEDETAILSWIDGET_H
#define INTERFACEDETAILSWIDGET_H

#include <QGraphicsGridLayout>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>

#include <solid/control/networkinterface.h>
#include <solid/control/networkgsminterface.h>
#include <Plasma/Label>
#include <Plasma/PushButton>
#include <Plasma/SignalPlotter>

class RemoteActivatable;
class RemoteInterfaceConnection;
class RemoteInterfaceList;

class InterfaceDetailsWidget : public QGraphicsWidget
{
Q_OBJECT
    public:
        InterfaceDetailsWidget(QGraphicsItem* parent = 0);
        virtual ~InterfaceDetailsWidget();
        void setInterface(Solid::Control::NetworkInterface* iface);
        void setUpdateEnabled(bool enable);
        void resetUi();
        QString getLastIfaceUni();

    public Q_SLOTS:
        void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);
        //void dataUpdated(const QString&, const Plasma::DataEngine::Data&);
        void sourceAdded(const QString&);
        void handleConnectionStateChange(int new_state, int old_state, int reason);

    Q_SIGNALS:
        void back();

    private:
        Plasma::DataEngine* engine();
        void updateWidgets();
        QString bitRate();
        QString currentIpAddress();
        QString getMAC();
        void updateInfo(bool reset, int signalQuality = -1);
        QSizeF sizeHint (Qt::SizeHint which, const QSizeF & constraint = QSizeF()) const;

        Solid::Control::NetworkInterface* m_iface;
        QString m_ifaceUni;

        QGraphicsGridLayout *m_gridLayout;
        Plasma::Label* m_info;
        Plasma::Label* m_trafficNameLabel;
        Plasma::SignalPlotter *m_trafficPlotter;
        QColor m_rxColor;
        QColor m_txColor;
        Plasma::Label* m_traffic;

        Plasma::PushButton* m_backButton;

        QString m_tx;
        QString m_txSource;
        QString m_txTotalSource;
        QString m_txUnit;
        QString m_rx;
        QString m_rxSource;
        QString m_rxTotalSource;
        QString m_rxUnit;
        qlonglong m_rxTotal;
        qlonglong m_txTotal;

        bool m_updateEnabled;

    private Q_SLOTS:
        void update();

        // To update signal quality quickly, other information are not updated as fast as this one.
        void updateSignalQuality(uint signalQuality);
};

#endif // INTERFACEDETAILSWIDGET_H
