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


#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <vector>
#include <deque>
#include <list>
#include <QPainter>
#include <QDateTime>
#include "globals.h"
#include "../plant/organism.h"
#include "../lighting/lighting.h"
#include "../settings/simulationsettings.h"
#include "../settings/environmentsettings.h"
#include "../plant/seed.h"
#include "../plant/genome.h"

#ifndef Q_MOC_RUN
#include "boost/serialization/list.hpp"
#include "boost/serialization/deque.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class Environment
{
public:
    Environment();
    ~Environment();

    void cleanUp();
    void reset();
    void resetTime();
    void advanceOneTick();
    bool possiblyChangeEnvironmentSize();
    void logStats();
    void drawOrganism(QPainter * painter, const Organism * organism, const Organism * highlightOrganism, bool alwaysDraw) const {organism->drawOrganism(painter, m_height, organism == highlightOrganism, alwaysDraw);}
    Organism * findOrganismUnderPoint(Point2D point) const;
    void addLeavesToVector(std::vector<PlantPart *> *leafVector);
    void setWidth(int newWidth);
    void setDateAndTimeOfSimStart() {m_dateAndTimeOfSimStart = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss").toStdString(); lastStartTime = QDateTime::currentDateTime();}
    void setLastStartTime(QDateTime newLastStartTime) {lastStartTime = newLastStartTime;}
    void setElapsedTime(long long newTime) {m_elapsedTime = newTime;}
    void resetAllGenerations();
    void killOrganism(Organism * organism);
    void helpOrganism(Organism * organism);
    bool populationIsExtinct() const {return getOrganismCount() == 0 && getSeedCount() == 0;}
    bool populationIsNotExtinct() const {return !populationIsExtinct();}
    long long getElapsedTime() const {return m_elapsedTime;}
    size_t getOrganismCount() const {return m_organisms.size();}
    size_t getSeedCount() const {return m_seeds.size() - m_numberOfNullSeeds;}
    bool isDaytime() const {return getDayProgression() < g_simulationSettings->dayLength;}
    int getDayProgression() const {return m_elapsedTime % (g_simulationSettings->dayLength + g_simulationSettings->nightLength);}
    int getHeight() const {return m_height;}
    int getWidth() const {return m_width;}
    double getSunAngle() const;
    double getAverageGeneration() const;
    double getTallestPlantHeight() const;

    void getPlantHeightPercentiles(double * tallestPlantHeight, double * ninetyNinthPercentilePlantHeight,
                                   double * ninetiethPercentilePlantHeight, double * medianPlantHeight) const;
    void getPlantMassPercentiles(double * heaviestPlantMass, double * ninetyNinthPercentilePlantMass,
                                 double * ninetiethPercentilePlantMass, double * medianPlantMass) const;
    void getPlantEnergyPercentiles(double * mostPlantEnergy, double * ninetyNinthPercentilePlantEnergy,
                                   double * ninetiethPercentilePlantEnergy, double * medianPlantEnergy) const;
    void getPercentilesOfDoubleVector(std::vector<double> * doubleVector,
                                      double * max, double * ninetyNinthPercentile,
                                      double * ninetiethPercentile, double * median) const;

    double getFullyGrownPlantFraction() const;
    int getFullyGrownPlantCount() const;
    double getAverageEnergyPerSeed() const;
    double getMeanPartsPerPlant(PlantPartType partType) const;
    Genome getModeGenome() const;
    const Organism * getOldestOrganism() const;
    int getLogIntervalMultiplier() const {return m_logIntervalMultiplier;}
    const std::list<Organism *> * getOrganismList() const {return &m_organisms;}
    int getMaxGenomeLength() const;
    const Organism *getRandomGrownOrganism() const;
    double getElapsedRealWorldSeconds() const {return m_elapsedRealWorldSeconds;}
    void addToElapsedRealWorldSeconds(double newSeconds) {m_elapsedRealWorldSeconds += newSeconds;}
    double getPopulationDensity() const {return double(getOrganismCount()) / m_width;}
    double getMeanSeedsPerPlant() const {if (getOrganismCount() == 0) return 0.0; else return double(getSeedCount()) / getOrganismCount();}
    double getSunIntensity() const;
    int getMode(std::vector<int> * numbers) const;
    std::vector<const Organism *> getGrownOrganisms() const;
    std::vector<const Organism *> getOldOrganisms() const;
    QString getDateAndTimeOfSimStart() const {return QString::fromStdString(m_dateAndTimeOfSimStart);}
    QDateTime getLastStartTime() const {return lastStartTime;}
    QString outputAllInfoOnCurrentPopulation() const;

private:
    int m_width;
    int m_height;
    long long m_elapsedTime;
    std::list<Organism *> m_organisms;
    std::deque<Seed> m_seeds;
    int m_numberOfNullSeeds;
    int m_logIntervalMultiplier;
    double m_elapsedRealWorldSeconds;
    std::string m_dateAndTimeOfSimStart;
    QDateTime lastStartTime;

    void killOffStarvedAndUnluckyOrganisms();
    void getRidOfOldSeeds();
    void createNewOrganisms();
    void distributeLightToLeaves();

signals:
    void addToWaitingDialog(QString text);

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_width;
        ar & m_height;
        ar & m_elapsedTime;
        ar & m_organisms;
        ar & m_seeds;
        ar & m_numberOfNullSeeds;
        ar & m_logIntervalMultiplier;
        ar & m_elapsedRealWorldSeconds;
        ar & m_dateAndTimeOfSimStart;
    }
};

#endif // ENVIRONMENT_H
