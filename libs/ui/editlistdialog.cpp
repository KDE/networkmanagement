/*
Copyright 2009 Andrey Batyiev <batyiev@gmail.com>

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

#include "editlistdialog.h"

#include <KEditListWidget>
#include <KLineEdit>

void removeEmptyItems(QStringList &list);

class EditListDialog::Private
{
public:
    Private() : editListWidget(0)
    {
    }
    KEditListWidget * editListWidget;

};

EditListDialog::EditListDialog(QWidget *parent, Qt::WFlags flags) : KDialog(parent, flags), d(new EditListDialog::Private)
{
    setButtons(KDialog::Ok | KDialog::Cancel);

    d->editListWidget = new KEditListWidget(this);
    d->editListWidget->setCheckAtEntering(true);

    setMainWidget(d->editListWidget);
    connect(this, SIGNAL(okClicked()), this, SLOT(okClicked()));
}

EditListDialog::~EditListDialog()
{
    delete d;
}

void EditListDialog::setItems(const QStringList &items)
{
    d->editListWidget->setItems(items);
}

QStringList EditListDialog::items() const
{
    return d->editListWidget->items();
}

void removeEmptyItems(QStringList &list)
{
    QStringList::iterator it = list.begin();
    const QStringList::iterator end = list.end();
    while (it != end) {
        if ((*it).trimmed().isEmpty()) {
            it = list.erase(it);
        }
        else {
            it++;
        }
    }
}

void EditListDialog::okClicked()
{
    QStringList list = items();
    removeEmptyItems(list);
    emit itemsEdited(list);
}

void EditListDialog::setValidator(const QValidator *validator)
{
    d->editListWidget->lineEdit()->setValidator(validator);
}

const QValidator* EditListDialog::validator() const
{
    return d->editListWidget->lineEdit()->validator();
}

void EditListDialog::setToolTip(const QString toolTip)
{
    d->editListWidget->lineEdit()->setToolTip(toolTip);
}
