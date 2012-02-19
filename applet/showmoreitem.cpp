/*
Copyright 2011 Lamarque V. Souza <lamarque@kde.org>

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

// Own
#include "showmoreitem.h"

// Qt
#include <QGraphicsLinearLayout>

// KDE
#include <KDebug>
#include <KLineEdit>
#include <KLocale>

// Plasma
#include <Plasma/IconWidget>
#include <Plasma/LineEdit>

ShowMoreItem::ShowMoreItem(const int count, QGraphicsWidget *parent) : ActivatableItem(0, parent),
    m_layout(0), m_networkCount(count), m_checked(false)
{
    //kDebug() << "ShowMoreItem";
}

ShowMoreItem::~ShowMoreItem()
{
}

void ShowMoreItem::setupItem()
{
    if (m_layout) {
        return;
    }
    m_layout = new QGraphicsLinearLayout(this);
    m_connectButton = new Plasma::IconWidget(this);
    m_connectButton->setDrawBackground(false);
    m_connectButton->setOrientation(Qt::Horizontal);
    m_connectButton->setIcon("list-add");
    connect(m_connectButton, SIGNAL(activated()), SLOT(emitClicked()));
    m_layout->addItem(m_connectButton);
}

void ShowMoreItem::emitClicked()
{
    emit clicked();
}

void ShowMoreItem::setChecked(bool checked)
{
    m_checked = checked;
    if (m_checked) {
        m_connectButton->setText(i18nc("text when this item is toggled", "Show less networks..."));
        m_connectButton->setIcon("list-remove");
    } else {
        m_connectButton->setText(i18ncp("text when this item not toggled", "Show one more network", "Show %1 more networks...", m_networkCount));
        m_connectButton->setIcon("list-add");
    }
}

void ShowMoreItem::setNetworkCount(const int count)
{
    if (count < 0 ) {
        kDebug() << "count == " << count;
        return;
    }
    m_networkCount = count;
    setChecked(m_checked);
}

// vim: sw=4 sts=4 et tw=100
