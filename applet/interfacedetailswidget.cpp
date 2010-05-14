/*
Copyright 2010 Sebastian Kügler <sebas@kde.org>
Copyright 2010 Alexander Naumov <posix.ru@gmail.com>

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

// own
#include <interfacedetailswidget.h>

// Qt
//#include <QGraphicsLinearLayout>
#include <QGridLayout>
//#include <QGraphicsLayout>
#include <QLabel>

// KDE
#include <KGlobalSettings>
#include <kdebug.h>

// Plasma
#include <Plasma/DataEngineManager>
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/SignalPlotter>

//Solid
#include <solid/control/wirelessaccesspoint.h>
#include <solid/control/wirelessnetworkinterface.h>
#include <solid/control/wirednetworkinterface.h>
#include <solid/control/networkipv4config.h>
#include <solid/control/networkinterface.h>
#include <solid/control/networkmanager.h>
#include <Solid/Device>

#include <uiutils.h>

#include "interfaceitem.h"

class InterfaceItem;

InterfaceDetailsWidget::InterfaceDetailsWidget(QGraphicsItem * parent) : QGraphicsWidget(parent, 0),
    m_iface(0)
{
    m_gridLayout = new QGraphicsGridLayout(this);

    int row = 0;

    //Interface
    m_interfaceNameLabel = new Plasma::Label(this);
    m_interfaceNameLabel->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    //m_interfaceNameLabel->setText("Interface:");
    m_gridLayout->addItem(m_interfaceNameLabel, row, 0, 1, 2);

    //Type
    row++;
    m_typeLabel = new Plasma::Label(this);
    m_typeLabel->setText(i18nc("interface details", "Interface Type:"));
    m_typeLabel->setAlignment(Qt::AlignRight);
    // FIXME 4.5: setWordWrap() is now a method in Plasma::Label, we're keeping this
    // for a while for backwards compat though. Remove the nativeWidget() call in between
    // when we depend on 4.5
    m_typeLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_typeLabel, row, 0);

    m_type = new Plasma::Label(this);
    m_type->setText("Type details!");
    m_type->nativeWidget()->setWordWrap(false);
    m_type->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_type, row, 1, Qt::AlignTop);


    //State
    row++;
    m_stateLabel = new Plasma::Label(this);
    m_stateLabel->setText(i18nc("interface details", "Connection State:"));
    m_stateLabel->setAlignment(Qt::AlignRight);
    m_stateLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_stateLabel, row, 0);

    m_state = new Plasma::Label(this);
    m_state->setText("State details!");
    m_state->nativeWidget()->setWordWrap(false);
    m_state->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_state, row, 1, 1, 2, Qt::AlignTop);

    //IP
    row++;
    m_ipLabel = new Plasma::Label(this);
    m_ipLabel->setText(i18nc("interface details", "Network Address (IP):"));
    m_ipLabel->setAlignment(Qt::AlignRight);
    m_ipLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_ipLabel, row, 0);

    m_ip = new Plasma::Label(this);
    m_ip->setText("IP details!");
    m_ip->nativeWidget()->setWordWrap(false);
    m_ip->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_ip, row, 1, 1, 2, Qt::AlignTop);

    //Bit
    row++;
    m_bitLabel = new Plasma::Label(this);
    m_bitLabel->setText(i18nc("interface details", "Connection Speed:"));
    m_bitLabel->setAlignment(Qt::AlignRight);
    m_bitLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_bitLabel, row, 0);

    m_bit = new Plasma::Label(this);
    m_bit->setText("Bit details!");
    m_bit->nativeWidget()->setWordWrap(false);
    m_bit->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_bit, row, 1, 1, 2, Qt::AlignTop);

    //Interface
    row++;
    m_interfaceLabel = new Plasma::Label(this);
    m_interfaceLabel->setText(i18nc("interface details", "System Name:"));
    m_interfaceLabel->setAlignment(Qt::AlignRight);
    m_interfaceLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_interfaceLabel, row, 0);

    m_interface = new Plasma::Label(this);
    m_interface->setText("Interface details!");
    m_interface->nativeWidget()->setWordWrap(false);
    m_interface->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_interface, row, 1, 1, 2, Qt::AlignTop);

    //MAC
    row++;
    m_macLabel = new Plasma::Label(this);
    m_macLabel->setText(i18nc("interface details", "Hardware Address (MAC):"));
    m_macLabel->setAlignment(Qt::AlignRight);
    m_macLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_macLabel, row, 0);

    m_mac = new Plasma::Label(this);
    m_mac->setText("MAC details!");
    m_mac->nativeWidget()->setWordWrap(false);
    m_mac->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_mac, row, 1, 1, 2, Qt::AlignTop);

    //Driver
    row++;
    m_driverLabel = new Plasma::Label(this);
    m_driverLabel->setText(i18nc("interface details", "Driver:"));
    m_driverLabel->setAlignment(Qt::AlignRight);
    m_driverLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_driverLabel, row, 0);

    m_driver = new Plasma::Label(this);
    m_driver->setText("Driver details!");
    m_driver->nativeWidget()->setWordWrap(false);
    m_driver->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_driver, row, 1, 1, 2, Qt::AlignTop);

    // Traffic
    row++;
    m_trafficLabel = new Plasma::Label(this);
    m_trafficLabel->setText(i18nc("traffic details", "Traffic in:\nTraffic out:"));
    m_trafficLabel->setAlignment(Qt::AlignRight);
    m_trafficLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_trafficLabel, row, 0);

    m_traffic = new Plasma::Label(this);
    m_traffic->setText("traffic details!");
    m_traffic->nativeWidget()->setWordWrap(false);
    m_traffic->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_traffic, row, 1, 1, 2, Qt::AlignTop);

    // Traffic plotter
    row++;
    m_trafficPlotter = new Plasma::SignalPlotter(this);
    m_trafficPlotter->addPlot(QColor("green")); // receiver
    m_trafficPlotter->addPlot(QColor("#F1DE3C")); // transmitter
    m_trafficPlotter->setMinimumHeight(80);
    m_trafficPlotter->setFont(KGlobalSettings::smallestReadableFont());
    m_trafficPlotter->setThinFrame(false);
    m_trafficPlotter->setShowLabels(true);
    m_trafficPlotter->setShowTopBar(false);
    m_trafficPlotter->setShowVerticalLines(false);
    m_trafficPlotter->setShowHorizontalLines(true);
    m_trafficPlotter->setHorizontalLinesCount(2);
    m_trafficPlotter->setUseAutoRange(true);
    m_trafficPlotter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_gridLayout->addItem(m_trafficPlotter, row, 0, 1, 3);

    Plasma::IconWidget* back = new Plasma::IconWidget(this);
    back->setIcon("go-previous");
    back->setMaximumSize(QSize(16, 16));
    m_gridLayout->addItem(back, 0, 2, 2, 1);
    setLayout(m_gridLayout);
    connect(back, SIGNAL(clicked()), this, SIGNAL(back()));

    // Add spacer to push content to the top
    row++;
    //QGraphicsWidget *spacer = new QGraphicsWidget(this);
    //spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    //m_gridLayout->addItem(spacer, row, 0);

    Plasma::DataEngine *e = Plasma::DataEngineManager::self()->loadEngine("systemmonitor");

    //connect(e, SIGNAL(sourceAdded(const QString&)), this, SLOT(sourceAdded(const QString&)));
}

void InterfaceDetailsWidget::sourceAdded(const QString &source)
{
    //kDebug() << "Source added:" << source;
}

InterfaceDetailsWidget::~InterfaceDetailsWidget()
{
}

void InterfaceDetailsWidget::setUpdateEnabled(bool enable)
{
    // disconnect / connect goes here
    Plasma::DataEngine *e = engine();
    if (e) {
        int interval = 1000;
        if (enable) {
            kDebug() << "connecting ..." << m_rxSource << m_txSource;
            e->connectSource(m_rxSource, this, interval);
            e->connectSource(m_txSource, this, interval);
        } else {
            kDebug() << "disconnecting ..." << m_rxSource << m_txSource;
            e->disconnectSource(m_rxSource, this);
            e->disconnectSource(m_txSource, this);
        }
    }
    m_updateEnabled = enable;
}

void InterfaceDetailsWidget::updateWidgets()
{
    QString s = i18nc("traffic, e.g. n KB/s\n m KB/s", "%1 %2\n%3 %4", m_rx, m_rxUnit, m_tx, m_txUnit);
    //kDebug() << s;
    m_traffic->setText(s);

    QList<double> v;
    v << m_rx.toDouble() << m_tx.toDouble();
    m_trafficPlotter->addSample(v);
    m_trafficPlotter->setUnit(m_rxUnit);
    m_trafficPlotter->setTitle(i18nc("traffic, e.g. n KB/s / m KB/s", "%1 %2\n%3 %4", m_rx, m_rxUnit, m_tx, m_txUnit));
}

Plasma::DataEngine* InterfaceDetailsWidget::engine()
{
    Plasma::DataEngine *e = Plasma::DataEngineManager::self()->engine("systemmonitor");


    if (e->isValid()) {
        kDebug() << "engine loaded. :-)";
    } else {
        kDebug() << "engine NOT loaded. )-:";
    }
    return e;

}

void InterfaceDetailsWidget::dataUpdated(const QString &sourceName, const Plasma::DataEngine::Data &data)
{
    //kDebug() << "Source Updated!!!" << sourceName << data;
    if (sourceName == m_txSource) {
        m_tx = data["value"].toString();
        m_txUnit = data["units"].toString();
    } else if (sourceName == m_rxSource) {
        m_rx = data["value"].toString();
        m_rxUnit = data["units"].toString();
    }
    //QString trafficInfo = i18nc("traffic status in details widget", "In: %1%2\nOut: ...", m_tx, m_txUnit);
    updateWidgets();
}

void InterfaceDetailsWidget::setInterface(Solid::Control::NetworkInterface* iface)
{
    if (iface) {
        m_iface = iface;
        m_interfaceNameLabel->setText(QString("<b>%1</b>").arg(UiUtils::interfaceNameLabel(iface->uni())));
        m_interface->setText(m_iface->interfaceName());
        m_driver->setText(iface->driver());
        //m_speed->setText(QString::number(iface->designSpeed()));
        m_type->setText(UiUtils::interfaceTypeLabel(iface->type()));
        m_state->setText(UiUtils::connectionStateToString(iface->connectionState()));

        m_rxSource = QString("network/interfaces/%1/receiver/data").arg(m_iface->interfaceName());
        m_txSource = QString("network/interfaces/%1/transmitter/data").arg(m_iface->interfaceName());
    }
    /*
    Solid::Device *dev = new Solid::Device(iface->uni());
    kDebug() << "IFACE:" << dev->vendor();
    kDebug() << "product:" << dev->product();
    kDebug() << "udi:" << dev->udi();
    kDebug() << "desc:" << dev->description();
    kDebug() << "icon:" << dev->icon();
    */
}

void InterfaceDetailsWidget::setMAC(Solid::Control::NetworkInterface* iface)
{
    QString temp;
    int bitRate = 0;
    QString bit = i18nc("bitrate", "Unknown");

    //wifi?
    Solid::Control::WirelessNetworkInterface * wliface =
                    dynamic_cast<Solid::Control::WirelessNetworkInterface*>(iface);
    if (wliface) {
        m_mac->setText(wliface->hardwareAddress());
        bitRate = wliface->bitRate(); //Bit
        /*
         * for later use ...
        Solid::Control::AccessPoint * ap = wliface->findAccessPoint(wliface->activeAccessPoint());
        if(ap) {
            temp = ap->ssid();
            kDebug() << "temp = " << temp;
        }
        */
    } else {     // wired?
        Solid::Control::WiredNetworkInterface * wdiface =
                                    dynamic_cast<Solid::Control::WiredNetworkInterface*> (iface);
        if (wdiface) {
            temp = wdiface->hardwareAddress();
            m_mac->setText(temp);
            bitRate = wdiface->bitRate();
            //m_bit->setText(QString::number(bitRate));
        }
        // NOTE: handle other kinds of interfaces as well here, for example GSM or bluetooth
    }
    if (bitRate) {
        m_bit->setText(UiUtils::connectionSpeed(bitRate));
    } else {
        m_bit->setText(i18nc("bitrate", "Unknown"));
    }
}

void InterfaceDetailsWidget::setIP(QString ip)
{
    m_ip->setText(ip);
}

// vim: sw=4 sts=4 et tw=100
