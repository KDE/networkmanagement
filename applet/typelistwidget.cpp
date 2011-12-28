/*
Copyright 2011 Lamarque Souza <lamarque@kde.org>

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
#include "typelistwidget.h"

// Qt
#include <QGridLayout>
#include <QLabel>
#include <QGraphicsGridLayout>

// KDE
#include <kdebug.h>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/Label>
#include <Plasma/PushButton>

#include <uiutils.h>

TypeListWidget::TypeListWidget(QGraphicsItem * parent) : QGraphicsWidget(parent, 0)
{
    QGraphicsGridLayout * layout = new QGraphicsGridLayout(this);

    Plasma::Label * title = new Plasma::Label(this);
    title->setText(i18nc("title for connect to another network window", "You want to connect to a network. We see the following options for you:"));
    layout->addItem(title, 0, 0);

    // add pushbutton for "back" action
    m_backButton = new Plasma::PushButton(this);
    m_backButton->setMaximumHeight(22);
    m_backButton->setMaximumWidth(22);
    m_backButton->setIcon(KIcon("go-previous"));
    m_backButton->setToolTip(i18n("Go Back"));
    layout->addItem(m_backButton, 0, 1);
    connect(m_backButton, SIGNAL(clicked()), this, SIGNAL(back()));

    setLayout(layout);
}

TypeListWidget::~TypeListWidget()
{
}
// vim: sw=4 sts=4 et tw=100
