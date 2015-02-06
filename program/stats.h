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


#ifndef STATS_H
#define STATS_H

#include <vector>
#include "globals.h"

#ifndef Q_MOC_RUN
#include "boost/serialization/vector.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class Environment;
class Organism;

class Stats
{
public:
    Stats();
    ~Stats();

    long long m_numberOfOrganismsSprouted;
    long long m_numberOfOrganismsDiedFromBadLuck;
    long long m_numberOfOrganismsDiedFromStarvation;
    long long m_numberOfSeedsGenerated;
    std::vector<double> m_time;
    std::vector<double> m_populationDensity;
    std::vector<double> m_tallestPlantHeight;
    std::vector<double> m_meanHeightOfFullyGrownPlants;
    std::vector<double> m_heaviestPlantMass;
    std::vector<double> m_meanMassOfFullyGrownPlants;
    std::vector<double> m_meanSeedsPerPlant;
    std::vector<double> m_meanEnergyPerSeed;
    std::vector<Organism *> m_averageGenomeOrganism;
    std::vector<Organism *> m_randomGenomeOrganism;

    void reset();
    void addToLog(Environment * environment);
    void deleteHalfOfAllData();
    int logEntries() const {return int(m_time.size());}
    double getHistoryOrganismHeightExtent(HistoryOrganismType historyOrganismType);
    double getHistoryOrganismRightExtent(HistoryOrganismType historyOrganismType);
    double getHistoryOrganismLeftExtent(HistoryOrganismType historyOrganismType);

private:
    void cleanUp();
    void deleteHalfOfOneDoubleDataGroup(std::vector<double> * data);
    void deleteHalfOfOneOrganismDataGroup(std::vector<Organism *> * data);
    std::vector<Organism *>::const_iterator getBeginIterator(HistoryOrganismType historyOrganismType);
    std::vector<Organism *>::const_iterator getEndIterator(HistoryOrganismType historyOrganismType);

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_numberOfOrganismsSprouted;
        ar & m_numberOfOrganismsDiedFromBadLuck;
        ar & m_numberOfOrganismsDiedFromStarvation;
        ar & m_numberOfSeedsGenerated;
        ar & m_time;
        ar & m_populationDensity;
        ar & m_tallestPlantHeight;
        ar & m_meanHeightOfFullyGrownPlants;
        ar & m_heaviestPlantMass;
        ar & m_meanMassOfFullyGrownPlants;
        ar & m_meanSeedsPerPlant;
        ar & m_meanEnergyPerSeed;
        ar & m_averageGenomeOrganism;
        ar & m_randomGenomeOrganism;
    }
};

#endif // STATS_H
