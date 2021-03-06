/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>
Copyright 2010-2013 Lamarque V. Souza <lamarque@kde.org>

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

#include <NetworkManagerQt/device.h>
#include <NetworkManagerQt/modemdevice.h>
#include <Plasma/Label>
#include <Plasma/IconWidget>
#include <Plasma/SignalPlotter>

#include "knmserviceprefs.h"

class RemoteActivatable;
class RemoteInterfaceConnection;
class RemoteInterfaceList;
class InterfaceDetails;

class InterfaceDetailsWidget : public QGraphicsWidget
{
Q_OBJECT
    public:
        InterfaceDetailsWidget(QGraphicsItem* parent = 0);
        virtual ~InterfaceDetailsWidget();
        void setInterface(const NetworkManager::Device::Ptr &iface = NetworkManager::Device::Ptr(), bool disconnectOld = true);
        void resetUi();
        QString getLastIfaceUni();

    public Q_SLOTS:
        void setUpdateEnabled(bool enable);
        void dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data);
        //void dataUpdated(const QString&, const Plasma::DataEngine::Data&);
        //void sourceAdded(const QString&);
        void handleConnectionStateChange(NetworkManager::Device::State new_state, NetworkManager::Device::State old_state, NetworkManager::Device::StateChangeReason reason);

    Q_SIGNALS:
        void back();
        void disconnectInterfaceRequested(const QString& deviceUni);

    private:
        Plasma::DataEngine* engine();
        void updateWidgets();
        int bitRate();
        QString getMAC();
        QString connectionStateToString(NetworkManager::Device::State state, const QString &connectionName = QString());
        void getDetails();
        void showDetails(bool reset = false);
        void connectSignals();
        void disconnectSignals();

        NetworkManager::Device::Ptr m_iface;
        QString m_ifaceUni;

        QGraphicsGridLayout *m_gridLayout;
        Plasma::Label* m_info;
        Plasma::Label* m_trafficNameLabel;
        Plasma::SignalPlotter *m_trafficPlotter;
        Plasma::Label* m_traffic;

        Plasma::IconWidget* m_disconnectButton;
        Plasma::IconWidget* m_backButton;

        QString m_tx;
        QString m_txSource;
        QString m_txTotalSource;
        QString m_txUnit;
        QColor m_txColor;
        QString m_rx;
        QString m_rxSource;
        QString m_rxTotalSource;
        QString m_rxUnit;
        QColor m_rxColor;
        qlonglong m_rxTotal;
        qlonglong m_txTotal;

        bool m_updateEnabled;
        InterfaceDetails * details;
        int m_speedUnit;

    private Q_SLOTS:
        void emitDisconnectInterfaceRequest();
        void resetInterfaceDetails();
        void updateIPv4Details();
        void updateBitRate(int bitRate);
        void updateActiveAccessPoint(const QString &ap);
        void modemUpdateEnabled(const bool enabled);
        void modemUpdateBand();
        void modemUpdateUnlockRequired(const QString &);
        void modemUpdateRegistrationInfo(const ModemManager::ModemGsmNetworkInterface::RegistrationInfoType & registrationInfo);
        void modemUpdateAccessTechnology(const ModemManager::ModemInterface::AccessTechnology & tech);
        void modemUpdateSignalQuality(const uint signalQuality);
        void modemUpdateAllowedMode(const ModemManager::ModemInterface::AllowedMode mode);
};

#endif // INTERFACEDETAILSWIDGET_H
