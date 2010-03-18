/*
Copyright 2009 Will Stephenson <wstephenson@kde.org>

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

#include "activatableitem.h"
#include "activatableitem_p.h"

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QStyleOptionMenuItem>

#include <KDebug>
#include <KGlobalSettings>
#include <KIconLoader>

ActivatableItemPrivate::ActivatableItemPrivate()
: hovered(false), first(false)
{
}

IconSizedSpacer::IconSizedSpacer(QWidget * parent) : QWidget(parent)
{
}

IconSizedSpacer::~IconSizedSpacer()
{
}

QSize IconSizedSpacer::sizeHint() const
{
    int xSize = KIconLoader::global()->currentSize(KIconLoader::Panel);
    return QSize(xSize, 1);
}

void ActivatableItemPrivate::init(QWidget * widgetParent)
{
    outerLayout = new QGridLayout(widgetParent);
    outerLayout->setObjectName("activatableitem_outerlayout");
    innerLayout = new QHBoxLayout();
    innerLayout->setObjectName("activatableitem_innerlayout");
    activeIcon = new QLabel(widgetParent);
    mainLabel = new QLabel(widgetParent);
    spacer = new IconSizedSpacer(widgetParent);
    mainLabel->setFont(KGlobalSettings::toolBarFont());

    outerLayout->addLayout(innerLayout, 0, 1);
    outerLayout->addWidget(activeIcon, 0, 0, 2, 1);
    outerLayout->addWidget(spacer, 0, 0, 2, 1);
    activeIcon->hide();
    innerLayout->addWidget(mainLabel, 1, Qt::AlignLeft);
}

ActivatableItemPrivate::~ActivatableItemPrivate()
{
}

ActivatableItem::ActivatableItem(ActivatableItemPrivate & dd, Knm::Activatable * activatable, QWidget * parent)
: QAbstractButton(parent), d_ptr(&dd)
{
    Q_D(ActivatableItem);
    d->init(this);
    d->activatable = activatable;
    QObject::connect(this, SIGNAL(clicked()), d->activatable, SLOT(activate()));
}

ActivatableItem::ActivatableItem(Knm::Activatable * activatable, QWidget * parent)
: QAbstractButton(parent), d_ptr(new ActivatableItemPrivate)
{
    Q_D(ActivatableItem);
    d->init(this);
    d->activatable = activatable;
    QObject::connect(this, SIGNAL(clicked()), d->activatable, SLOT(activate()));
}

ActivatableItem::~ActivatableItem()
{
    delete d_ptr;
}

void ActivatableItem::setFirst(bool first)
{
    Q_D(ActivatableItem);
    d->first = first;
    setFirstInternal(first);
}

void ActivatableItem::setFirstInternal(bool first)
{
    Q_D(ActivatableItem);
    if (first) {
        d->mainLabel->setFont(KGlobalSettings::menuFont());
        d->activeIcon->setPixmap(pixmap());
        d->activeIcon->show();
    } else {
        d->mainLabel->setFont(KGlobalSettings::toolBarFont());
        d->activeIcon->hide();
    }
}

void ActivatableItem::setText(const QString & text)
{
    Q_D(ActivatableItem);
    d->mainLabel->setText(text);
}

void ActivatableItem::addIcon(QWidget * iconWidget)
{
    Q_D(ActivatableItem);
    d->innerLayout->addWidget(iconWidget, 0, Qt::AlignRight);
}

void ActivatableItem::removeIcon(QWidget * iconWidget)
{
    Q_D(ActivatableItem);
    d->innerLayout->removeWidget(iconWidget);
}

Knm::Activatable * ActivatableItem::activatable() const
{
    Q_D(const ActivatableItem);
    return d->activatable;
}

QPixmap ActivatableItem::pixmap() const
{
    Q_D(const ActivatableItem);
    return KIconLoader::global()->loadIcon(iconName(), (d->first ? KIconLoader::Panel : KIconLoader::Small));
}

void ActivatableItem::enterEvent(QEvent *)
{
    Q_D(ActivatableItem);
    d->hovered = true;
    repaint();
}

void ActivatableItem::leaveEvent(QEvent *)
{
    Q_D(ActivatableItem);
    d->hovered = false;
    repaint();
}

void ActivatableItem::paintEvent(QPaintEvent * event)
{
    Q_D(ActivatableItem);
    QPainter p(this);
    QStyleOptionMenuItem opt;
    opt.initFrom(this);
    if (d->hovered) {
        opt.state |= QStyle::State_Selected;
    }
    opt.rect = rect();
    style()->drawControl(QStyle::CE_MenuItem, &opt, &p, this);

    QWidget::paintEvent(event);
}

// vim: sw=4 sts=4 et tw=100
