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


#include "mutationratevisualaid.h"
#include <QPainter>
#include <QPen>
#include <algorithm>
#include <math.h>
#include "../../program/globals.h"
#include "../../program/randomnumbers.h"


MutationRateVisualAid::MutationRateVisualAid(QWidget *parent, double mutationRate, double maxMutationRate) :
    QWidget(parent),
    m_mutationRate(mutationRate), m_maxMutationRate(maxMutationRate)
{
    m_height = 60;
    m_width = 200;

    m_baseCount = 60;
    m_maxUVRays = 50;

    for (int i = 0; i < m_maxUVRays; ++i)
    {
        QPainterPath uvRay = makeUVRay();
        QPainterPath uvArrowHead = makeUVArrowHead();

        double xTranslation = g_randomNumbers->getRandomDouble(10.0, 190.0);
        double yTranslation = g_randomNumbers->getRandomDouble(-30.0, -15.0);
        double rotation = g_randomNumbers->getRandomDouble(-25.0, 25.0);

        QTransform transform;
        transform.translate(xTranslation, yTranslation);
        transform.rotate(rotation);

        m_rayLines.push_back(transform.map(uvRay));
        m_arrowHeads.push_back(transform.map(uvArrowHead));
    }

    setFixedSize(m_width, m_height);
}


void MutationRateVisualAid::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.fillRect(0, 0, m_width, m_height, QBrush(Qt::white));
    painter.setRenderHint(QPainter::Antialiasing, true);

    paintDNA(&painter);
    paintUV(&painter);

}

void MutationRateVisualAid::paintDNA(QPainter * painter)
{
    double numberOfFullTurns = m_baseCount / 10.0; //~10 bases per turn in real DNA
    double lengthOfFullTurn = m_width / numberOfFullTurns;
    double interval = m_width / m_baseCount;
    QPen helixPen(Qt::black);
    helixPen.setWidth(1);
    painter->setPen(helixPen);
    double waveMagnitude = m_height/6.0 - 5.0;
    double centerY = m_height * 0.87;
    double period = lengthOfFullTurn / 6.283185307179586476925286766559;
    double shift = 2.2;  //Chosen for aesthetics.


    //Paint the double helix
    int baseNumber = 0;
    for (double x = 0.0; x < m_width; x += interval)
    {
        double h1y1 = centerY + waveMagnitude * sin(x / period);
        double h1y2 = centerY + waveMagnitude * sin((x + interval) / period);
        double h2y1 = centerY + waveMagnitude * sin(shift + x / period);
        double h2y2 = centerY + waveMagnitude * sin(shift + (x + interval) / period);

        painter->drawLine(x, h1y1, x, h2y1);
        ++baseNumber;

        //Helix 1
        painter->drawLine(x, h1y1, x + interval, h1y2);

        //Helix 2
        painter->drawLine(x, h2y1, x + interval, h2y2);
    }
}


void MutationRateVisualAid::paintUV(QPainter * painter)
{
    double fractionOfMax = m_mutationRate / m_maxMutationRate;
    int drawnUVRays = m_maxUVRays * pow(fractionOfMax, 0.75);  //0.75 chosen for aesthetics

    QColor violet(127, 0, 255);
    QPen pen(violet);
    pen.setWidth(1);
    QBrush brush(violet);

    for (int i = 0; i < drawnUVRays; ++i)
    {
        painter->strokePath(m_rayLines[i], pen);
        painter->fillPath(m_arrowHeads[i], brush);
    }
}



QPainterPath MutationRateVisualAid::makeUVRay()
{
    QPainterPath ray;
    double period = 10.0 / 6.283185307179586476925286766559;
    double amplitude = 2.5;

    ray.moveTo(0.0, 0.0);
    int y;
    for (y = 0; y < 60; ++y)
        ray.lineTo(amplitude * sin(y / period), y);

    return ray;
}


QPainterPath MutationRateVisualAid::makeUVArrowHead()
{
    QPainterPath arrowHead;

    arrowHead.moveTo(0.0, 64.0);
    arrowHead.lineTo(3.0, 60.0);
    arrowHead.lineTo(-3.0, 60.0);
    arrowHead.lineTo(0.0, 64.0);

    return arrowHead;
}
