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

#ifndef ACTIVATABLEITEM_H
#define ACTIVATABLEITEM_H

#include <QAbstractButton>
#include <QSize>

namespace Knm
{
class Activatable;
}

class ActivatableItemPrivate;

/*
 * This class is used in items that do not have a visible icon 
 * to take up the same amount of space as the icon, so the 
 * labels appear at the same place as those items with icons
 */
class IconSizedSpacer : public QWidget
{
Q_OBJECT
public:
    IconSizedSpacer(QWidget * parent = 0);
    virtual ~IconSizedSpacer();
    virtual QSize sizeHint() const;
};

class ActivatableItem : public QAbstractButton
{
Q_OBJECT
public:
    ActivatableItem(Knm::Activatable *, QWidget * parent = 0);
    virtual ~ActivatableItem();

    /**
     * Set as the first item of this type, giving extra visual emphasis
     */
    void setFirst(bool first);

    /**
     * Set the main text, use for connection names
     */
    void setText(const QString & text);

    /**
     * Adds a subwidget at the right edge of the main layout
     */
    void addIcon(QWidget *);
    /**
     * Remove an icon from the right side
     */
    void removeIcon(QWidget *);

    /**
     * Return the current icon name to use
     */
    virtual QString iconName() const = 0;

    /**
     * Access the Activatable this ActivatableItem represents
     */
    Knm::Activatable * activatable() const;
protected:
    /**
     * Allows subclasses to update their appearance when first
     */
    virtual void setFirstInternal(bool first);
    virtual QPixmap pixmap() const;
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);
    virtual void paintEvent(QPaintEvent *);
    ActivatableItem(ActivatableItemPrivate &, Knm::Activatable *, QWidget * parent);
    Q_DECLARE_PRIVATE(ActivatableItem)
    ActivatableItemPrivate * d_ptr;
};

#endif // ACTIVATABLEITEM_H
