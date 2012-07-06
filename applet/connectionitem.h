#ifndef CONNECTION_ITEM_H
#define CONNECTION_ITEM_H

#include "remoteactivatable.h"
#include "remotewirelessinterfaceconnection.h"
#include "remotewirelessnetwork.h"

class ConnectionItem : public QObject
{
    Q_OBJECT

public:
    ConnectionItem(RemoteActivatable *activatable, bool hidden = false, QObject *parent = 0);
    QString deviceUni();
    QString activatableType();
    bool isShared();
    QString ssid();
    QString wiredName();
    bool hidden();
    int signalStrength();
    bool connected();
    QString connectionUuid();
    RemoteInterfaceConnection* interfaceConnection() const;
    RemoteActivatable* activatable() const;
    bool equals(const ConnectionItem *item);
    void disconnect();
    void connectNetwork();
    QString status();
    QString protectedIcon();
    QString connectionType();

protected Q_SLOTS:
    void handlePropertiesChanges(int strength);
    void activationStateChanged(Knm::InterfaceConnection::ActivationState oldState, Knm::InterfaceConnection::ActivationState newState);
    void notifyNetworkingState();

Q_SIGNALS:
    void itemChanged();

private:
    RemoteActivatable *m_activatable;
    bool m_connected;
    QString m_status;
    QString m_type;
    bool m_hidden;
};

#endif
