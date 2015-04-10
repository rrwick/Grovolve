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


#include "cloud.h"
#include "../settings/environmentsettings.h"
#include "../settings/environmentvalues.h"
#include "../program/randomnumbers.h"

Cloud::Cloud(double elevation, int initialMovement) :
    m_elevation(elevation)
{
    //Build the cloud shape using circles and rectangles.
    m_cloudShape.setFillRule(Qt::WindingFill);
    m_cloudShape.addRect(5, -7, 45, 7);
    m_cloudShape.addEllipse(0, -10, 10, 10);
    m_cloudShape.addEllipse(5, -15, 14, 14);
    m_cloudShape.addEllipse(10, -23, 20, 20);
    m_cloudShape.addEllipse(25, -13, 10, 10);
    m_cloudShape.addEllipse(31, -15, 12, 12);
    m_cloudShape.addEllipse(39, -11, 10, 10);
    m_cloudShape.addEllipse(45, -10, 10, 10);

    double scale = getCloudScale(m_elevation);

    //Half of the clouds will be mirrored to be backwards.
    QTransform transform;
    if (g_randomNumbers->chanceOfTrue(0.5))
        transform.scale(scale, scale);
    else
        transform.scale(-1.0 * scale, scale);
    m_cloudShape = m_cloudShape * transform;

    //The cloud's movement step is directly related to its size.
    m_movementStep = g_simulationSettings->cloudSpeed * scale;

    //Move the cloud so it is just off the screen.
    m_cloudShape.translate(-1.0 * getRightEdge(), 0.0);

    moveCloudMultipleSteps(initialMovement);
}


void Cloud::paintCloud(QPainter * painter, double environmentHeight)
{
    double translation = environmentHeight - m_elevation;
    m_cloudShape.translate(0.0, translation);
    painter->fillPath(m_cloudShape, g_simulationSettings->getSunIntensityCloudColor(g_environmentSettings->m_currentValues.m_sunIntensity));
    m_cloudShape.translate(0.0, -1.0 * translation);
}



void Cloud::moveCloudMultipleSteps(int steps)
{
    m_cloudShape.translate(steps * m_movementStep, 0.0);
}

void Cloud::moveCloud()
{
    m_cloudShape.translate(m_movementStep, 0.0);
}


double Cloud::getLeftEdge() const
{
    return m_cloudShape.boundingRect().left();
}
double Cloud::getRightEdge() const
{
    return m_cloudShape.boundingRect().right();
}
double Cloud::getTopEdge() const
{
    return m_cloudShape.boundingRect().top();
}
double Cloud::getBottomEdge() const
{
    return m_cloudShape.boundingRect().bottom();
}
QRectF Cloud::getBoundingRect(double environmentHeight) const
{
    QRectF rect = m_cloudShape.boundingRect();
    rect.translate(0.0, environmentHeight - m_elevation);
    return rect;
}

