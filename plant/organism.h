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


#ifndef ORGANISM_H
#define ORGANISM_H

#include <vector>
#include <deque>
#include <QPolygonF>
#include <QLineF>
#include <QRectF>
#include <QPainter>
#include <QColor>
#include "../program/globals.h"
#include "../program/point2d.h"
#include "genome.h"
#include "../program/globals.h"

#ifndef Q_MOC_RUN
#include "boost/utility.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/serialization/shared_ptr.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class PlantPart;
class Environment;
class Seed;
class GeneAnnotation;

class Organism : boost::noncopyable
{
public:
    Organism() {}
    Organism(double energy, long long elapsedTime, double xPos);
    Organism(Seed & seed1, Seed & seed2, long long elapsedTime, double xPos);
    Organism(Genome genome, double generation);
    ~Organism();

    void growOneTick();
    void transmitLoadAndGrowWidthOneTick();
    void useEnergyOneTick();
    double getMaintenanceCost() const;
    void deductEnergy(double energyToDeduct) {m_energy -= energyToDeduct;}
    void addEnergy(double energyToAdd) {m_energy += energyToAdd;}
    void age(int ticksToAge);
    void createSeeds(std::deque<Seed> * seeds, long long elapsedTime, bool dayTime);
    void addToEnergyFromPhotosynthesis(double energy) {m_energyFromPhotosynthesis += energy;}
    void addToEnergySpentOnGrowthAndMaintenance(double energy) {m_energySpentOnGrowthAndMaintenance += energy;}
    void addToEnergySpentOnReproduction(double energy) {m_energySpentOnReproduction += energy;}
    void addLeavesToLightingVector(std::vector<PlantPart *> * leaves);
    void setGeneration(double newGeneration) {m_generation = newGeneration;}
    void resetBirthDate() {m_birthDate = 0;}
    void help() {m_helped = true;}
    void drawOrganism(QPainter * painter, double environmentHeight, bool highlight = false, bool alwaysDraw = false) const;
    bool isPointInsideOrganism(Point2D point) const;
    bool isFinishedGrowing() const;
    double getHeight() const;
    long long getAge(long long elapsedTime) const;
    double getSeedX() const;
    double getHighestDrawnPoint() const;
    double getRightmostDrawnPoint() const;
    double getLeftmostDrawnPoint() const;
    double getEnergy() const {return m_energy;}
    boost::shared_ptr<Genome> getGenomeSharedPointer() {return m_genome;}
    Genome * getGenome() const {return m_genome.get();}
    double getGeneration() const {return m_generation;}
    double getRandomness() const {return m_randomness;}
    bool isHistoryOrganism() const {return m_historyOrganism;}
    int getLeafCount() const;
    int getBranchCount() const;
    int getSeedpodCount() const;
    int getPlantPartCount() const;
    double getMass() const;
    double getEnergyFromPhotosynthesis() const {return m_energyFromPhotosynthesis;}
    double getEnergySpentOnGrowthAndMaintenance() const {return m_energySpentOnGrowthAndMaintenance;}
    double getEnergySpentOnReproduction() const {return m_energySpentOnReproduction;}
    bool isHelped() const {return m_helped;}

private:
    double m_energy;
    boost::shared_ptr<Genome> m_genome;
    long long m_birthDate;
    double m_generation;
    double m_randomness;
    bool m_historyOrganism; //True if this object isn't in the environment but is instead part of the history record.
    int m_branchRed, m_branchGreen, m_branchBlue;
    int m_leafRed, m_leafGreen, m_leafBlue;
    double m_energyFromPhotosynthesis;
    double m_energySpentOnGrowthAndMaintenance;
    double m_energySpentOnReproduction;
    PlantPart * m_firstPart;
    bool m_helped;

    void drawBranches(QPainter * painter, bool highlight, bool helpingLayer,
                      std::vector<QLineF> * branchLines, std::vector<double> * branchWidths,
                      QColor * branchFillColor, QColor * branchLineColor) const;
    void drawLeaves(QPainter * painter, bool highlight, bool helpingLayer,
                    std::vector<QLineF> * leafLines, QColor * leafColor) const;
    void drawSeedpods(QPainter * painter, bool highlight, bool helpingLayer,
                      std::vector<QLineF> * seedpodsLines,
                      std::vector<QRectF> * seedpodsEnds) const;
    void setColorsWithRandomness();
    void setColorsWithoutRandomness();
    int constrainNumber(int number, int min, int max) const;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_energy;
        ar & m_genome;
        ar & m_firstPart;
        ar & m_birthDate;
        ar & m_generation;
        ar & m_randomness;
        ar & m_historyOrganism;
        ar & m_branchRed;
        ar & m_branchGreen;
        ar & m_branchBlue;
        ar & m_leafRed;
        ar & m_leafGreen;
        ar & m_leafBlue;
        ar & m_energyFromPhotosynthesis;
        ar & m_energySpentOnGrowthAndMaintenance;
        ar & m_energySpentOnReproduction;
        ar & m_helped;

        if (isHistoryOrganism())
            ++g_historyOrganismsSavedOrLoaded;
        else
            ++g_organismsSavedOrLoaded;
    }
};

#endif // ORGANISM_H
