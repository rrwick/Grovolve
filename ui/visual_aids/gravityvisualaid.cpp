//Copyright 2015 Ryan Wick

//This file is part of Grovolve.

//Grovolve is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//Grovolve is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

//You should have received a copy of the GNU General Public License
//along with Grovolve.  If not, see <http://www.gnu.org/licenses/>.


#include "gravityvisualaid.h"
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QPolygonF>
#include <QPointF>
#include <QFont>
#include <math.h>

GravityVisualAid::GravityVisualAid(QWidget *parent, double gravity, double maxGravity) :
    QWidget(parent),
    m_gravity(gravity), m_maxGravity(maxGravity)
{
    m_height = 60;
    m_width = 200;

    setFixedSize(m_width, m_height);
}


void GravityVisualAid::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(0, 0, m_width, m_height, QBrush(Qt::white));
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_gravity == 0.0)
    {
        QFont font;
        font.setPointSizeF(font.pointSizeF() * 2.0);
        painter.setFont(font);
        painter.setPen(QPen(Qt::darkRed));
        painter.drawText(0, 0, m_width, m_height, Qt::AlignCenter, "Zero G");
        return;
    }

    double margin = 15;
    QPen arrowPen(Qt::black);
    double gravityFractionOfMax = m_gravity / m_maxGravity;
    double adjustedFractionOfMax = sqrt(gravityFractionOfMax);
    double arrowWidth = adjustedFractionOfMax * 20;
    arrowPen.setWidth(arrowWidth);
    arrowPen.setCapStyle(Qt::FlatCap);

    double arrowStartY = margin - arrowWidth/2;
    double arrowEndY = m_height - margin;

    double arrowHeadTopY = arrowEndY - arrowWidth/2.0;
    double arrowHeadBottomY = arrowEndY + arrowWidth/2.0;

    QBrush arrowHeadBrush(Qt::black);
    painter.setBrush(arrowHeadBrush);

    for (int i = 1; i <= 4; ++i)
    {
        painter.setPen(arrowPen);
        double arrowX = i * m_width / 5.0;
        painter.drawLine(arrowX, arrowStartY, arrowX, arrowEndY);

        painter.setPen(Qt::NoPen);
        QPolygonF arrowHead;
        arrowHead << QPointF(arrowX - arrowWidth, arrowHeadTopY);
        arrowHead << QPointF(arrowX + arrowWidth, arrowHeadTopY);
        arrowHead << QPointF(arrowX, arrowHeadBottomY);
        painter.drawPolygon(arrowHead);
    }



}
