/*
Copyright 2008 Helmut Schaa <helmut.schaa@googlemail.com>

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

#include <QWidget>

#include <KDebug>

#include "wpaeapwidget.h"

#include <nm-setting-wireless-security.h>
#include <nm-setting-8021x.h>

#include "802_11_wireless_security_widget.h"
#include "ui_wpaeap.h"
#include "peapwidget.h"
#include "ttlswidget.h"
#include "tlswidget.h"
#include "connection.h"
#include "settings/802-11-wireless-security.h"
#include "settings/802-1x.h"

class WpaEapWidget::Private
{
public:
    Ui_WpaEap ui;

    QHash<int, EapWidget*> eapWidgetHash;

    int peapIndex;
    int ttlsIndex;
    int tlsIndex;

    Knm::WirelessSecuritySetting* settingSec;
    Knm::Security8021xSetting* setting8021x;
};

WpaEapWidget::WpaEapWidget(Knm::Connection* connection, QWidget * parent)
: SecurityWidget(connection, parent)
, d(new WpaEapWidget::Private)
{
    d->ui.setupUi((SecurityWidget*)this);

    d->settingSec = static_cast<Knm::WirelessSecuritySetting *>(connection->setting(Knm::Setting::WirelessSecurity));
    d->setting8021x = static_cast<Knm::Security8021xSetting *>(connection->setting(Knm::Setting::Security8021x));

    // we have to be careful here as we deal with two settings objects.
    // the eap widgets need the 802.1x setting as KConfig attribute

    int index = 0;

    EapWidget* ew = new PeapWidget(connection, d->ui.stackedWidget);
    d->eapWidgetHash.insert(index, ew);
    d->ui.stackedWidget->insertWidget(index, ew);
    d->peapIndex = index++;

    ew = new TtlsWidget(connection, d->ui.stackedWidget);
    d->eapWidgetHash.insert(index, ew);
    d->ui.stackedWidget->insertWidget(index, ew);
    d->ttlsIndex = index++;

    ew = new TlsWidget(connection, d->ui.stackedWidget);
    d->eapWidgetHash.insert(index, ew);
    d->ui.stackedWidget->insertWidget(index, ew);
    d->tlsIndex = index++;

    // TODO: remove TLS for now as TLS is not ready yet
    d->ui.cboEAPMethod->removeItem(d->tlsIndex);

    methodChanged(d->ui.cboEAPMethod->currentIndex());

    SecurityWidget::connect(d->ui.cboEAPMethod, SIGNAL(currentIndexChanged(int)), (SecurityWidget*)this, SLOT(methodChanged(int)));
}

WpaEapWidget::~WpaEapWidget()
{
    delete d;
}

QWidget* WpaEapWidget::widget()
{
    return this;
}

void WpaEapWidget::methodChanged(int index)
{
    d->ui.stackedWidget->setCurrentWidget(d->eapWidgetHash.value(index));
}

bool WpaEapWidget::validate() const
{
    return true;
}

void WpaEapWidget::readConfig()
{
    // This is WPA EAP, we have to handle
    // 1. the wireless security setting called by the wireless security top level widget
    // 2. the 802.1x setting called directly by the KCModule
    // As we cannot distinquish the callers just handle both settings

    Knm::Security8021xSetting::EapMethods eap = d->setting8021x->eapFlags();

    // default is peap
    EapWidget * ew = d->eapWidgetHash.value(d->peapIndex);
    d->ui.cboEAPMethod->setCurrentIndex(d->peapIndex);

    kDebug() << "test " << eap;

    if (eap.testFlag(Knm::Security8021xSetting::ttls))
    {
        d->ui.cboEAPMethod->setCurrentIndex(d->ttlsIndex);
        ew = d->eapWidgetHash.value(d->ttlsIndex);
    } else if (eap.testFlag(Knm::Security8021xSetting::tls))
    {
        d->ui.cboEAPMethod->setCurrentIndex(d->tlsIndex);
        ew = d->eapWidgetHash.value(d->tlsIndex);
    } else if (eap.testFlag(Knm::Security8021xSetting::peap))
    {
        d->ui.cboEAPMethod->setCurrentIndex(d->peapIndex);
        ew = d->eapWidgetHash.value(d->peapIndex);
    }
    ew->readConfig();
    d->ui.stackedWidget->setCurrentWidget(ew);

}

void WpaEapWidget::writeConfig()
{
    // This is WPA EAP, we have to handle
    // 1. the wireless security setting
    // 2. the 802.1x setting
    kDebug() << "write eap settings" ;

    EapWidget * ew = d->eapWidgetHash.value(d->ui.cboEAPMethod->currentIndex());
    if (ew) {
        ew->writeConfig();
    }

    switch(d->ui.cboEAPMethod->currentIndex())
    {
        case 0:
            d->setting8021x->setEapFlags(Knm::Security8021xSetting::peap);
            break;
        case 1:
            d->setting8021x->setEapFlags(Knm::Security8021xSetting::ttls);
            break;
        case 2:
            d->setting8021x->setEapFlags(Knm::Security8021xSetting::tls);
            break;
    }
    d->setting8021x->setEnabled(true);
}


void WpaEapWidget::readSecrets()
{
    Knm::Security8021xSetting::EapMethods eap = d->setting8021x->eapFlags();

    // default is peap
    EapWidget * ew = d->eapWidgetHash.value(d->peapIndex);

    if (eap.testFlag(Knm::Security8021xSetting::ttls))
    {
        d->ui.cboEAPMethod->setCurrentIndex(d->ttlsIndex);
        ew = d->eapWidgetHash.value(d->ttlsIndex);
    } else if (eap.testFlag(Knm::Security8021xSetting::tls))
    {
        d->ui.cboEAPMethod->setCurrentIndex(d->tlsIndex);
        ew = d->eapWidgetHash.value(d->tlsIndex);
    } else if (eap.testFlag(Knm::Security8021xSetting::peap))
    {
        d->ui.cboEAPMethod->setCurrentIndex(d->peapIndex);
        ew = d->eapWidgetHash.value(d->peapIndex);
    }
    ew->readSecrets();
}

#include "wpaeapwidget.moc"
// vim: sw=4 sts=4 et tw=100
