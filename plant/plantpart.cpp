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


#include "plantpart.h"
#include <math.h>
#include "organism.h"
#include "genome.h"
#include "../program/randomnumbers.h"
#include "../settings/environmentsettings.h"
#include "../program/stats.h"
#include "seed.h"

PlantPart::PlantPart(Organism * organism, PlantPart * parent, int geneIndex,
                     Point2D start) :
    m_organism(organism), m_parent(parent),
    m_start(start), m_end(start), m_geneIndex(geneIndex), m_finishedGrowing(false),
    m_centreOfMass(start), m_mass(0.0), m_previousLengthOrArea(0.0), m_width(1.0)
{
    m_type = m_organism->getGenome()->getTypeFrom2Nucleotides(geneIndex);

    if (m_type == NO_PART)
    {
        m_finalLength = 0;
        m_finishedGrowing = true;
    }

    //Get part parameters from the genome.
    AngleReference angleReference = getAngleReference(m_organism->getGenome()->getNucleotide(geneIndex + 2));
    double angleFromGenome = m_organism->getGenome()->getSignedNumberFrom4Nucleotides(geneIndex + 3);
    double growthRate = m_organism->getGenome()->getUnsignedNumberFrom4Nucleotides(geneIndex + 7);
    growthRate = growthRate * g_simulationSettings->growRateGeneRatio / 100.0 + g_simulationSettings->minimumGrowthRate;

    if (m_type == LEAF)
        m_finalLength = g_simulationSettings->leafLength;
    else
    {
        m_finalLength = m_organism->getGenome()->getUnsignedNumberFrom4Nucleotides(geneIndex + 11) + g_simulationSettings->minimumPlantPartLength;
        if (m_type == SEEDPOD)
            m_finalLength /= g_simulationSettings->seedpodLengthScale;
    }

    //Add randomness to the angle and the growth rate.
    double randomness = m_organism->getRandomness();
    if (randomness > 0.0)
    {
        double randomAngleRange = 360.0 * m_organism->getRandomness();
        angleFromGenome += g_randomNumbers->getRandomDouble(-1.0 * randomAngleRange, randomAngleRange);
        double randomGrowthRateRange = growthRate * m_organism->getRandomness();
        growthRate += g_randomNumbers->getRandomDouble(-1.0 * randomGrowthRateRange, randomGrowthRateRange);
        double randomLengthRange = m_finalLength * m_organism->getRandomness();
        m_finalLength += g_randomNumbers->getRandomDouble(-1.0 * randomLengthRange, randomLengthRange);
    }

    //Determine the X and Y that will be changed with daily growth.
    if (m_parent == 0 || angleReference == VERTICAL)
        m_angle = angleFromGenome;
    else
        m_angle = angleFromGenome + parent->getAngle();
    double angleRadians = m_angle * 0.01745329251994329576923690768489;
    m_dailyGrowth.m_x = growthRate * cos(angleRadians);
    m_dailyGrowth.m_y = growthRate * sin(angleRadians);
}

PlantPart::~PlantPart()
{
    for (std::vector<PlantPart *>::iterator i = m_children.begin(); i != m_children.end(); ++i)
        delete *i;
}


AngleReference PlantPart::getAngleReference(int nucleotide)
{
    switch (nucleotide)
    {
    case 1:
    case 2:
        return PARENT;
    case 3:
    default:
        return VERTICAL;
    }
}

void PlantPart::growOneTick()
{
    if (m_finishedGrowing)
    {
        growChildParts();
        return;
    }
    m_end += m_dailyGrowth;

    //If the growth puts the part below the ground, undo the growth and
    //stop the part from growing in the future.
    if (m_end.m_y < 0.0)
    {
        m_end -= m_dailyGrowth;
        m_finishedGrowing = true;
        return;
    }


    //If the PlantPart is now longer than its maximum length, do the following:
    // 1) change its endPoint such that is is at the maximum length
    // 2) stop its future growth
    // 3) create any child plant parts, if appropriate
    if (getLength() > m_finalLength)
    {
        double angleRadians = m_angle * 0.01745329251994329576923690768489;
        m_end.m_x = m_start.m_x + m_finalLength * cos(angleRadians);
        m_end.m_y = m_start.m_y + m_finalLength * sin(angleRadians);

        m_finishedGrowing = true;

        createChildParts();
    }

    //Now deduct from the organism the energy used in growth.

    if (!m_organism->isHistoryOrganism())
    {
        double growthCost = getGrowthCost();
        m_organism->deductEnergy(growthCost);
        m_organism->addToEnergySpentOnGrowthAndMaintenance(growthCost);
    }
}




bool PlantPart::descendsFromGeneIndex(double otherGeneIndex) const
{
    if (otherGeneIndex == m_geneIndex)
        return true;
    else if (m_parent == 0)
        return false;
    else
        return m_parent->descendsFromGeneIndex(otherGeneIndex);
}



//http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
double PlantPart::distanceFromPointToLineSegment(const Point2D v, const Point2D w, const Point2D p) const
{
    const float distSq = v.distanceToSquared( w ); // i.e. |w-v|^2 ... avoid a sqrt
    if (distSq == 0.0)
        return v.distanceTo(p);

    const float t = (p - v).dotProduct(w - v) / distSq;
    if (t < 0.0)
        return v.distanceTo(p);
    else if ( t > 1.0 )
        return w.distanceTo(p);

    const Point2D projection = v + ((w - v) * t);
    return p.distanceTo(projection);
}




double PlantPart::getGrowthCost()
{
    double currentLengthOrArea;
    if (m_type == BRANCH)
        currentLengthOrArea = getLength() * m_width;
    else //LEAF or SEEDPOD
        currentLengthOrArea = getLength();

    double lengthOrAreaGrown = currentLengthOrArea - m_previousLengthOrArea;
    m_previousLengthOrArea = currentLengthOrArea;

    if (m_type == BRANCH)
        return lengthOrAreaGrown * g_simulationSettings->branchGrowthCost;
    else if (m_type == LEAF)
        return lengthOrAreaGrown * g_simulationSettings->leafGrowthCost;
    else //SEEDPOD
        return lengthOrAreaGrown * g_simulationSettings->seedpodGrowthCost;
}



double PlantPart::getMaintenanceCost() const
{
    double maintenanceCost = g_simulationSettings->plantPartMaintenanceCost;

    if (m_type == BRANCH)
    {
        maintenanceCost += getArea() * g_simulationSettings->branchMaintenanceCost;
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            maintenanceCost += (*i)->getMaintenanceCost();

    }
    else if (m_type == LEAF)
        maintenanceCost += getLength() * g_simulationSettings->leafMaintenanceCost;
    else //SEEDPOD
        maintenanceCost += getLength() * g_simulationSettings->seedpodMaintenanceCost;

    return maintenanceCost;
}



//This code looks at the genome to determine which (if any) child PlantParts need
//to be created, and then it creates them.  It only does anything for branches.
void PlantPart::createChildParts()
{
    if (m_type != BRANCH)
        return;

    int thisGeneIndex = getGeneIndex();
    int promotersStartIndex = thisGeneIndex + 15; //Branch genes have 14 nucleotides before promoter references
    Genome * genome = getOrganism()->getGenome();

    for (int i = 0; i < g_simulationSettings->maxChildrenPerBranch; ++i)
    {
        std::vector<char> promoter;
        for (int j = 0; j < g_simulationSettings->promoterLength; ++j)
            promoter.push_back(genome->getNucleotide(promotersStartIndex + i * g_simulationSettings->promoterLength + j));

        int childGeneIndex = getOrganism()->getGenome()->getIndexFromPromoter(thisGeneIndex, &promoter);

        //The Genome::getGeneIndexFromPromoter function returns negative one if a match is not found.
        if (childGeneIndex == -1)
            continue;

        //Create the child PlantParts, but do not allow any direct duplicates.  I.e. two children
        //cannot have the same gene index.  This would result in them always growing exactly on top
        //of each other, appearing as one part to the user (confusing).  This rule also helps to
        //prevent runaway exponential growth in a plant.
        bool geneIndexAlreadyUsed = false;
        for (std::vector<PlantPart *>::iterator j = m_children.begin(); j != m_children.end(); ++j)
        {
            if ((*j)->getGeneIndex() == childGeneIndex)
            {
                geneIndexAlreadyUsed = true;
                break;
            }
        }
        if (!geneIndexAlreadyUsed)
            createOneChildPart(childGeneIndex);
    }
}

void PlantPart::createOneChildPart(int childGeneIndex)
{
    //If the gene's has no type, then no child is made.
    if (getOrganism()->getGenome()->getTypeFrom2Nucleotides(childGeneIndex) == NO_PART)
        return;

    //If the organism has already reached the maximum number of plant parts,
    //then no child is made.
    if (getOrganism()->getPlantPartCount() >= g_simulationSettings->maxPlantPartsPerOrganism)
        return;

    //Whether or not a loop (child having the same gene index as the parent) is allowed is
    //determined by a setting.
    if ( !(g_simulationSettings->allowLoops) )
    {
        if (descendsFromGeneIndex(childGeneIndex))
            return;
    }

    m_children.push_back(new PlantPart(m_organism, this, childGeneIndex, m_end));
}




void PlantPart::growChildParts()
{
    for (std::vector<PlantPart *>::iterator i = m_children.begin(); i != m_children.end(); ++i)
        (*i)->growOneTick();
}





void PlantPart::createSeeds(std::deque<Seed> *seeds, long long elapsedTime, bool dayTime)
{
    if (m_type == BRANCH)
    {
        for (std::vector<PlantPart *>::iterator i = m_children.begin(); i != m_children.end(); ++i)
            (*i)->createSeeds(seeds, elapsedTime, dayTime);
    }

    else if (m_type == SEEDPOD)
    {
        //CURRENTLY THE ENERGY THAT'S PUT INTO THE SEED IS SIMPLY
        //THE SEEDPOD LENGTH SQUARED.  HOWEVER, I MAY WANT TO MAKE THIS
        //MORE FLEXIBLE LATER.
        double length = getLength();
        double seedEnergy = length * length;

        int seedCount = g_randomNumbers->changeDoubleToProbabilisticInt(g_simulationSettings->newSeedsPerTickPerSeedpod);

        for (int i = 0; i < seedCount; ++i)
        {
            //The seedpod will only make seeds that the organism can afford.
            if (seedEnergy < getOrganism()->getEnergy() && dayTime)
            {
                //Create the seed and add it to the container.
                seeds->emplace_back(seedEnergy, getOrganism()->getGenomeSharedPointer(), elapsedTime, getOrganism()->getGeneration());

                //Deduct the energy from the Organism.
                double totalEnergyCost = seedEnergy + g_simulationSettings->seedCreationCost;
                m_organism->deductEnergy(totalEnergyCost);
                m_organism->addToEnergySpentOnReproduction(totalEnergyCost);

                ++(g_stats->m_numberOfSeedsGenerated);
            }
        }
    }
}



void PlantPart::calculateCenterOfMass()
{
    if (m_type == BRANCH)
    {
        //First deal with any PlantParts above this one.
        for (std::vector<PlantPart *>::iterator i = m_children.begin(); i != m_children.end(); ++i)
            (*i)->calculateCenterOfMass();

        //Determine the mass and centre of mass for this branch and all parts above it.
        //The mass is found by simply adding up all the masses.  The centre of mass is
        //found by doing a weighted average.
        double massOfOnlyThisBranch = getArea() * g_simulationSettings->branchDensity + 1.0;  //Add one to prevent zero-mass PlantParts
        Point2D average = ((m_start + m_end) / 2.0) * massOfOnlyThisBranch;

        double totalMass = massOfOnlyThisBranch;

        for (std::vector<PlantPart *>::iterator i = m_children.begin(); i != m_children.end(); ++i)
        {
            double childMass = (*i)->getMass();
            totalMass += childMass;
            average += (((*i)->m_start + (*i)->m_end) / 2.0) * childMass;
        }

        average /= totalMass;
        m_mass = totalMass;
        m_centreOfMass = average;

        //Determine the load on the branch.  Load is a sum of mass and torque.  The
        //torque is scaled by a scaling factor so its influence on load can be changed.
        double torque = (m_start.m_x - average.m_x) * totalMass;
        double load = totalMass + fabs(torque) * g_simulationSettings->torqueScalingFactor;
        load *= g_environmentSettings->m_currentValues.m_gravity;

        //Determine the branch's strength.  If the branch's strength is not enough to hold the load, increase its width.
        double strength = pow(m_width, g_simulationSettings->branchStrengthScalingPower) * g_simulationSettings->branchStrengthFactor;
        if (load > strength)
            m_width += g_simulationSettings->branchWidthGrowthIncrement;

        return;
    }

    else if (m_type == LEAF)
        m_mass = getLength() * g_simulationSettings->leafDensity + 1.0;  //Add one to prevent zero-mass PlantParts
    else //SEEDPOD
        m_mass = getLength() * g_simulationSettings->seedpodDensity + 1.0;  //Add one to prevent zero-mass PlantParts
    m_centreOfMass = Point2D((m_start.m_x + m_end.m_x) / 2.0, (m_start.m_y + m_end.m_y) / 2.0);
}




void PlantPart::addLeavesToLightingVector(std::vector<PlantPart *> * leaves)
{
    if (m_type == BRANCH)
    {
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            (*i)->addLeavesToLightingVector(leaves);
    }
    else if (m_type == LEAF)
        leaves->push_back(this);
}



void PlantPart::receiveLight(double incomingLight)
{
    //Helped organisms get bonus energy from photosynthesis.
    if (m_organism->isHelped())
        incomingLight *= g_simulationSettings->helpedBonusEnergy;

    m_organism->addEnergy(incomingLight);
    m_organism->addToEnergyFromPhotosynthesis(incomingLight);
}




double PlantPart::getDrawnThickness() const
{
    if (m_type == BRANCH)
        return std::max(m_width, g_simulationSettings->branchLineThickness);
    else if (m_type == LEAF)
        return g_simulationSettings->leafThickness;
    else //SEEDPOD
        return g_simulationSettings->seedpodThickness;
}


double PlantPart::getHighestPoint() const
{
    double highestPoint = std::max(m_start.m_y, m_end.m_y);

    if (m_type == BRANCH)
    {
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
        {
            double highestPointInChild = (*i)->getHighestPoint();
            if (highestPointInChild > highestPoint)
                highestPoint = highestPointInChild;
        }
    }

    return highestPoint;
}



double PlantPart::getHighestDrawnPoint(bool checkDescendants) const
{
    double highestDrawnPoint = std::max(m_start.m_y, m_end.m_y) + getDrawnThickness() / 2.0;

    if (m_type == BRANCH && checkDescendants)
    {
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
        {
            double highestDrawnPointInChild = (*i)->getHighestDrawnPoint();
            if (highestDrawnPointInChild > highestDrawnPoint)
                highestDrawnPoint = highestDrawnPointInChild;
        }
    }
    if (m_type == SEEDPOD)
        highestDrawnPoint = std::max(highestDrawnPoint, m_end.m_y + getBulbRadius());

    return highestDrawnPoint;
}

double PlantPart::getRightmostDrawnPoint(bool checkDescendants) const
{
    double rightmostDrawnPoint = std::max(m_start.m_x, m_end.m_x) + getDrawnThickness() / 2.0;

    if (m_type == BRANCH && checkDescendants)
    {
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
        {
            double rightmostDrawnPointInChild = (*i)->getRightmostDrawnPoint();
            if (rightmostDrawnPointInChild > rightmostDrawnPoint)
                rightmostDrawnPoint = rightmostDrawnPointInChild;
        }
    }
    if (m_type == SEEDPOD)
        rightmostDrawnPoint = std::max(rightmostDrawnPoint, m_end.m_x + getBulbRadius());

    return rightmostDrawnPoint;
}

double PlantPart::getLeftmostDrawnPoint(bool checkDescendants) const
{
    double leftmostDrawnPoint = std::min(m_start.m_x, m_end.m_x) - getDrawnThickness() / 2.0;

    if (m_type == BRANCH && checkDescendants)
    {
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
        {
            double leftmostDrawnPointInChild = (*i)->getLeftmostDrawnPoint();
            if (leftmostDrawnPointInChild < leftmostDrawnPoint)
                leftmostDrawnPoint = leftmostDrawnPointInChild;
        }
    }
    if (m_type == SEEDPOD)
        leftmostDrawnPoint = std::min(leftmostDrawnPoint, m_end.m_x - getBulbRadius());

    return leftmostDrawnPoint;
}


bool PlantPart::isFinishedGrowing() const
{
    if (m_type == BRANCH)
    {
        //Return false if this Branch hasn't finished growing.
        if (!m_finishedGrowing)
            return false;

        //Return false if any of its children (or their children) haven't
        //finished growing.
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
        {
            if (!((*i)->isFinishedGrowing()))
                return false;
        }

        return true;
    }
    else //LEAF or SEEDPOD
        return m_finishedGrowing;
}




bool PlantPart::isPointInsidePart(Point2D point) const
{
    double halfDrawnThickness = getDrawnThickness() / 2.0;
    if (distanceFromPointToLineSegment(m_start, m_end, point) < halfDrawnThickness)
        return true;

    if (m_type == BRANCH)
    {
        //If the point is in any of the Branch's descendents, return true.
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
        {
            if ((*i)->isPointInsidePart(point))
                return true;
        }
    }
    else if (m_type == SEEDPOD)
        return (m_end.distanceTo(point) < getBulbRadius());

    return false;
}




void PlantPart::getShapesForDrawing(std::vector<QLineF> * branchLines,
                                    std::vector<double> * branchWidths,
                                    std::vector<QLineF> * leafLines,
                                    std::vector<QLineF> * seedpodsLines,
                                    std::vector<QRectF> * seedpodsEnds,
                                    double environmentHeight,
                                    bool ignoreVisibleArea) const
{
    //Only get the shapes if they are in the visible area.
    if (ignoreVisibleArea ||
            (getLeftmostDrawnPoint(false) < g_visibleRect.right() &&
             getRightmostDrawnPoint(false) > g_visibleRect.left() &&
             environmentHeight - getHighestDrawnPoint(false) < g_visibleRect.bottom()))
    {
        if (m_type == BRANCH)
        {
            branchLines->push_back(QLineF(m_start.m_x, environmentHeight - m_start.m_y, m_end.m_x, environmentHeight - m_end.m_y));  //Subtract Y from height to invert the drawing, as simulation (0, 0) is bottom left but drawing (0, 0) is top left.
            branchWidths->push_back(m_width);
        }
        else if (m_type == LEAF)
        {
            leafLines->push_back(QLineF(m_start.m_x, environmentHeight - m_start.m_y,
                                        m_end.m_x, environmentHeight - m_end.m_y));  //Subtract Y from height to invert the drawing, as simulation (0, 0) is bottom left but drawing (0, 0) is top left.
        }
        else //SEEDPOD
        {
            seedpodsLines->push_back(QLineF(m_start.m_x, environmentHeight - m_start.m_y, m_end.m_x, environmentHeight - m_end.m_y));  //Subtract Y from height to invert the drawing, as simulation (0, 0) is bottom left but drawing (0, 0) is top left.

            double seedpodRadius = getBulbRadius();
            seedpodsEnds->push_back(QRectF(m_end.m_x - seedpodRadius,
                                           environmentHeight - (m_end.m_y + seedpodRadius),
                                           seedpodRadius * 2, seedpodRadius * 2));
        }
    }

    //Now pass the objects on to any children.
    if (m_type == BRANCH)
    {
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            (*i)->getShapesForDrawing(branchLines, branchWidths, leafLines, seedpodsLines, seedpodsEnds, environmentHeight, ignoreVisibleArea);
    }
}




int PlantPart::getLeafCount() const
{
    if (m_type == LEAF)
        return 1;
    else if (m_type == BRANCH)
    {
        int leafCount = 0;
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            leafCount += (*i)->getLeafCount();
        return leafCount;
    }
    else
        return 0;
}

int PlantPart::getBranchCount() const
{
    if (m_type == BRANCH)
    {
        int branchCount = 1;
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            branchCount += (*i)->getBranchCount();
        return branchCount;
    }
    else
        return 0;
}

int PlantPart::getSeedpodCount() const
{
    if (m_type == SEEDPOD)
        return 1;
    else if (m_type == BRANCH)
    {
        int seedpodCount = 0;
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            seedpodCount += (*i)->getSeedpodCount();
        return seedpodCount;
    }
    else
        return 0;
}

int PlantPart::getPlantPartCount() const
{
    if (m_type == BRANCH)
    {
        int partCount = 1;
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            partCount += (*i)->getPlantPartCount();
        return partCount;
    }
    else
        return 1;
}



double PlantPart::getMassHereAndAbove() const
{
    if (m_type == BRANCH)
    {
        double mass = m_mass;
        for (std::vector<PlantPart *>::const_iterator i = m_children.begin(); i != m_children.end(); ++i)
            mass += (*i)->getMassHereAndAbove();
        return mass;
    }
    else
        return getMass();
}
