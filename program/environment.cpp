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


#include "environment.h"
#include <math.h>
#include <algorithm>    // std::sort
#include <vector>
#include <map>
#include "randomnumbers.h"
#include "../plant/plantpart.h"
#include "../plant/genome.h"
#include "../plant/seed.h"
#include "stats.h"
#include "../settings/environmentsettings.h"

Environment::Environment() :
    m_width(g_simulationSettings->startingEnvironmentWidth), m_height(g_simulationSettings->startingEnvironmentHeight),
    m_elapsedTime(0), m_numberOfNullSeeds(0), m_elapsedRealWorldSeconds(0.0)
{
    reset();
}


Environment::~Environment()
{
    cleanUp();
}


void Environment::cleanUp()
{
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        delete *i;
    m_organisms.clear();
    m_seeds.clear();
}


void Environment::reset()
{
    cleanUp();
    g_stats->reset();
    resetTime();
    m_numberOfNullSeeds = 0;
    m_height = g_simulationSettings->startingEnvironmentHeight;
    m_width = g_simulationSettings->startingEnvironmentWidth;
    g_lighting->resetSunIntensity();

    for (int i = 0; i < g_simulationSettings->targetPopulationSize; ++i)
    {
        m_organisms.push_back(new Organism(g_simulationSettings->startingOrganismEnergy,
                                           m_elapsedTime,
                                           g_randomNumbers->getRandomDouble(0.0, m_width)));
        ++(g_stats->m_numberOfOrganismsSprouted);
    }

    logStats();
}


void Environment::resetTime()
{
    m_elapsedTime = 0;
    m_elapsedRealWorldSeconds = 0.0;
    setDateAndTimeOfSimStart();
    m_logIntervalMultiplier = 1;
}


void Environment::advanceOneTick()
{
    for (std::list<Organism *>::iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        (*i)->growOneTick();
        (*i)->transmitLoadAndGrowWidthOneTick();
        (*i)->useEnergyOneTick();
    }

    killOffStarvedAndUnluckyOrganisms();
    getRidOfOldSeeds();

    for (std::list<Organism *>::iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        (*i)->createSeeds(&m_seeds, m_elapsedTime, isDaytime());

    createNewOrganisms();
    ++m_elapsedTime;
    distributeLightToLeaves();

    if (m_elapsedTime % (m_logIntervalMultiplier * g_simulationSettings->statLoggingInterval) == 0)
        logStats();
}



void Environment::distributeLightToLeaves()
{
    std::vector<PlantPart *> leaves;
    addLeavesToVector(&leaves);
    g_lighting->distributeLight(&leaves, this, getSunIntensity(), getSunAngle());
}


void Environment::addLeavesToVector(std::vector<PlantPart *> * leafVector)
{
    for (std::list<Organism *>::iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        (*i)->addLeavesToLightingVector(leafVector);
}




//This function determines how bright the sun is.  If it is night, this
//function returns 0.  If it is day, it returns a value that depends on
//the time of day, the brightest point being midday.
double Environment::getSunIntensity() const
{
    int dayProgression = getDayProgression();

    //If it is day, determine the sun intensity
    if (dayProgression < g_simulationSettings->dayLength)
    {
        double fractionThroughDay = double(dayProgression) / g_simulationSettings->dayLength;
        double fractionOfMaxIntensity = sin(fractionThroughDay * 3.1415926535897932384626433832795);
        return g_environmentSettings->m_currentValues.m_sunIntensity * fractionOfMaxIntensity;
    }

    else
        return 0.0;
}



//If the simulation is during the day, this function returns a sun angle.
//If it is night, it returns 0.0, as the angle isn't used at night anyway.
double Environment::getSunAngle() const
{
    int dayProgression = getDayProgression();

    //If it is day, determine the sun angle.
    if (dayProgression < g_simulationSettings->dayLength)
    {
        double fractionThroughDay = double(dayProgression) / g_simulationSettings->dayLength;
        return fractionThroughDay * (g_simulationSettings->sunsetAngle - g_simulationSettings->sunriseAngle) + g_simulationSettings->sunriseAngle;
    }

    else
        return 0.0;
}




void Environment::killOffStarvedAndUnluckyOrganisms()
{
    for (std::list<Organism *>::iterator i = m_organisms.begin(); i != m_organisms.end();)
    {
        //Kill starved organisms
        if ((*i)->getEnergy() < 0.0)
        {
            long long deathAge = (*i)->getAge(m_elapsedTime);
            delete *i;
            i = m_organisms.erase(i);
            ++(g_stats->m_numberOfOrganismsDiedFromStarvation);
            ++(g_stats->m_numberOfOrganismsDiedFromStarvationSinceLastLog);
            g_stats->m_organismsDiedFromStarvationSinceLastLogAgeSum += deathAge;
        }

        //Kill unlucky organisms
        else if (g_randomNumbers->chanceOfTrue(g_simulationSettings->randomDeathRate))
        {
            //Helped organisms have a lower chance of death.
            if (!(*i)->isHelped() ||
                    g_randomNumbers->chanceOfTrue(g_simulationSettings->helpedDeathRate))
            {
                long long deathAge = (*i)->getAge(m_elapsedTime);
                delete *i;
                i = m_organisms.erase(i);
                ++(g_stats->m_numberOfOrganismsDiedFromBadLuck);
                ++(g_stats->m_numberOfOrganismsDiedFromBadLuckSinceLastLog);
                g_stats->m_organismsDiedFromBadLuckSinceLastLogAgeSum += deathAge;
            }
        }
        else
            ++i;
    }
}


void Environment::getRidOfOldSeeds()
{
    //Since Seeds will be naturally sorted by age (as they all always pushed
    //onto the end), delete old ones by popping them off the front.
    //Also clean up null pointers are the front which are left when seeds combine
    //to form an organism.
    while (m_seeds.size() > 0 &&
           (m_seeds.front().isNull() ||
            m_seeds.front().getAge(m_elapsedTime) > g_simulationSettings->maxSeedAge))
    {
        if (m_seeds.front().isNull())
            --m_numberOfNullSeeds;
        m_seeds.pop_front();
    }
}



void Environment::createNewOrganisms()
{
    int newOrganismCount = g_randomNumbers->changeDoubleToProbabilisticInt(g_simulationSettings->newOrganismsPerTickPerSeed * m_seeds.size());

    for (int i = 0; i < newOrganismCount; ++i)
    {
        //If there are less than 2, then nothing can be done, so quit the function.
        if (getSeedCount() < 2)
            return;

        //Choose two random seeds.
        int seedIndex1, seedIndex2;
        do
        {
            seedIndex1 = g_randomNumbers->getRandomInt(0, int(m_seeds.size()) - 1);
        } while(m_seeds[seedIndex1].isNull());
        do
        {
            seedIndex2 = g_randomNumbers->getRandomInt(0, int(m_seeds.size()) - 1);
        } while (m_seeds[seedIndex2].isNull() || seedIndex1 == seedIndex2);

        //Create an organism from the two Seeds.
        m_organisms.push_back(new Organism(m_seeds[seedIndex1], m_seeds[seedIndex2],
                                           m_elapsedTime,
                                           g_randomNumbers->getRandomDouble(0.0, m_width)));
        ++(g_stats->m_numberOfOrganismsSprouted);

        //Delete the two Seeds.  They are just labelled as null as actually removing
        //them from the middle of the deque is a costly procedure.
        m_seeds[seedIndex1].makeNull();
        m_seeds[seedIndex2].makeNull();

        m_numberOfNullSeeds += 2;
    }
}




void Environment::logStats()
{
    if (int(g_stats->m_time.size()) >= g_simulationSettings->maxLogEntries &&
            g_stats->m_time.size() % 2 == 0)
    {
        g_stats->deleteHalfOfAllData();
        m_logIntervalMultiplier *= 2;
    }
    g_stats->addToLog(this);
}




double Environment::getAverageGeneration() const
{
    if (m_organisms.size() == 0)
        return 0.0;

    double generationSum = 0.0;

    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        generationSum += (*i)->getGeneration();

    return generationSum / m_organisms.size();
}



double Environment::getTallestPlantHeight() const
{
    double tallestPlantHeight = 0.0;

    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        double plantHeight = (*i)->getHeight();
        if (plantHeight > tallestPlantHeight)
            tallestPlantHeight = plantHeight;
    }

    return tallestPlantHeight;
}




int Environment::getFullyGrownPlantCount() const
{
    int fullyGrownPlantCount = 0;

    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        if ((*i)->isFinishedGrowing())
            ++fullyGrownPlantCount;
    }

    return fullyGrownPlantCount;
}



double Environment::getFullyGrownPlantFraction() const
{
    if (getOrganismCount() == 0)
        return 0.0;

    return double(getFullyGrownPlantCount()) / getOrganismCount();
}



////Returns 0.0 if there are no fully grown plants.
//double Environment::getAverageHeightOfFullyGrownPlants() const
//{
//    double heightSum = 0.0;

//    std::vector<const Organism *> grownOrganisms = getGrownOrganisms();
//    if (grownOrganisms.size() == 0)
//        return 0.0;

//    for (std::vector<const Organism *>::const_iterator i = grownOrganisms.begin(); i != grownOrganisms.end(); ++i)
//        heightSum += (*i)->getHeight();

//    return heightSum / grownOrganisms.size();
//}


void Environment::getPlantHeightPercentiles(double * tallestPlantHeight, double * ninetyNinthPercentilePlantHeight,
                                            double * ninetiethPercentilePlantHeight, double * medianPlantHeight) const
{
    std::vector<double> heights;
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        heights.push_back((*i)->getHeight());
    getPercentilesOfDoubleVector(&heights, tallestPlantHeight, ninetyNinthPercentilePlantHeight,
                                 ninetiethPercentilePlantHeight, medianPlantHeight);
}

void Environment::getPlantMassPercentiles(double * heaviestPlantMass, double * ninetyNinthPercentilePlantMass,
                                          double * ninetiethPercentilePlantMass, double * medianPlantMass) const
{
    std::vector<double> masses;
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        masses.push_back((*i)->getMass());
    getPercentilesOfDoubleVector(&masses, heaviestPlantMass, ninetyNinthPercentilePlantMass,
                                 ninetiethPercentilePlantMass, medianPlantMass);
}

void Environment::getPlantEnergyPercentiles(double * mostPlantEnergy, double * ninetyNinthPercentilePlantEnergy,
                                          double * ninetiethPercentilePlantEnergy, double * medianPlantEnergy) const
{
    std::vector<double> energies;
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        energies.push_back((*i)->getEnergy());
    getPercentilesOfDoubleVector(&energies, mostPlantEnergy, ninetyNinthPercentilePlantEnergy,
                                 ninetiethPercentilePlantEnergy, medianPlantEnergy);
}

void Environment::getPercentilesOfDoubleVector(std::vector<double> * doubleVector,
                                               double * max, double * ninetyNinthPercentile,
                                               double * ninetiethPercentile, double * median) const
{
    *max = 100.0; //TEMP
    *ninetyNinthPercentile = 80.0; //TEMP
    *ninetiethPercentile = 60.0; //TEMP
    *median = 40.0; //TEMP
}





bool Environment::possiblyChangeEnvironmentSize()
{
    bool sizeChanged = false;

    if (m_elapsedTime % g_simulationSettings->environmentHeightCheckInterval == 0)
    {
        double heightOfTallestPlant = getTallestPlantHeight();
        double topGap = m_height - heightOfTallestPlant;

        if (topGap < g_simulationSettings->environmentHeightGap)
        {
            sizeChanged = true;
            m_height = heightOfTallestPlant + g_simulationSettings->environmentHeightGap;
        }
        else if (topGap > g_simulationSettings->startingEnvironmentHeight)
        {
            sizeChanged = true;
            m_height = heightOfTallestPlant + g_simulationSettings->startingEnvironmentHeight;
        }
    }

    if (m_elapsedTime % g_simulationSettings->environmentWidthCheckInterval == 0)
    {
        double population = double(m_organisms.size());
        double targetPopulation = double(g_simulationSettings->targetPopulationSize);

        //Allow populations of +/- 20% of the taget size.
        double minPopulation = targetPopulation * 0.8;
        double maxPopulation = targetPopulation * 1.2;

        if (population > maxPopulation || population < minPopulation)
        {
            double widthChangeRatio = targetPopulation / population;
            int newWidth = double(m_width) * widthChangeRatio + 0.5;

            if (newWidth < g_simulationSettings->minimumEnvironmentWidth)
                newWidth = g_simulationSettings->minimumEnvironmentWidth;

            setWidth(newWidth);
            sizeChanged = true;
        }
    }

    return sizeChanged;
}




Organism * Environment::findOrganismUnderPoint(Point2D point) const
{
    //Loop through organisms backwards.  This is to make sure that organisms
    //that are drawn on top of others (i.e. later) are found first when clicked
    //on.
    for (std::list<Organism *>::const_reverse_iterator i = m_organisms.rbegin(); i != m_organisms.rend(); ++i)
    {
        if ((*i)->isPointInsideOrganism(point))
            return *i;
    }

    return 0;
}





double Environment::getAverageEnergyPerSeed() const
{
    int seedCount = 0;
    double totalSeedEnergy = 0.0;

    for (std::deque<Seed>::const_iterator i = m_seeds.begin(); i != m_seeds.end(); ++i)
    {
        if (i->isNotNull())
        {
            ++seedCount;
            totalSeedEnergy += i->getEnergy();
        }
    }

    if (seedCount == 0)
        return 0.0;

    return totalSeedEnergy / m_seeds.size();
}


//This function calculates the mean number of plant parts (of the given type)
//for the fraction of the population that has reached the non-starved age.
double Environment::getMeanPartsPerPlant(PlantPartType partType) const
{
    int totalParts = 0;

    std::vector<const Organism *> grownOrganisms = getGrownOrganisms();
    if (grownOrganisms.size() == 0)
        return 0.0;

    for (std::vector<const Organism *>::const_iterator i = grownOrganisms.begin(); i != grownOrganisms.end(); ++i)
    {
        switch (partType)
        {
        case BRANCH:
            totalParts += (*i)->getBranchCount();
            break;
        case LEAF:
            totalParts += (*i)->getLeafCount();
            break;
        case SEEDPOD:
            totalParts += (*i)->getSeedpodCount();
            break;
        case ANY_PART:
            totalParts += (*i)->getPlantPartCount();
            break;
        case NO_PART:
            break;
        }
    }

    return double(totalParts) / grownOrganisms.size();
}



//This function creates and returns a genome for which the following are true:
// -Its length is equal to the most common genome length in the current population.
// -The gene at each position is equal to the most common gene at that position in the current population.
//It therefore serves to capture the 'standard' genome for the current population, even though
//it is quite possible (likely?) that the genome it returns is not exactly represented in any organism.
Genome Environment::getModeGenome() const
{
    std::vector<Genome *> allGenomes;
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
        allGenomes.push_back((*i)->getGenome());

    std::vector<int> genomeLengths;
    for (std::vector<Genome *>::iterator i = allGenomes.begin(); i != allGenomes.end(); ++i)
        genomeLengths.push_back(int((*i)->getGenomeLength()));

    int modeGenomeLength = getMode(&genomeLengths);
    Genome modeGenome;

    std::vector<int> genesAtOnePosition;
    for (int i = 0; i < modeGenomeLength; ++i)
    {
        genesAtOnePosition.clear();
        for (std::vector<Genome *>::iterator j = allGenomes.begin(); j != allGenomes.end(); ++j)
        {
            if (i < (*j)->getGenomeLength())
                genesAtOnePosition.push_back((*j)->getNucleotide(i));
        }
        modeGenome.addNucleotide(getMode(&genesAtOnePosition));
    }

    return modeGenome;
}







//This function assumes that numbers is a vector that contains only the values
//0 to 3.  If any other values are present, this function will crash.
int Environment::getMode(std::vector<int> * numbers) const
{
    std::map<int, int> frequencies;

    for (size_t i = 0; i < numbers->size(); ++i)
    {
        std::map<int, int>::iterator it = frequencies.find((*numbers)[i]);
        if (it == frequencies.end())
            frequencies[(*numbers)[i]] = 1;
        else
            ++(it->second);
    }

    int mostCommonValue = 0;
    int highestFrequency = 0;
    for(std::map<int, int>::iterator it = frequencies.begin();
        it != frequencies.end(); ++it)
    {
        if (it->second > highestFrequency)
        {
            mostCommonValue = it->first;
            highestFrequency = it->second;
        }
    }

    return mostCommonValue;
}




const Organism * Environment::getOldestOrganism() const
{
    const Organism * oldestOrganism = 0;
    long long oldestAge = -1;
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        long long organismAge = (*i)->getAge(m_elapsedTime);
        if (organismAge > oldestAge)
        {
            oldestOrganism = *i;
            oldestAge = organismAge;
        }
    }

    return oldestOrganism;
}




int Environment::getMaxGenomeLength() const
{
    int maxGenomeLength = 0;

    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        int genomeLength = (*i)->getGenome()->getGenomeLength();
        if (genomeLength > maxGenomeLength)
            maxGenomeLength = genomeLength;
    }

    return maxGenomeLength;
}


//This function randomly selects an organism from the current population.
//It tries to choose from organisms that have stopped growing.  If there aren't any,
//it choses ones that are old (above the average non-starved age).  If there aren't
//anyof those either, it just chooses any organism at random.
const Organism * Environment::getRandomGrownOrganism() const
{
    if (m_organisms.size() == 0)
        return 0;

    //First try to find a random fully-grown organism.
    std::vector<const Organism *> grownOrganisms = getGrownOrganisms();
    if (grownOrganisms.size() > 0)
    {
        int randomSelection = g_randomNumbers->getRandomInt(0, int(grownOrganisms.size()) - 1);
        return grownOrganisms[randomSelection];
    }

    //If that failed, try to find an organism that is old.
    std::vector<const Organism *> oldOrganisms = getOldOrganisms();
    if (oldOrganisms.size() > 0)
    {
        int randomSelection = g_randomNumbers->getRandomInt(0, int(oldOrganisms.size()) - 1);
        return oldOrganisms[randomSelection];
    }

    //If that failed too, just select any random one from the whole population.
    else
    {
        int randomSelection = g_randomNumbers->getRandomInt(0, int(m_organisms.size()) - 1);
        std::list<Organism *>::const_iterator i = m_organisms.begin();
        for (int count = 0; count < randomSelection; ++count)
            ++i;
        return (*i);
    }
}



void Environment::setWidth(int newWidth)
{
    //If the width is being reduced, kill off any organism that's now out of the
    //environment bounds
    if (newWidth < m_width)
    {
        for (std::list<Organism *>::iterator i = m_organisms.begin(); i != m_organisms.end();)
        {
            if ((*i)->getSeedX() > newWidth)
            {
                delete *i;
                i = m_organisms.erase(i);
                ++(g_stats->m_numberOfOrganismsDiedFromBadLuck);
            }
            else
                ++i;
        }
    }

    m_width = newWidth;
}



std::vector<const Organism *> Environment::getGrownOrganisms() const
{
    std::vector<const Organism *> grownOrganisms;
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        if ((*i)->isFinishedGrowing() && (*i)->getMass() > 1.0) //Mass requirement is to exclude organisms that grew into the ground (i.e. didn't grow at all).
            grownOrganisms.push_back(*i);
    }
    return grownOrganisms;
}

std::vector<const Organism *> Environment::getOldOrganisms() const
{
    std::vector<const Organism *> oldOrganisms;
    double ageCutoff = g_simulationSettings->getAverageNonStarvedAge();
    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        if ((*i)->getAge(m_elapsedTime) >= ageCutoff && (*i)->getMass() > 1.0) //Mass requirement is to exclude organisms that grew into the ground (i.e. didn't grow at all).
            oldOrganisms.push_back(*i);
    }
    return oldOrganisms;
}


void Environment::resetAllGenerations()
{
    for (std::list<Organism *>::iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        (*i)->setGeneration(1.0);
        (*i)->resetBirthDate();
    }
    for (std::deque<Seed>::iterator i = m_seeds.begin(); i != m_seeds.end(); ++i)
    {
        i->setGeneration(1.0);
        i->resetBirthDate();
    }
}



void Environment::killOrganism(Organism * organism)
{
    delete organism;
    m_organisms.erase(std::remove(m_organisms.begin(), m_organisms.end(), organism), m_organisms.end());
}

void Environment::helpOrganism(Organism * organism)
{
    organism->help();
}


QString Environment::outputAllInfoOnCurrentPopulation() const
{
    QString output;

    output += "Age,Energy,Height,Mass,Generation,Branches,Leaves,Seedpods,"
              "Energy gained from photosynthesis,Energy spent on growth and maintenance,"
              "Energy spent on reproduction,Genome\n";

    for (std::list<Organism *>::const_iterator i = m_organisms.begin(); i != m_organisms.end(); ++i)
    {
        output += QString::number((*i)->getAge(m_elapsedTime)) + ",";
        output += QString::number((*i)->getEnergy()) + ",";
        output += QString::number((*i)->getHeight()) + ",";
        output += QString::number((*i)->getMass()) + ",";
        output += QString::number((*i)->getGeneration()) + ",";
        output += QString::number((*i)->getBranchCount()) + ",";
        output += QString::number((*i)->getLeafCount()) + ",";
        output += QString::number((*i)->getSeedpodCount()) + ",";
        output += QString::number((*i)->getEnergyFromPhotosynthesis()) + ",";
        output += QString::number((*i)->getEnergySpentOnGrowthAndMaintenance()) + ",";
        output += QString::number((*i)->getEnergySpentOnReproduction()) + ",";
        output += (*i)->getGenome()->outputAsString();
        output += "\n";
    }

    return output;
}
