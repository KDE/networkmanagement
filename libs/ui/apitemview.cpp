/*
Copyright (C) 2008 Christopher Blauvelt <cblauvelt@gmail.com>

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

#include "apitemview.h"

#include <QApplication>
#include <QPainter>
#include <QPoint>
#include <QSize>
#include <QRect>

#include <KIcon>
#include <KDebug>

ApItemView::ApItemView(QWidget *parent)
    : QWidget(parent),
      m_model(0),
      m_delegate(0),
      m_selectionModel(0),
      m_background(0),
      m_apLayout(),
      m_options(),
      m_itemHeight(0),
      m_itemWidth(0)
{
    initStyleOptions();
}

ApItemView::~ApItemView()
{
}

void ApItemView::initStyleOptions()
{
    m_options.textElideMode = Qt::ElideRight;
    QFont itemFont = QApplication::font();
    itemFont.setPointSize(fontHeight);
    m_options.font = itemFont;
    QFontMetrics fm = QFontMetrics(itemFont);
    m_itemHeight = fm.lineSpacing() + padding;
    //width = label width + 2*(icon width + padding)
    m_itemWidth = itemCharWidth * fm.averageCharWidth() + 2*(m_itemHeight + padding);
    m_options.palette.setCurrentColorGroup(QPalette::Normal);
    m_options.decorationSize = QSize(fm.height(),fm.height());
}

void ApItemView::setModel(QAbstractItemModel *model)
{
    if (m_model == model) {
        return;
    }
    if (m_model)
        disconnect(m_model, 0, this, 0);
    m_model = model;
    connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(repaint()));
    connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(repaint()));
}

QAbstractItemModel * ApItemView::model()
{
    return m_model;
}

void ApItemView::setItemDelegate(QAbstractItemDelegate *delegate)
{
    m_delegate = delegate;
}

QAbstractItemDelegate* ApItemView::delegate() const
{
    return m_delegate;
}

void ApItemView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    m_selectionModel = selectionModel;
}

QItemSelectionModel* ApItemView::selectionModel() const
{
    return m_selectionModel;
}

QModelIndex ApItemView::currentIndex() const
{
    return m_selectionModel->currentIndex();
}

void ApItemView::setCurrentIndex(const QModelIndex &index)
{
    m_selectionModel->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

QModelIndex ApItemView::indexAt(const QPoint &p) const
{
    foreach (const QModelIndex &index, m_apLayout.keys()) {
        QRect rect = m_apLayout[index];
        if (rect.contains(p)) {
            return index;
        }
    }
    return QModelIndex();
}

void ApItemView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    paintBackground();

    //draw background
    painter.drawPixmap(0,0, *m_background);
    KIcon computer = KIcon("computer");
    QPoint point = QPoint(0, geometry().height()/2-computer.actualSize(QSize(computerHeight,computerHeight)).height()/2);
    computer.paint(&painter, QRect(point,computer.actualSize(QSize(computerHeight,computerHeight))));

    // Re-sort items
    layoutItems();

    //draw AP Items
    int numRows = m_model->rowCount();
    QStyleOptionViewItem options = m_options;
    for (int ctr=0; ctr < numRows; ctr++) {
        QModelIndex index = m_model->index(ctr,0);
        options.rect = m_apLayout[index];
        if (m_selectionModel != 0 && m_selectionModel->isSelected(index)) {
            options.state |= QStyle::State_Selected;
        }
        m_delegate->paint(&painter, options, index);
        //turn off selected
        options.state &= ~QStyle::State_Selected;
    }
}

void ApItemView::layoutItems()
{
    QList<QModelIndex> list75, list50, list25, list00;

    m_apLayout.clear();
    int numRows = m_model->rowCount();
    for (int ctr=0; ctr < numRows; ctr++) {
        QModelIndex index = m_model->index(ctr,0);
        int strength = m_model->data(index,NetworkItemModel::SignalStrength).toInt();
        int x = (geometry().width()-m_itemWidth)-(geometry().width()-m_itemWidth-computerHeight)*strength/100;
        QPoint topLeft = QPoint(x,0);
        m_apLayout[index] = QRect(topLeft,QSize(m_itemWidth, m_itemHeight));

        //sort into groups
        strength -= strength%25;
        switch (strength) {
            case 100:
            case 75:
                list75 << index;
                break;
            case 50:
                list50 << index;
                break;
            case 25:
                list25 << index;
                break;
            case 0:
                list00 << index;
                break;
        }
    }

    //layout groups
    layoutGroup(list75);
    layoutGroup(list50);
    layoutGroup(list25);
    layoutGroup(list00);
}

void ApItemView::layoutGroup(QList<QModelIndex> group)
{
    int numItems = group.size();

    if (numItems == 0) {
        return;
    } else if(numItems*(m_itemHeight+padding) > geometry().height()) {
        //start at the top and work your way down
        for(int ctr=0; ctr < numItems; ctr++) {
            m_apLayout[group[ctr]].translate(0, m_itemHeight*ctr);
        }
    } else {
        int spacing = geometry().height()/(numItems+1);
        for(int ctr=0; ctr < numItems; ctr++) {
            m_apLayout[group[ctr]].translate(0, spacing*(ctr+1));
        }
    }
}

void ApItemView::paintBackground()
{
    if (m_background != 0) {
        delete m_background;
    }
    m_background = new QPixmap(geometry().size());
    m_background->fill(Qt::white);
    QPainter painter(m_background);
    const int numArcs = 4;
    QSize arcSize = QSize(geometry().width()/numArcs,geometry().height());
    //draw background
    for (int x=0; x <= numArcs; x++) {
        QPoint topLeft(x*geometry().width()/numArcs-arcSize.width()/2,0);
        //angles are defined in 1/16th of a degree
        painter.drawArc(QRect(topLeft,arcSize), -90*16, 180*16);
    }
}

void ApItemView::mouseReleaseEvent ( QMouseEvent *event )
{
    if (event->button() == Qt::LeftButton) {
        QModelIndex clickedIndex = indexAt(event->pos());

        if (clickedIndex.isValid() && m_selectionModel != 0) {
            m_selectionModel->setCurrentIndex(clickedIndex, QItemSelectionModel::ClearAndSelect);
            kDebug() << "You clicked on: " << m_model->data(clickedIndex).toString();
            repaint();
        }
    }
}

void ApItemView::mouseDoubleClickEvent (QMouseEvent *event)
{
    QModelIndex clickedIndex = indexAt(event->pos());
    emit doubleClicked(clickedIndex);
}
