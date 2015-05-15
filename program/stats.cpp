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


#include "stats.h"
#include "environment.h"
#include "../settings/simulationsettings.h"
#include "../plant/organism.h"

Stats::Stats()
{
    reset();
}

Stats::~Stats()
{
    cleanUp();
}


void Stats::reset()
{
    m_numberOfOrganismsSprouted = 0;
    m_numberOfOrganismsDiedFromBadLuck = 0;
    m_numberOfOrganismsDiedFromStarvation = 0;
    m_numberOfSeedsGenerated = 0;

    cleanUp();
}

void Stats::cleanUp()
{
    m_time.clear();
    m_populationDensity.clear();
    m_tallestPlantHeight.clear();
    m_meanHeightOfFullyGrownPlants.clear();
    m_heaviestPlantMass.clear();
    m_meanMassOfFullyGrownPlants.clear();
    m_meanSeedsPerPlant.clear();
    m_meanEnergyPerSeed.clear();
    m_meanEnergyPerPlant.clear();
    m_meanMaintenanceCostPerPlant.clear();
    m_meanDeathAge.clear();
    m_meanNonStarvationDeathAge.clear();

    for (std::vector<Organism *>::const_iterator i = m_averageGenomeOrganism.begin();
         i != m_averageGenomeOrganism.end(); ++i)
        delete *i;
    m_averageGenomeOrganism.clear();

    for (std::vector<Organism *>::const_iterator i = m_randomGenomeOrganism.begin();
         i != m_randomGenomeOrganism.end(); ++i)
        delete *i;
    m_randomGenomeOrganism.clear();
}




void Stats::addToLog(Environment * environment)
{
    m_time.push_back(environment->getElapsedTime());
    m_populationDensity.push_back(environment->getPopulationDensity());
    m_tallestPlantHeight.push_back(environment->getTallestPlantHeight());
    m_meanHeightOfFullyGrownPlants.push_back(environment->getAverageHeightOfFullyGrownPlants());
    m_heaviestPlantMass.push_back(environment->getHeaviestPlantMass());
    m_meanMassOfFullyGrownPlants.push_back(environment->getMeanMassOfFullyGrownPlants());
    m_meanSeedsPerPlant.push_back(environment->getMeanSeedsPerPlant());
    m_meanEnergyPerSeed.push_back(environment->getAverageEnergyPerSeed());
    m_meanEnergyPerPlant.push_back(10.0);  //TEMP - WILL NEED TO MAKE AN ACTUAL FUNCTION FOR THIS
    m_meanMaintenanceCostPerPlant.push_back(20.0);  //TEMP - WILL NEED TO MAKE AN ACTUAL FUNCTION FOR THIS
    m_meanDeathAge.push_back(40.0);  //TEMP - WILL NEED TO MAKE AN ACTUAL FUNCTION FOR THIS
    m_meanNonStarvationDeathAge.push_back(80.0);  //TEMP - WILL NEED TO MAKE AN ACTUAL FUNCTION FOR THIS

    if (environment->getOrganismCount() == 0)
    {
        m_averageGenomeOrganism.push_back(0);
        m_randomGenomeOrganism.push_back(0);
    }
    else
    {
        Organism * newAverageGenomeOrganism = new Organism(environment->getModeGenome(),
                                                           environment->getAverageGeneration());
        newAverageGenomeOrganism->age(2 * g_simulationSettings->getAverageNonStarvedAge());  //Double the non-starved age should be enough...
        m_averageGenomeOrganism.push_back(newAverageGenomeOrganism);

        const Organism * randomOrganism = environment->getRandomGrownOrganism();
        Organism * newRandomGenomeOrganism = new Organism(*(randomOrganism->getGenome()),
                                                          randomOrganism->getGeneration());
        newRandomGenomeOrganism->age(2 * g_simulationSettings->getAverageNonStarvedAge());
        m_randomGenomeOrganism.push_back(newRandomGenomeOrganism);
    }
}



//Should only be called when there are an even number of data points in each vector.
void Stats::deleteHalfOfAllData()
{
    deleteHalfOfOneDoubleDataGroup(&m_time);
    deleteHalfOfOneDoubleDataGroup(&m_populationDensity);
    deleteHalfOfOneDoubleDataGroup(&m_tallestPlantHeight);
    deleteHalfOfOneDoubleDataGroup(&m_meanHeightOfFullyGrownPlants);
    deleteHalfOfOneDoubleDataGroup(&m_heaviestPlantMass);
    deleteHalfOfOneDoubleDataGroup(&m_meanMassOfFullyGrownPlants);
    deleteHalfOfOneDoubleDataGroup(&m_meanSeedsPerPlant);
    deleteHalfOfOneDoubleDataGroup(&m_meanEnergyPerSeed);
    deleteHalfOfOneDoubleDataGroup(&m_meanEnergyPerPlant);
    deleteHalfOfOneDoubleDataGroup(&m_meanMaintenanceCostPerPlant);
    deleteHalfOfOneDoubleDataGroup(&m_meanDeathAge);
    deleteHalfOfOneDoubleDataGroup(&m_meanNonStarvationDeathAge);

    deleteHalfOfOneOrganismDataGroup(&m_averageGenomeOrganism);
    deleteHalfOfOneOrganismDataGroup(&m_randomGenomeOrganism);
}


void Stats::deleteHalfOfOneDoubleDataGroup(std::vector<double> * data)
{
    std::vector<double> newData;
    for (size_t i = 0; i < data->size(); i += 2)
        newData.push_back((*data)[i]);
    *data = newData;
}


void Stats::deleteHalfOfOneOrganismDataGroup(std::vector<Organism *> * data)
{
    std::vector<Organism *> newData;
    for (size_t i = 0; i < data->size(); ++i)
    {
        if (i % 2 == 0)
            newData.push_back((*data)[i]);
        else
            delete (*data)[i];
    }
    *data = newData;
}



//THE FOLLOWING FUNCTIONS ARE VERY REPETITIVE.  I'M SURE THEY
//COULD BE MUCH IMPROVED.
double Stats::getHistoryOrganismHeightExtent(HistoryOrganismType historyOrganismType)
{
    double maxHeight = 0.0;

    std::vector<Organism *>::const_iterator i = getBeginIterator(historyOrganismType);
    std::vector<Organism *>::const_iterator end = getEndIterator(historyOrganismType);
    for (; i != end; ++i)
    {
        if ((*i) != 0)
        {
            double organismHighPoint = (*i)->getHighestDrawnPoint();
            if (organismHighPoint > maxHeight)
                maxHeight = organismHighPoint;
        }
    }
    return maxHeight;
}
double Stats::getHistoryOrganismRightExtent(HistoryOrganismType historyOrganismType)
{
    double maxRight = 0.0;

    std::vector<Organism *>::const_iterator i = getBeginIterator(historyOrganismType);
    std::vector<Organism *>::const_iterator end = getEndIterator(historyOrganismType);
    for (; i != end; ++i)
    {
        if ((*i) != 0)
        {
            double organismRightPoint = (*i)->getRightmostDrawnPoint();
            if (organismRightPoint > maxRight)
                maxRight = organismRightPoint;
        }
    }
    return maxRight;
}
double Stats::getHistoryOrganismLeftExtent(HistoryOrganismType historyOrganismType)
{
    double minLeft = std::numeric_limits<double>::max();

    std::vector<Organism *>::const_iterator i = getBeginIterator(historyOrganismType);
    std::vector<Organism *>::const_iterator end = getEndIterator(historyOrganismType);
    for (; i != end; ++i)
    {
        if ((*i) != 0)
        {
            double organismLeftPoint = (*i)->getLeftmostDrawnPoint();
            if (organismLeftPoint < minLeft)
                minLeft = organismLeftPoint;
        }
    }
    return minLeft;
}

std::vector<Organism *>::const_iterator Stats::getBeginIterator(HistoryOrganismType historyOrganismType)
{
    if (historyOrganismType == AVERAGE_GENOME)
        return m_averageGenomeOrganism.begin();
    else
        return m_randomGenomeOrganism.begin();
}
std::vector<Organism *>::const_iterator Stats::getEndIterator(HistoryOrganismType historyOrganismType)
{
    if (historyOrganismType == AVERAGE_GENOME)
        return m_averageGenomeOrganism.end();
    else
        return m_randomGenomeOrganism.end();
}
