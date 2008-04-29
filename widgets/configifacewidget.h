/*  This file is part of the KDE project
    Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef CONFIGIFACEWIDGET_H
#define CONFIGIFACEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QButtonGroup>
#include <QPushButton>
#include <QGroupBox>

#include <KConfig>
#include <klocalizedstring.h>

class ConfigIfaceWidget : public QWidget
{
    Q_OBJECT

    public:
        enum Type {UnknownType=0, Vpn, Ieee8023, Ieee80211};
        
        ConfigIfaceWidget(QWidget *parent=0);
        ~ConfigIfaceWidget();

        KConfig* config() const;
        void setConfig(KConfig *config);
        virtual Type ifaceType() const = 0;

    protected:
        void layoutIpWidget();
        
        KConfig *m_config;
        QGroupBox *m_ipGroupBox;
        QVBoxLayout *m_mainLayout, *m_ipLayout;
        QButtonGroup *m_ipGroup;
        QRadioButton *m_dynamicButton, *m_staticButton;
        QLabel *m_ipLabel, *m_netmaskLabel, *m_broadcastLabel;
        QLineEdit *m_ipAddressEdit, *m_netmaskEdit, *m_broadcastEdit;
};

class WiredConfigIfaceWidget : public ConfigIfaceWidget
{
    Q_OBJECT

    public:
        WiredConfigIfaceWidget(QWidget *parent=0);
        ~WiredConfigIfaceWidget();

        Type ifaceType() const;
};

class WifiConfigIfaceWidget : public ConfigIfaceWidget
{
    Q_OBJECT

    public:
        WifiConfigIfaceWidget(QWidget *parent=0);
        ~WifiConfigIfaceWidget();

        Type ifaceType() const;

    private:
        QGroupBox *m_wifiGroupBox;
        QVBoxLayout *m_wifiLayout;
        QButtonGroup *m_wifiGroup;
        QRadioButton *m_anyButton, *m_specificButton;
        QLabel *m_essidLabel;
        QLineEdit *m_essidEdit;
        QPushButton *m_essidButton;
};

#endif

