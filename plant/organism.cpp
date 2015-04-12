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


#include "organism.h"
#include "plantpart.h"
#include "../program/environment.h"
#include "../lighting/lighting.h"
#include "../program/randomnumbers.h"
#include "seed.h"
#include "genome.h"
#include "../settings/environmentsettings.h"
#include "../program/point2d.h"
#include "../program/stats.h"
#include <algorithm>    // std::sort

//This constructor makes the initial batch of organisms.
Organism::Organism(double energy, long long elapsedTime, double xPos) :
    m_energy(energy),
    m_genome(new Genome(true, boost::shared_ptr<Genome>(), boost::shared_ptr<Genome>())),
    m_birthDate(elapsedTime), m_generation(1.0),
    m_randomness(g_simulationSettings->growthRandomness), m_historyOrganism(false),
    m_energyFromPhotosynthesis(0.0), m_energySpentOnGrowthAndMaintenance(0.0), m_energySpentOnReproduction(0.0),
    m_firstPart(new PlantPart(this, 0, 0, Point2D(xPos, 0.0))),
    m_helped(false)
{
    setColorsWithRandomness();
}


//This constructor makes most organisms - those with two parents.
Organism::Organism(Seed &seed1, Seed &seed2, long long elapsedTime, double xPos) :
    m_genome(new Genome(false, seed1.m_genome, seed2.m_genome)),
    m_birthDate(elapsedTime),
    m_generation((seed1.getGeneration() + seed2.getGeneration()) / 2.0 + 1.0),
    m_randomness(g_simulationSettings->growthRandomness), m_historyOrganism(false),
    m_energyFromPhotosynthesis(0.0), m_energySpentOnGrowthAndMaintenance(0.0), m_energySpentOnReproduction(0.0),
    m_firstPart(new PlantPart(this, 0, 0, Point2D(xPos, 0.0))),
    m_helped(false)
{
    //The amount of energy going into the plant is limited by the seed
    //with the least energy.
    double minSeedEnergy = std::min(seed1.getEnergy(), seed2.getEnergy());
    m_energy = 2.0 * minSeedEnergy;

    setColorsWithRandomness();
}


//This constructor makes the organisms that are stored in the Stats object.
Organism::Organism(Genome genome, double generation) :
    m_energy(0.0), m_genome(new Genome(genome)),
    m_generation(generation), m_randomness(0.0), m_historyOrganism(true),
    m_energyFromPhotosynthesis(0.0), m_energySpentOnGrowthAndMaintenance(0.0), m_energySpentOnReproduction(0.0),
    m_helped(false)
{
    m_firstPart = new PlantPart(this, 0, 0, Point2D(0.0, 0.0));
    setColorsWithoutRandomness();
}


Organism::~Organism()
{
    delete m_firstPart;
}



void Organism::setColorsWithRandomness()
{
    int branchHue, branchSaturation, branchLightness;
    g_simulationSettings->branchFillColor.getHsl(&branchHue, &branchSaturation, &branchLightness);

    int leafHue, leafSaturation, leafLightness;
    g_simulationSettings->leafColor.getHsl(&leafHue, &leafSaturation, &leafLightness);

    int maxBranchVariation = g_simulationSettings->branchColorVariation;
    int minBranchVariation = -1 * g_simulationSettings->branchColorVariation;

    int newBranchHue = branchHue + g_randomNumbers->getRandomInt(minBranchVariation, maxBranchVariation);
    newBranchHue = constrainNumber(newBranchHue, 0, 359);
    int newBranchSaturation = branchSaturation + g_randomNumbers->getRandomInt(minBranchVariation, maxBranchVariation);
    newBranchSaturation = constrainNumber(newBranchSaturation, 0, 255);
    int newBranchLightness = branchLightness + g_randomNumbers->getRandomInt(minBranchVariation, maxBranchVariation);
    newBranchLightness = constrainNumber(newBranchLightness, 0, 255);

    int maxLeafVariation = g_simulationSettings->leafColorVariation;
    int minLeafVariation = -1 * g_simulationSettings->leafColorVariation;

    int newLeafHue = leafHue + g_randomNumbers->getRandomInt(minLeafVariation, maxLeafVariation);
    newLeafHue = constrainNumber(newLeafHue, 0, 359);
    int newLeafSaturation = leafSaturation + g_randomNumbers->getRandomInt(minLeafVariation, maxLeafVariation);
    newLeafSaturation = constrainNumber(newLeafSaturation, 0, 255);
    int newLeafLightness = leafLightness + g_randomNumbers->getRandomInt(minLeafVariation, maxLeafVariation);
    newLeafLightness = constrainNumber(newLeafLightness, 0, 255);

    QColor branchColor;
    branchColor.setHsl(newBranchHue, newBranchSaturation, newBranchLightness);

    QColor leafColor;
    leafColor.setHsl(newLeafHue, newLeafSaturation, newLeafLightness);

    m_branchRed = branchColor.red();
    m_branchGreen = branchColor.green();
    m_branchBlue = branchColor.blue();

    m_leafRed = leafColor.red();
    m_leafGreen = leafColor.green();
    m_leafBlue = leafColor.blue();
}


int Organism::constrainNumber(int number, int min, int max) const
{
    if (number < min)
        number = min;
    else if (number > max)
        number = max;
    return number;
}

void Organism::setColorsWithoutRandomness()
{
    m_branchRed = g_simulationSettings->branchFillColor.red();
    m_branchGreen = g_simulationSettings->branchFillColor.green();
    m_branchBlue = g_simulationSettings->branchFillColor.blue();
    m_leafRed = g_simulationSettings->leafColor.red();
    m_leafGreen = g_simulationSettings->leafColor.green();
    m_leafBlue = g_simulationSettings->leafColor.blue();
}






//This function will add all of the necessary shapes to the following containers
//so the organism can be drawn.  It does this by passing the pointers to the first
//PlantPart which will add its shape(s) and then pass the pointers on to any
//children it has.
//The alwaysDraw parameter controls whether this function takes the visible area
//into account.  If false, an organism is only drawn when its region of the
//environment is visible.  If true, it is always drawn.  True is used for things
//like the SingleOrganismWidget and saving images.
void Organism::drawOrganism(QPainter * painter, double environmentHeight, bool highlight, bool alwaysDraw) const
{
    QColor branchFillColor;
    QColor branchLineColor;
    QColor leafColor;
    if (!m_historyOrganism)
    {
        branchFillColor = QColor(m_branchRed, m_branchGreen, m_branchBlue);
        branchLineColor = QColor(m_branchRed-8, m_branchGreen-8, m_branchBlue-8);
        leafColor = QColor(m_leafRed, m_leafGreen, m_leafBlue);
    }
    else
    {
        branchFillColor = g_simulationSettings->branchFillColor;
        branchLineColor = QColor(branchFillColor.red()-8, branchFillColor.green()-8, branchFillColor.blue()-8);
        leafColor = g_simulationSettings->leafColor;
    }

    //Create and fill the shape vectors.
    std::vector<QLineF> branchLines;
    std::vector<double> branchWidths;
    std::vector<QLineF> leafLines;
    std::vector<QLineF> seedpodsLines;
    std::vector<QRectF> seedpodsEnds;
    m_firstPart->getShapesForDrawing(&branchLines, &branchWidths, &leafLines, &seedpodsLines, &seedpodsEnds, environmentHeight, alwaysDraw);

    if (m_helped)
    {
        drawBranches(painter, highlight, true, &branchLines, &branchWidths, &branchFillColor, &branchLineColor);
        drawLeaves(painter, highlight, true, &leafLines, &leafColor);
        drawSeedpods(painter, highlight, true, &seedpodsLines, &seedpodsEnds);
    }

    drawBranches(painter, highlight, false, &branchLines, &branchWidths, &branchFillColor, &branchLineColor);
    drawLeaves(painter, highlight, false, &leafLines, &leafColor);
    drawSeedpods(painter, highlight, false, &seedpodsLines, &seedpodsEnds);
}


void Organism::drawBranches(QPainter * painter, bool highlight, bool helpingLayer,
                            std::vector<QLineF> * branchLines, std::vector<double> * branchWidths,
                            QColor * branchFillColor, QColor * branchLineColor) const
{
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);

    if (helpingLayer)
        pen.setBrush(g_simulationSettings->helpedColor);
    else if (highlight)
        pen.setBrush(g_simulationSettings->organismHighlightColor);
    else
        pen.setBrush(*branchFillColor);

    double extraThickness = 0.0;
    if (helpingLayer)
        extraThickness = 2.0 * g_simulationSettings->helpedBorderThickness;

    for (size_t i = 0; i < branchLines->size(); ++i)
    {
        pen.setWidth((*branchWidths)[i] + extraThickness);
        painter->setPen(pen);
        painter->drawLine((*branchLines)[i]);
    }

    if (helpingLayer)
        pen.setBrush(g_simulationSettings->helpedColor);
    else if (highlight)
        pen.setBrush(g_simulationSettings->organismHighlightColor);
    else
        pen.setBrush(*branchLineColor);

    pen.setWidth(g_simulationSettings->branchLineThickness + extraThickness);
    painter->setPen(pen);
    for (std::vector<QLineF>::const_iterator i = branchLines->begin(); i != branchLines->end(); ++i)
        painter->drawLine(*i);
}


void Organism::drawLeaves(QPainter * painter, bool highlight, bool helpingLayer,
                          std::vector<QLineF> * leafLines,
                          QColor * leafColor) const
{
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);
    pen.setWidth(g_simulationSettings->leafThickness);
    if (helpingLayer)
        pen.setBrush(g_simulationSettings->helpedColor);
    else if (highlight)
        pen.setBrush(g_simulationSettings->organismHighlightColor);
    else
        pen.setBrush(*leafColor);

    if (helpingLayer)
        pen.setWidth(g_simulationSettings->leafThickness + 2.0 * g_simulationSettings->helpedBorderThickness);
    else
        pen.setWidth(g_simulationSettings->leafThickness);

    painter->setPen(pen);
    for (std::vector<QLineF>::const_iterator i = leafLines->begin(); i != leafLines->end(); ++i)
        painter->drawLine(*i);
}

void Organism::drawSeedpods(QPainter * painter, bool highlight, bool helpingLayer,
                            std::vector<QLineF> * seedpodsLines, std::vector<QRectF> * seedpodsEnds) const
{
    QPen pen;
    pen.setCapStyle(Qt::RoundCap);

    if (helpingLayer)
        pen.setBrush(g_simulationSettings->helpedColor);
    else if (highlight)
        pen.setBrush(g_simulationSettings->organismHighlightColor);
    else
        pen.setBrush(g_simulationSettings->seedpodColor);

    double extraThickness = 0.0;
    if (helpingLayer)
        extraThickness = g_simulationSettings->helpedBorderThickness;
    pen.setWidth(g_simulationSettings->seedpodThickness + 2.0 * extraThickness);
    painter->setPen(pen);

    for (std::vector<QLineF>::const_iterator i = seedpodsLines->begin(); i != seedpodsLines->end(); ++i)
        painter->drawLine(*i);
    painter->setPen(Qt::NoPen);

    if (helpingLayer)
        painter->setBrush(g_simulationSettings->helpedColor);
    else if (highlight)
        painter->setBrush(g_simulationSettings->organismHighlightColor);
    else
        painter->setBrush(g_simulationSettings->seedpodColor);

    for (std::vector<QRectF>::const_iterator i = seedpodsEnds->begin(); i != seedpodsEnds->end(); ++i)
    {
        if (helpingLayer)
            painter->drawEllipse(i->adjusted(-extraThickness, -extraThickness, extraThickness, extraThickness));
        else
            painter->drawEllipse(*i);
    }
}

void Organism::growOneTick()
{
    m_firstPart->growOneTick();
}

void Organism::transmitLoadAndGrowWidthOneTick()
{
    m_firstPart->calculateCenterOfMass();
}

void Organism::addLeavesToLightingVector(std::vector<PlantPart *> * leaves)
{
    m_firstPart->addLeavesToLightingVector(leaves);
}

void Organism::useEnergyOneTick()
{
    if (!m_historyOrganism)
    {
        double maintenanceCost = g_simulationSettings->organismMaintenanceCost + m_firstPart->getMaintenanceCost();
        m_energy -= maintenanceCost;
        m_energySpentOnGrowthAndMaintenance += maintenanceCost;
    }
}

void Organism::createSeeds(std::deque<Seed> *seeds, long long elapsedTime, bool dayTime)
{
    m_firstPart->createSeeds(seeds, elapsedTime, dayTime);
}

void Organism::age(int ticksToAge)
{
    for (int i = 0; i < ticksToAge; ++i)
    {
        growOneTick();
        transmitLoadAndGrowWidthOneTick();
    }
}

bool Organism::isFinishedGrowing() const
{
    return m_firstPart->isFinishedGrowing();
}

double Organism::getHeight() const
{
    return m_firstPart->getHighestPoint();
}

double Organism::getHighestDrawnPoint() const
{
    if (m_helped)
        return m_firstPart->getHighestDrawnPoint() + g_simulationSettings->helpedBorderThickness;
    else
        return m_firstPart->getHighestDrawnPoint();
}

double Organism::getRightmostDrawnPoint() const
{
    if (m_helped)
        return m_firstPart->getRightmostDrawnPoint() + g_simulationSettings->helpedBorderThickness;
    else
        return m_firstPart->getRightmostDrawnPoint();
}

double Organism::getLeftmostDrawnPoint() const
{
    if (m_helped)
        return m_firstPart->getLeftmostDrawnPoint() - g_simulationSettings->helpedBorderThickness;
    else
        return m_firstPart->getLeftmostDrawnPoint();
}

bool Organism::isPointInsideOrganism(Point2D point) const
{
    return m_firstPart->isPointInsidePart(point);
}

long long Organism::getAge(long long elapsedTime) const
{
    return elapsedTime - m_birthDate;
}

int Organism::getLeafCount() const
{
    return m_firstPart->getLeafCount();
}

int Organism::getBranchCount() const
{
    return m_firstPart->getBranchCount();
}

int Organism::getSeedpodCount() const
{
    return m_firstPart->getSeedpodCount();
}

int Organism::getPlantPartCount() const
{
    return m_firstPart->getPlantPartCount();
}

double Organism::getMass() const
{
    return m_firstPart->getMassHereAndAbove();
}

double Organism::getSeedX() const
{
    return m_firstPart->getStart().m_x;
}
