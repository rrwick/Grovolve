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


#ifndef POINT2D_H
#define POINT2D_H

#include <math.h>

#ifndef Q_MOC_RUN
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class Point2D
{
public:
    Point2D() : m_x(0.0), m_y(0.0) {}
    Point2D(const double x, const double y) : m_x(x), m_y(y) {}

    double m_x;
    double m_y;

    Point2D operator+(const Point2D &v) const {return Point2D(m_x + v.m_x, m_y + v.m_y);}
    void operator+=(const Point2D &v) {m_x += v.m_x; m_y += v.m_y;}
    Point2D operator-(const Point2D &v) const {return Point2D(m_x - v.m_x, m_y - v.m_y);}
    void operator-=(const Point2D &v) {m_x -= v.m_x; m_y -= v.m_y;}
    Point2D operator*(const float f) const {return Point2D(m_x * f, m_y * f);}
    void operator*=(const float f) {m_x *= f; m_y *= f;}
    Point2D operator/(const float f) const {return Point2D(m_x / f, m_y / f);}
    void operator/=(const float f) {m_x /= f; m_y /= f;}

    double distanceToSquared(const Point2D &p) const
    {
        const double dX = p.m_x - m_x;
        const double dY = p.m_y - m_y;
        return dX*dX + dY*dY;
    }
    double distanceTo(const Point2D &p) const {return sqrt(distanceToSquared(p));}
    double dotProduct(const Point2D &p) const {return m_x * p.m_x + m_y * p.m_y;}

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_x;
        ar & m_y;
    }
};

#endif // POINT2D_H
