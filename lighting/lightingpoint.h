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


#ifndef LIGHTINGPOINT_H
#define LIGHTINGPOINT_H

#include "../program/globals.h"

class PlantPart;

class LightingPoint
{
public:
    LightingPoint(PlantPart * leaf, double x, double y, double sine, double cosine) :
        m_leaf(leaf),
        m_x(x * cosine - y * sine),
        m_y(x * sine + y * cosine)
    {}

    PlantPart * m_leaf;
    float m_x;
    float m_y;

    //This is a pointer to the other point that originated from the same leaf.
    LightingPoint * m_pairPoint;

    //When a point is further to the left than its pair point, then it is
    //considered to be a stating point, as set by this label.  Point pairs
    //that are vertical (have the same x) will both have isStart as false.
    bool m_isStart;
};

#endif // LIGHTINGPOINT_H
