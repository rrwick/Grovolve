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


#ifndef PLANTPART_H
#define PLANTPART_H

#include <vector>
#include <deque>
#include <QPolygonF>
#include <QLineF>
#include <QRectF>
#include "../program/globals.h"
#include "../program/point2d.h"
#include "../settings/simulationsettings.h"

#ifndef Q_MOC_RUN
#include "boost/utility.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/serialization/vector.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class Organism;
class Seed;

class PlantPart : boost::noncopyable
{
public:
    PlantPart() {}
    PlantPart(Organism * organism, PlantPart * parent, int geneIndex, Point2D start);
    ~PlantPart();

    void growOneTick();
    void createChildParts();
    void growChildParts();
    void calculateCenterOfMass();
    void receiveLight(double incomingLight);
    void createSeeds(std::deque<Seed> * seeds, long long elapsedTime, bool dayTime);
    void addLeavesToLightingVector(std::vector<PlantPart *> * leaves);
    void getShapesForDrawing(std::vector<QLineF> * branchLines,
                             std::vector<double> * branchWidths,
                             std::vector<QLineF> * leafLines,
                             std::vector<QLineF> * seedpodsLines,
                             std::vector<QRectF> * seedpodsEnds,
                             double environmentHeight,
                             bool ignoreVisibleArea) const;
    double getGrowthCost();
    double getMaintenanceCost() const;
    bool descendsFromGeneIndex(double otherGeneIndex) const;
    double getHighestPoint() const;
    double getHighestDrawnPoint(bool checkDescendants = true) const;
    double getRightmostDrawnPoint(bool checkDescendants = true) const;
    double getLeftmostDrawnPoint(bool checkDescendants = true) const;
    bool isFinishedGrowing() const;
    double getArea() const {return getLength() * m_width;} //Only used for branches
    double getBulbRadius() const {return getLength() / g_simulationSettings->seedpodLengthToBulbRadius;} //Only used for seedpods
    double getDrawnThickness() const;
    bool isPointInsidePart(Point2D point) const;
    Point2D getStart() const {return m_start;}
    Point2D getEnd() const {return m_end;}
    double getAngle() const {return m_angle;}
    int getGeneIndex() const {return m_geneIndex;}
    Organism * getOrganism() const {return m_organism;}
    PlantPart * getParent() const {return m_parent;}
    double getMass() const {return m_mass;}
    double getMassHereAndAbove() const;
    double getLength() const {return m_start.distanceTo(m_end);}
    bool getFinishedGrowing() const {return m_finishedGrowing;}
    int getLeafCount() const;
    int getBranchCount() const;
    int getSeedpodCount() const;
    int getPlantPartCount() const;

private:
    Organism * m_organism;
    PlantPart * m_parent;
    PlantPartType m_type;
    Point2D m_start;
    Point2D m_end;
    int m_geneIndex;
    Point2D m_dailyGrowth;
    double m_angle;
    double m_finalLength;
    bool m_finishedGrowing;
    Point2D m_centreOfMass;
    double m_mass;
    double m_previousLengthOrArea;
    double m_width; //Only used for Branches
    std::vector<PlantPart *> m_children; //Only used for Branches

    AngleReference getAngleReference(int nucleotide);
    double distanceFromPointToLineSegment(const Point2D v, const Point2D w, const Point2D p) const;
    void createOneChildPart(int childGeneIndex);

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_type;
        ar & m_start;
        ar & m_end;
        ar & m_geneIndex;
        ar & m_parent;
        ar & m_organism;
        ar & m_dailyGrowth;
        ar & m_angle;
        ar & m_finalLength;
        ar & m_finishedGrowing;
        ar & m_centreOfMass;
        ar & m_mass;
        ar & m_previousLengthOrArea;
        ar & m_width;
        ar & m_children;
    }
};

#endif // PLANTPART_H
