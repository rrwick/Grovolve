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
    m_numberOfOrganismsDiedFromBadLuckSinceLastLog = 0;
    m_numberOfOrganismsDiedFromStarvationSinceLastLog = 0;
    m_organismsDiedFromBadLuckSinceLastLogAgeSum = 0;
    m_organismsDiedFromStarvationSinceLastLogAgeSum = 0;

    cleanUp();
}

void Stats::cleanUp()
{
    m_time.clear();
    m_populationDensity.clear();

    m_tallestPlantHeight.clear();
    m_99thPercentilePlantHeight.clear();
    m_95thPercentilePlantHeight.clear();
    m_90thPercentilePlantHeight.clear();
    m_medianPlantHeight.clear();

    m_heaviestPlantMass.clear();
    m_99thPercentilePlantMass.clear();
    m_95thPercentilePlantMass.clear();
    m_90thPercentilePlantMass.clear();
    m_medianPlantMass.clear();

    m_mostPlantEnergy.clear();
    m_99thPercentilePlantEnergy.clear();
    m_95thPercentilePlantEnergy.clear();
    m_90thPercentilePlantEnergy.clear();
    m_medianPlantEnergy.clear();

    m_meanSeedsPerPlant.clear();
    m_meanEnergyPerSeed.clear();
    m_meanDeathAge.clear();
    m_meanStarvationDeathAge.clear();
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

    double tallestPlantHeight;
    double ninetyNinthPercentilePlantHeight;
    double ninetyFifthPercentilePlantHeight;
    double ninetiethPercentilePlantHeight;
    double medianPlantHeight;
    environment->getPlantHeightPercentiles(&tallestPlantHeight,
                                           &ninetyNinthPercentilePlantHeight,
                                           &ninetyFifthPercentilePlantHeight,
                                           &ninetiethPercentilePlantHeight,
                                           &medianPlantHeight);
    m_tallestPlantHeight.push_back(tallestPlantHeight);
    m_99thPercentilePlantHeight.push_back(ninetyNinthPercentilePlantHeight);
    m_95thPercentilePlantHeight.push_back(ninetyFifthPercentilePlantHeight);
    m_90thPercentilePlantHeight.push_back(ninetiethPercentilePlantHeight);
    m_medianPlantHeight.push_back(medianPlantHeight);

    double heaviestPlantMass;
    double ninetyNinthPercentilePlantMass;
    double ninetyFifthPercentilePlantMass;
    double ninetiethPercentilePlantMass;
    double medianPlantMass;
    environment->getPlantMassPercentiles(&heaviestPlantMass,
                                         &ninetyNinthPercentilePlantMass,
                                         &ninetyFifthPercentilePlantMass,
                                         &ninetiethPercentilePlantMass,
                                         &medianPlantMass);
    m_heaviestPlantMass.push_back(heaviestPlantMass);
    m_99thPercentilePlantMass.push_back(ninetyNinthPercentilePlantMass);
    m_95thPercentilePlantMass.push_back(ninetyFifthPercentilePlantMass);
    m_90thPercentilePlantMass.push_back(ninetiethPercentilePlantMass);
    m_medianPlantMass.push_back(medianPlantMass);

    double mostPlantEnergy;
    double ninetyNinthPercentilePlantEnergy;
    double ninetyFifthPercentilePlantEnergy;
    double ninetiethPercentilePlantEnergy;
    double medianPlantEnergy;
    environment->getPlantEnergyPercentiles(&mostPlantEnergy,
                                           &ninetyNinthPercentilePlantEnergy,
                                           &ninetyFifthPercentilePlantEnergy,
                                           &ninetiethPercentilePlantEnergy,
                                           &medianPlantEnergy);
    m_mostPlantEnergy.push_back(mostPlantEnergy);
    m_99thPercentilePlantEnergy.push_back(ninetyNinthPercentilePlantEnergy);
    m_95thPercentilePlantEnergy.push_back(ninetyFifthPercentilePlantEnergy);
    m_90thPercentilePlantEnergy.push_back(ninetiethPercentilePlantEnergy);
    m_medianPlantEnergy.push_back(medianPlantEnergy);

    m_meanSeedsPerPlant.push_back(environment->getMeanSeedsPerPlant());
    m_meanEnergyPerSeed.push_back(environment->getAverageEnergyPerSeed());

    long long totalNumberOfOrganismsDiedSinceLastLog = m_numberOfOrganismsDiedFromBadLuckSinceLastLog + m_numberOfOrganismsDiedFromStarvationSinceLastLog;
    long long totalOrganismsDiedSinceLastLogAgeSum = m_organismsDiedFromBadLuckSinceLastLogAgeSum + m_organismsDiedFromStarvationSinceLastLogAgeSum;

    double meanDeathAge = 0.0;
    double meanStarvationDeathAge = 0.0;
    double meanNonStarvationDeathAge = 0.0;

    if (totalNumberOfOrganismsDiedSinceLastLog > 0)
        meanDeathAge = double(totalOrganismsDiedSinceLastLogAgeSum) / totalNumberOfOrganismsDiedSinceLastLog;
    if (m_numberOfOrganismsDiedFromStarvationSinceLastLog > 0)
        meanStarvationDeathAge = double(m_organismsDiedFromStarvationSinceLastLogAgeSum) / m_numberOfOrganismsDiedFromStarvationSinceLastLog;
    if (m_numberOfOrganismsDiedFromBadLuckSinceLastLog > 0)
        meanNonStarvationDeathAge = double(m_organismsDiedFromBadLuckSinceLastLogAgeSum) / m_numberOfOrganismsDiedFromBadLuckSinceLastLog;

    m_meanDeathAge.push_back(meanDeathAge);
    m_meanStarvationDeathAge.push_back(meanStarvationDeathAge);
    m_meanNonStarvationDeathAge.push_back(meanNonStarvationDeathAge);

    m_numberOfOrganismsDiedFromBadLuckSinceLastLog = 0;
    m_numberOfOrganismsDiedFromStarvationSinceLastLog = 0;
    m_organismsDiedFromBadLuckSinceLastLogAgeSum = 0;
    m_organismsDiedFromStarvationSinceLastLogAgeSum = 0;

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
    deleteHalfOfOneDoubleDataGroup(&m_99thPercentilePlantHeight);
    deleteHalfOfOneDoubleDataGroup(&m_95thPercentilePlantHeight);
    deleteHalfOfOneDoubleDataGroup(&m_90thPercentilePlantHeight);
    deleteHalfOfOneDoubleDataGroup(&m_medianPlantHeight);

    deleteHalfOfOneDoubleDataGroup(&m_heaviestPlantMass);
    deleteHalfOfOneDoubleDataGroup(&m_99thPercentilePlantMass);
    deleteHalfOfOneDoubleDataGroup(&m_95thPercentilePlantMass);
    deleteHalfOfOneDoubleDataGroup(&m_90thPercentilePlantMass);
    deleteHalfOfOneDoubleDataGroup(&m_medianPlantMass);

    deleteHalfOfOneDoubleDataGroup(&m_mostPlantEnergy);
    deleteHalfOfOneDoubleDataGroup(&m_99thPercentilePlantEnergy);
    deleteHalfOfOneDoubleDataGroup(&m_95thPercentilePlantEnergy);
    deleteHalfOfOneDoubleDataGroup(&m_90thPercentilePlantEnergy);
    deleteHalfOfOneDoubleDataGroup(&m_medianPlantEnergy);

    deleteHalfOfOneDoubleDataGroup(&m_meanSeedsPerPlant);
    deleteHalfOfOneDoubleDataGroup(&m_meanEnergyPerSeed);
    deleteHalfOfOneDoubleDataGroup(&m_meanDeathAge);
    deleteHalfOfOneDoubleDataGroup(&m_meanStarvationDeathAge);
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
