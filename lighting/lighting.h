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


#ifndef LIGHTING_H
#define LIGHTING_H

#include "shadowpoint.h"
#include "../program/globals.h"
#include <vector>
#include <QPolygonF>

class LightingPoint;
class Environment;
class PlantPart;

class Lighting
{
public:
    Lighting();

    //This is the primary public function of the class.  It takes a vector
    //of Leaf objects, along with lighting parameters.  It calculates the
    //amount of light falling on each Leaf, and it gives the Leaf that
    //light.
    void distributeLight(std::vector<PlantPart *> * leaves, Environment * environment,
                         double sunIntensity, double sunAngle);

    void resetSunIntensity() {m_sunIntensity = 0.0;}
    double getSunIntensity() const {return m_sunIntensity;}

private:
    //These variables are the parameters for the current lighting run.  They
    //are saved as member variables to avoid having to pass them all around.
    double m_sunIntensity;
    double m_sunAngle;

    Environment * m_environment;
    std::vector<LightingPoint *> m_points;

    //Sine and cosine are used a lot, so they are calculated once for a given
    //angle and then saved to be used later.
    double m_sine;
    double m_cosine;

    void findLightOnLeaf(LightingPoint * leafStart);
    int findStartPoint(double targetX);
    inline double getIntersectionX(LightingPoint * p1, LightingPoint * p2, LightingPoint * p3, LightingPoint * p4);
    double calculateLightForLeafSection(int shadowCount, double xDistance);
    bool isPointAboveLine(LightingPoint * testPoint, LightingPoint * linePoint1, LightingPoint * linePoint2);
};

#endif // LIGHTING_H
