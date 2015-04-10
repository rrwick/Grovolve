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


#ifndef CLOUD_H
#define CLOUD_H

#include <QPainterPath>
#include <QPainter>
#include <cmath>
#include "../settings/simulationsettings.h"
#include "../program/globals.h"

class Cloud
{
public:
    Cloud(double elevation, int initialMovement);

    void paintCloud(QPainter * painter, double environmentHeight);
    void moveCloud();
    void moveCloudMultipleSteps(int steps);
    double getLeftEdge() const;
    double getRightEdge() const;
    double getTopEdge() const;
    double getBottomEdge() const;
    QRectF getBoundingRect(double environmentHeight) const;
    static double getCloudScale(double elevation) {return 0.1 * pow(elevation, g_simulationSettings->cloudScalingPower);}

private:
    QPainterPath m_cloudShape;
    double m_elevation;
    double m_movementStep;
};

#endif // CLOUD_H
