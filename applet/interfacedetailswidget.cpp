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
#include <QMetaProperty>

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
    //Type
    //row++;
    m_typeLabel = new Plasma::Label(this);
    m_typeLabel->setText(i18nc("interface details", "Type:"));
    m_typeLabel->setAlignment(Qt::AlignRight);
    m_typeLabel->setFont(KGlobalSettings::smallestReadableFont());
    // FIXME 4.5: setWordWrap() is now a method in Plasma::Label, we're keeping this
    // for a while for backwards compat though. Remove the nativeWidget() call in between
    // when we depend on 4.5
    m_typeLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_typeLabel, row, 0);

    m_type = new Plasma::Label(this);
    m_type->nativeWidget()->setWordWrap(false);
    m_type->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_type->setFont(KGlobalSettings::smallestReadableFont());
    m_gridLayout->addItem(m_type, row, 1, Qt::AlignTop);


    //State
    row++;
    m_stateLabel = new Plasma::Label(this);
    m_stateLabel->setText(i18nc("interface details", "Connection State:"));
    m_stateLabel->setAlignment(Qt::AlignRight);
    m_stateLabel->nativeWidget()->setWordWrap(false);
    m_stateLabel->setFont(KGlobalSettings::smallestReadableFont());
    m_gridLayout->addItem(m_stateLabel, row, 0);

    m_state = new Plasma::Label(this);
    m_state->nativeWidget()->setWordWrap(false);
    m_state->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_state->setFont(KGlobalSettings::smallestReadableFont());
    m_gridLayout->addItem(m_state, row, 1, 1, 2, Qt::AlignTop);

    //IP
    row++;
    m_ipLabel = new Plasma::Label(this);
    m_ipLabel->setText(i18nc("interface details", "Network Address (IP):"));
    m_ipLabel->setAlignment(Qt::AlignRight);
    m_ipLabel->nativeWidget()->setWordWrap(false);
    m_ipLabel->setFont(KGlobalSettings::smallestReadableFont());
    m_gridLayout->addItem(m_ipLabel, row, 0);

    m_ip = new Plasma::Label(this);
    m_ip->nativeWidget()->setWordWrap(false);
    m_ip->setFont(KGlobalSettings::smallestReadableFont());
    m_ip->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_ip, row, 1, 1, 2, Qt::AlignTop);

    //Bit
    row++;
    m_bitLabel = new Plasma::Label(this);
    m_bitLabel->setText(i18nc("interface details", "Connection Speed:"));
    m_bitLabel->setAlignment(Qt::AlignRight);
    m_bitLabel->setFont(KGlobalSettings::smallestReadableFont());
    m_bitLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_bitLabel, row, 0);

    m_bit = new Plasma::Label(this);
    m_bit->nativeWidget()->setWordWrap(false);
    m_bit->setFont(KGlobalSettings::smallestReadableFont());
    m_bit->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_bit, row, 1, 1, 2, Qt::AlignTop);

    //Interface
    row++;
    m_interfaceLabel = new Plasma::Label(this);
    m_interfaceLabel->setText(i18nc("interface details", "System Name:"));
    m_interfaceLabel->setAlignment(Qt::AlignRight);
    m_interfaceLabel->nativeWidget()->setWordWrap(false);
    m_interfaceLabel->setFont(KGlobalSettings::smallestReadableFont());
    m_gridLayout->addItem(m_interfaceLabel, row, 0);

    m_interface = new Plasma::Label(this);
    m_interface->nativeWidget()->setWordWrap(false);
    m_interface->setFont(KGlobalSettings::smallestReadableFont());
    m_interface->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_interface, row, 1, 1, 2, Qt::AlignTop);

    //MAC
    row++;
    m_macLabel = new Plasma::Label(this);
    m_macLabel->setText(i18nc("interface details", "Hardware Address (MAC):"));
    m_macLabel->setAlignment(Qt::AlignRight);
    m_macLabel->setFont(KGlobalSettings::smallestReadableFont());
    m_macLabel->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_macLabel, row, 0);

    m_mac = new Plasma::Label(this);
    m_mac->nativeWidget()->setWordWrap(false);
    m_mac->setFont(KGlobalSettings::smallestReadableFont());
    m_mac->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_mac, row, 1, 1, 2, Qt::AlignTop);

    //Driver
    row++;
    m_driverLabel = new Plasma::Label(this);
    m_driverLabel->setText(i18nc("interface details", "Driver:"));
    m_driverLabel->setAlignment(Qt::AlignRight);
    m_driverLabel->nativeWidget()->setWordWrap(false);
    m_driverLabel->setFont(KGlobalSettings::smallestReadableFont());
    m_gridLayout->addItem(m_driverLabel, row, 0);

    m_driver = new Plasma::Label(this);
    m_driver->nativeWidget()->setWordWrap(false);
    m_driver->setFont(KGlobalSettings::smallestReadableFont());
    m_driver->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_driver, row, 1, 1, 2, Qt::AlignTop);

    // Traffic

    //Interface
    row++;
    m_trafficNameLabel = new Plasma::Label(this);
    m_trafficNameLabel->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_trafficNameLabel->setText(i18nc("interface details", "<b>Traffic</b>"));
    m_gridLayout->addItem(m_trafficNameLabel, row, 0, 1, 2);
    
    // Traffic plotter
    row++;
    m_rxColor = QColor("#0099FF"); // blue
    //m_rxColor = QColor("lightblue"); // blue
    //m_rxColor.setAlphaF(0.6);
    m_txColor = QColor("#91FF00"); // yellow
    //m_txColor = QColor("lightgreen"); // yellow
    m_txColor.setAlphaF(0.6);
    m_trafficPlotter = new Plasma::SignalPlotter(this);
    m_trafficPlotter->setMinimumHeight(50);
    m_trafficPlotter->setFont(KGlobalSettings::smallestReadableFont());
    m_trafficPlotter->addPlot(m_rxColor); // receiver green
    m_trafficPlotter->addPlot(m_txColor); // transmitter yellow
    m_trafficPlotter->setThinFrame(true);
    m_trafficPlotter->setShowLabels(true);
    m_trafficPlotter->setShowTopBar(true);
    m_trafficPlotter->setShowVerticalLines(false);
    m_trafficPlotter->setShowHorizontalLines(true);
    m_trafficPlotter->setHorizontalLinesCount(2);
    m_trafficPlotter->setUseAutoRange(true);
    m_trafficPlotter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_gridLayout->addItem(m_trafficPlotter, row, 0, 1, 3);

    row++;
    m_trafficRx = new Plasma::Label(this);
    m_trafficRx->setAlignment(Qt::AlignRight);
    m_trafficRx->setFont(KGlobalSettings::smallestReadableFont());
    m_trafficRx->nativeWidget()->setWordWrap(false);
    m_gridLayout->addItem(m_trafficRx, row, 0);

    m_trafficTx = new Plasma::Label(this);
    m_trafficTx->nativeWidget()->setWordWrap(false);
    m_trafficTx->setFont(KGlobalSettings::smallestReadableFont());
    m_trafficTx->nativeWidget()->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_gridLayout->addItem(m_trafficTx, row, 1, 1, 2, Qt::AlignTop);

    // add pushbutton for "back" action
    m_backButton = new Plasma::PushButton(this);
    m_backButton->setMaximumHeight(22);
    m_backButton->setMaximumWidth(22);
    m_backButton->setIcon(KIcon("go-previous"));
    m_backButton->setToolTip(i18n("back"));

    connect(m_backButton, SIGNAL(clicked()), this, SIGNAL(back()));

    m_gridLayout->addItem(m_backButton, 0, 2, 2, 1);
    setLayout(m_gridLayout);

    // Add spacer to push content to the top
    row++; 
    //QGraphicsWidget *spacer = new QGraphicsWidget(this);
    //spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    //m_gridLayout->addItem(spacer, row, 0);

    //resetUi();

    Plasma::DataEngineManager::self()->loadEngine("systemmonitor");

    //connect(e, SIGNAL(sourceAdded(const QString&)), this, SLOT(sourceAdded(const QString&)));
}

void InterfaceDetailsWidget::resetUi()
{

    QString na = i18nc("entry not available", "not available");

    m_type->setText(na);
    m_state->setText(na);
    m_ip->setText(na);
    m_bit->setText(na);
    m_interface->setText(na);
    m_mac->setText(na);
    m_driver->setText(na);

    m_trafficRx->setText(i18nc("traffic received empty", "Received: -"));
    m_trafficTx->setText(i18nc("traffic transmitted empty", "Transmitted: -"));

    // Quite ugly, but I need to investigate why I'm getting those crashes after removePlot calls
    for (int i = 0; i < 500; i++) {
        QList<double> v;
        v << 0 << 0;
        m_trafficPlotter->addSample(v);
    }
    /*
    kDebug() << m_trafficPlotter->plotColors().count() << "clrs, now removing";
    m_trafficPlotter->removePlot(0);
    m_trafficPlotter->removePlot(1);
    kDebug() << "Now:" << m_trafficPlotter->plotColors().count();
    m_trafficPlotter->addPlot(m_rxColor); // receiver green
    m_trafficPlotter->addPlot(m_txColor); // transmitter yellow
    kDebug() << "and adding back" << m_trafficPlotter->plotColors().count();
    */
}

void InterfaceDetailsWidget::adjustTrafficPlotterHeight()
{
    m_trafficPlotter->setMinimumHeight(m_trafficPlotter->size().rwidth() / 3);
}

void InterfaceDetailsWidget::sourceAdded(const QString &source)
{
    kDebug() << "Source added:" << source;
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
            if (m_iface) {
                kDebug() << "connecting ..." << m_rxSource << m_txSource;
                e->connectSource(m_rxSource, this, interval);
                e->connectSource(m_txSource, this, interval);
                e->connectSource(m_rxTotalSource, this, interval);
                e->connectSource(m_txTotalSource, this, interval);
            }
        } else {
            kDebug() << "disconnecting ..." << m_rxSource << m_txSource;
            e->disconnectSource(m_rxSource, this);
            e->disconnectSource(m_txSource, this);
            e->disconnectSource(m_rxTotalSource, this);
            e->disconnectSource(m_txTotalSource, this);
        }
    }
    m_updateEnabled = enable;
}

void InterfaceDetailsWidget::updateWidgets()
{
    //kDebug() << s;
    //m_traffic->setText(s);

    double _r = m_rx.toDouble();
    double _t = m_tx.toDouble();
    QList<double> v;
    v << _r << _t;
    m_trafficPlotter->addSample(v);
    m_trafficPlotter->setUnit(m_rxUnit);
    //m_trafficPlotter->setTitle(i18nc("traffic, e.g. n KB/s / m KB/s", "%1 %2 %3 %4", m_rx, m_rxUnit, m_tx, m_txUnit));

    QString r = KGlobal::locale()->formatByteSize(_r*1024);
    r.append("/s");
    QString t = KGlobal::locale()->formatByteSize(_t*1024);
    t.append("/s");
    QString s = i18nc("traffic, e.g. n KB/s\n m KB/s", "%1 %2", r, t);
    m_trafficPlotter->setTitle(s);

    m_trafficRx->setText(i18nc("", "Received: %1", KGlobal::locale()->formatByteSize(m_rxTotal*1000, 2)));
    m_trafficTx->setText(i18nc("", "Transmitted: %1", KGlobal::locale()->formatByteSize(m_txTotal*1000, 2)));
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
    } else if (sourceName == m_rxTotalSource) {
        m_rxTotal = data["value"].toString().toLong();
        QString _srx = QString("%1Total").arg(m_rxSource);
    } else if (sourceName == m_txTotalSource) {
        m_txTotal = data["value"].toString().toLong();
    }
    //QString trafficInfo = i18nc("traffic status in details widget", "In: %1%2\nOut: ...", m_tx, m_txUnit);
    updateWidgets();
}

void InterfaceDetailsWidget::setInterface(Solid::Control::NetworkInterface* iface)
{
    if (m_iface == iface) {
        return;
    }
    resetUi();
    if (iface) {
        m_iface = iface;
        //m_trafficNameLabel->setText(QString("<b>%1</b>").arg(UiUtils::interfaceNameLabel(iface->uni())));
        m_interface->setText(m_iface->interfaceName());
        m_driver->setText(iface->driver());
        //m_speed->setText(QString::number(iface->designSpeed()));
        m_type->setText(UiUtils::interfaceTypeLabel(iface->type()));
        m_state->setText(UiUtils::connectionStateToString(iface->connectionState()));

        m_rxSource = QString("network/interfaces/%1/receiver/data").arg(m_iface->interfaceName());
        m_txSource = QString("network/interfaces/%1/transmitter/data").arg(m_iface->interfaceName());
        m_rxTotalSource = QString("network/interfaces/%1/receiver/dataTotal").arg(m_iface->interfaceName());
        m_txTotalSource = QString("network/interfaces/%1/transmitter/dataTotal").arg(m_iface->interfaceName());
        setMAC(iface);
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
        } else {
            // prevent crash for unconnected devices
            if (iface) {
                QList<Solid::Device> list = Solid::Device::listFromQuery(QString::fromLatin1("NetworkInterface.ifaceName == '%1'").arg(iface->interfaceName()));
                QList<Solid::Device>::iterator it = list.begin();

                if (it != list.end()) {
                    Solid::Device device = *it;
                    Solid::DeviceInterface *interface = it->asDeviceInterface(Solid::DeviceInterface::NetworkInterface);

                    if (interface) {
                        const QMetaObject *meta = interface->metaObject();

                        for (int i = meta->propertyOffset(); i<meta->propertyCount(); i++) {
                            QMetaProperty property = meta->property(i);

                            if (QString(meta->className()).mid(7) + "." + property.name() == QString::fromLatin1("NetworkInterface.hwAddress")) {
                                QVariant value = property.read(interface);
                                m_mac->setText(value.toString());
                                break;
                            }
                        }
                    }
                }
            }
        }
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
