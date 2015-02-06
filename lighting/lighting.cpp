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


#include "lighting.h"
#include <math.h>
#include <algorithm>    // std::sort
#include "tbb/parallel_sort.h"
#include "tbb/parallel_for.h"
#include "lightingpoint.h"
#include "../program/environment.h"
#include "../settings/environmentsettings.h"
#include "../plant/plantpart.h"


//This function is used to sort the array of LightingPoint pointers.
bool compareLightingPoints(LightingPoint * i, LightingPoint * j) {return (i->m_x < j->m_x);}


Lighting::Lighting() :
    m_sunIntensity(0.0)
{
}




//ALGORITHM OVERVIEW
//------------------
//For a given collection of line segments (leaves) that is being illuminated by parallel
//light rays, this algorithm calculates how much light is absorbed by each of the line
//segments.  The angle and intensity of the light can change, as can the fraction of light
//absorbed by each line segment.
//The algorithm can also generate polygons to represent the shadow cast by each line
//segment, useful for drawing the simulation to the screen.
//
//One limitation of this algorithm is that it needs to know the longest possible line
//segment which corresponds to the widest possible shadow.  The performance of this
//algorithm will degrade as this value increases.  Therefore, this algorithm will perform
//better on a collection of short line segments.
//
//The procedure used is as follows:
// 1) Each line segment has its end points stored and rotated with respect to the light
//    source.  This makes all shadows vertical in the rotated frame of reference,
//    simplifying later steps.
// 2) The collection of points is sorted from left to right (in terms of the rotated
//    frame of reference.  This makes the following step considerably faster.
// 3) For each line segment, the points are found that represent either a start or end
//    of another line segment that will be casting a shadow onto this line segment.
//    These points are saved as ShadowPoint objects.
// 4) The found ShadowPoints are sorted and traced from left to right, determining the
//    amount of shadow cast along each part of the line segment.  The amount of received
//    light is calculated during this process and given to the line segment at the end.
//
//The algorithm seems to perform and scale well when the number of ShadowPoints per leaf
//is relatively constant.  For example, as an environment grows in width but not height
//(a typical case for Grovolve), the algorithm scales well for light coming directly
//from above, but not well for light coming from the side.  As a workaround, Grovolve
//could have the day go from 100 to 260 degrees instead of 90 to 270, to avoid the poor
//performance of shallow light angles.





void Lighting::distributeLight(std::vector<PlantPart *> * leaves, Environment * environment,
                               double sunIntensity, double sunAngle)
{
    //Save relevant values as members.
    m_sunIntensity = sunIntensity;
    m_sunAngle = sunAngle;
    m_environment = environment;


    //If the sun intensity is 0, quit right now without doing anything.
    if (sunIntensity == 0.0)
        return;


    //Calculate the sine and cosine of the angle, as these will be used to rotate
    //each leaf point when creating the LightingPoints.
    double rotationAngleRadians = (sunAngle - 90) * 0.01745329251994329576923690768489;
    m_sine = sin(rotationAngleRadians);
    m_cosine = -1.0 * cos(rotationAngleRadians);


    //Store all lighting points.
    m_points.clear();
    m_points.reserve(leaves->size());
    LightingPoint * point1;
    LightingPoint * point2;
    for (std::vector<PlantPart *>::const_iterator i = leaves->begin(); i != leaves->end(); ++i)
    {
        //Create a LightingPoint for the start and end of the leaf.  The LightingPoint
        //constructor does the rotation.
        point1 = new LightingPoint(*i, (*i)->getStart().m_x, (*i)->getStart().m_y, m_sine, m_cosine);
        point2 = new LightingPoint(*i, (*i)->getEnd().m_x, (*i)->getEnd().m_y, m_sine, m_cosine);

        //Each point points to the other in the pair.
        point1->m_pairPoint = point2;
        point2->m_pairPoint = point1;

        //The left-most of the two LightingPoint is saved in the leaf as the start and the
        //other (right-most) LightingPoint is saved as the end.  The start point is also added
        //to the points vector.  The end point isn't added to the vector because all LightingPoints
        //have a pointer to their pair.
        if (point1->m_x < point2->m_x)
            m_points.push_back(point1);
        else
            m_points.push_back(point2);
    }


    //Sort the points from left to right.  This makes it possible to quickly
    //find the relevant points for the shadows on each leaf.
    tbb::parallel_sort(m_points.begin(), m_points.end(), compareLightingPoints);


    //For each leaf, determine the total incoming light.
    //Done in parallel using Intel Threading Building Blocks.
    tbb::parallel_for(tbb::blocked_range<size_t>(0, m_points.size()),
                      [=](const tbb::blocked_range<size_t>& r)
    {
        for(size_t i=r.begin(); i!=r.end(); ++i)
            findLightOnLeaf(m_points[i]);
    }
    );


    //Clean up the points.  Since only the starting points are stored in the points
    //vector, the ending points (pairs of the starting points) must be deleted first.
    for (std::vector<LightingPoint *>::const_iterator i = m_points.begin(); i != m_points.end(); ++i)
    {
        delete (*i)->m_pairPoint;
        delete *i;
    }
}

//This function calculates the light that is absorbed by a leaf.  It does so by creating
//all of the relevant ShadowPoints for that leaf.  It assumes that the vector of LightingPoints
//has been sorted.
void Lighting::findLightOnLeaf(LightingPoint * leafStart)
{
    PlantPart * leaf = leafStart->m_leaf;
    LightingPoint * leafEnd = leafStart->m_pairPoint;
    std::vector<ShadowPoint> shadowPoints;


    //Find the starting LightingPoint for the search: the first point that is within
    //maxLeafLength of the leaf's left point.
    double targetX = leafStart->m_x - g_simulationSettings->leafLength;
    size_t i = findStartPoint(targetX);


    //Loop through all LightingPoints from the starting one to the end of the leaf.
    double endOfLeaf = leafEnd->m_x;
    LightingPoint * currentStartPoint;
    LightingPoint * currentEndPoint;
    bool startPointAbove;
    bool endPointAbove;
    for (; i < m_points.size() && (*m_points[i]).m_x < endOfLeaf; ++i)
    {
        currentStartPoint = m_points[i];

        //If the current point belongs to the current leaf, then skip this one, as a
        //leaf can't shadow itself!
        if (currentStartPoint == leafStart)
            continue;

        currentEndPoint = currentStartPoint->m_pairPoint;


        //If both the current points are above the leaf, then they will definitely be
        //casting a relevant shadow.  Create a ShadowPoint for each of them and add
        //them to the vector.
        startPointAbove = isPointAboveLine(currentStartPoint, leafStart, leafEnd);
        endPointAbove = isPointAboveLine(currentEndPoint, leafStart, leafEnd);
        if (startPointAbove && endPointAbove)
        {
            shadowPoints.emplace_back(currentStartPoint->m_x, true);
            shadowPoints.emplace_back(currentEndPoint->m_x, false);
        }

        //If both the current points are below the leaf, then they definitely won't be
        //casting a relevant shadow.  Do nothing.
        else if (!startPointAbove && !endPointAbove)
            continue;

        //If one of the current points is below the leaf, then part of their line segment
        //will be casting a relevant shadow.  Move whichever point is below the leaf up
        //to the line of the leaf (while staying on its line segment), create ShadowPoints
        //for both, and add them to the vector.
        else if (startPointAbove)
        {
            //In this case, the start point is already above and a corresponding ShadowPoint
            //can be easily made.
            shadowPoints.emplace_back(currentStartPoint->m_x, true);

            //The end point needs to be moved 'up' to the line of the leaf.  However, since
            //a ShadowPoint object only holds the x value, that's the only number we need to
            //calculate.
            shadowPoints.emplace_back(getIntersectionX(leafStart, leafEnd, currentStartPoint, currentEndPoint), false);
        }
        else if (endPointAbove)
        {
            //The start point needs to be moved 'up' to the line of the leaf.  However, since
            //a ShadowPoint object only holds the x value, that's the only number we need to
            //calculate.
            shadowPoints.emplace_back(getIntersectionX(leafStart, leafEnd, currentStartPoint, currentEndPoint), true);

            //The end point is already above and a corresponding ShadowPoint
            //can be easily made.
            shadowPoints.emplace_back(currentEndPoint->m_x, false);
        }
    }


    //Sort the vector of ShadowPoints.
    std::sort(shadowPoints.begin(), shadowPoints.end());


    //Loop through the vector of ShadowPoints, tracking the number of shadows present and
    //allocating light to the leaf as appropriate.
    int shadowCount = 0;
    double lightForLeaf = 0.0;
    double lastLeafX = leafStart->m_x;
    for (std::vector<ShadowPoint>::iterator i = shadowPoints.begin(); i != shadowPoints.end(); ++i)
    {
        //If we have reached a point beyond the end of the leaf, break out of the
        //loop as there is no need to go on.
        if (i->m_x > leafEnd->m_x)
            break;

        //If we encounter a shadow point in the leaf's range, then the appropriate
        //amount of light needs to be given to the leaf.
        if (i->m_x > leafStart->m_x)
        {
            lightForLeaf += calculateLightForLeafSection(shadowCount, i->m_x - lastLeafX);
            lastLeafX = i->m_x;
        }

        //ShadowPoints that indicate the start of a shadow increase the shadow count.  The rest
        //indicate the end of a shadow and decrease the shadow count.  At no point should the
        //shadow count drop below zero (DOUBLE CHECK THIS!).
        if (i->m_isStart)
            ++shadowCount;
        else
            --shadowCount;
    }

    //Now that the loop is done, any remaining section of the leaf needs to receive its light.
    lightForLeaf += calculateLightForLeafSection(shadowCount, leafEnd->m_x - lastLeafX);

    //Now actually give the light to the leaf!
    leaf->receiveLight(lightForLeaf);
}




//Uses binary search to find the point with the x value closest to targetX (without going under).
//Since the target X was determined by subtracting from a leaf's starting point, this search should
//always be successful as this is always a starting point greater than targetX.
int Lighting::findStartPoint(double targetX)
{
    //Use binary search to locate the first potential point to examine.
    int l = 0;
    int r = int(m_points.size()) - 1;
    int m;
    while (l <= r)
    {
        m = (l + r) / 2;
        if (targetX == (*m_points[m]).m_x)
            return m;
        else if (targetX < (*m_points[m]).m_x)
            r = m - 1;
        else
            l = m + 1;
    }

    //If the code got here, then an exact match was not found (probably
    //the most common case).  Return the larger of the two closest values,
    //which should correspond to the left value (as it has now passed
    //the right value).
    return l;
}




//This function finds the intersection between two lines and returns the x coordinate
//of this intersection.
//p1 and p2 define the first line, p3 and p4 define the second.
//While I think two parallel (i.e. non-intersecting) lines might result in a divide-by-zero
//error, this function should only be called when the two lines are known to intersect.
//http://en.wikipedia.org/wiki/Line-line_intersection
double Lighting::getIntersectionX(LightingPoint * p1, LightingPoint * p2,
                                  LightingPoint * p3, LightingPoint * p4)
{
    return ((((p1->m_x * p2->m_y) - (p1->m_y * p2->m_x)) * (p3->m_x - p4->m_x)) - ((p1->m_x - p2->m_x) * ((p3->m_x * p4->m_y) - (p3->m_y * p4->m_x)))) / (((p1->m_x - p2->m_x) * (p3->m_y - p4->m_y)) - ((p1->m_y - p2->m_y) * (p3->m_x - p4->m_x)));
}





//This function calculates how much light a leaf absorbs for a given shadow count (how
//many leaves are above this one with respect to the light) and distance (the length of
//the leaf segment with the specified number of shadows).
//This may be called multiple times for a single leaf if that leaf is partially in any
//shadows.
double Lighting::calculateLightForLeafSection(int shadowCount, double xDistance)
{
    //Intensity of light = sunIntensity * pow(1.0 - leafAbsorbance, shadowCount)
    //Quantity of light = Intensity * xDistance
    //Absorbed light = Quantity * leafAbsorbance
    return m_sunIntensity * pow(1.0 - g_simulationSettings->leafAbsorbance, shadowCount) * xDistance * g_simulationSettings->leafAbsorbance;
}



//From: http://stackoverflow.com/questions/3461453/determine-which-side-of-a-line-a-point-lies
bool Lighting::isPointAboveLine(LightingPoint * testPoint, LightingPoint * linePoint1, LightingPoint * linePoint2)
{
    return ((linePoint2->m_x - linePoint1->m_x) * (testPoint->m_y - linePoint1->m_y) - (linePoint2->m_y - linePoint1->m_y) * (testPoint->m_x - linePoint1->m_x)) < 0;
}
