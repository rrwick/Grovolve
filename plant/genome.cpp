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


#include "genome.h"
#include <math.h>
#include "../program/randomnumbers.h"
#include "../settings/simulationsettings.h"
#include "../settings/environmentsettings.h"



//This constructor can either make a genome using the starting genome in settings or using
//two parent genomes.
Genome::Genome(bool startingGenome, boost::shared_ptr<Genome> parent1, boost::shared_ptr<Genome> parent2)
{
    m_nucleotides.reserve(g_simulationSettings->genomeLength);

    if (startingGenome)
    {
        int startingGenomeSize = int(g_simulationSettings->startingGenome.m_nucleotides.size());
        for (int i = 0; i < g_simulationSettings->genomeLength; ++i)
        {
            if (i < startingGenomeSize)
                m_nucleotides.push_back(g_simulationSettings->startingGenome.m_nucleotides[i]);
            else
                m_nucleotides.push_back(0);
        }
        return;
    }

    //Add one gene to the genome at a time, switching between the
    //two based on the crossover frequency.
    Genome * sourceGenome = parent1.get();
    Genome * otherGenome = parent2.get();
    if (g_randomNumbers->fiftyPercentChance())
        std::swap(sourceGenome, otherGenome);

    //Instead of calculating a random chance of crossover at each
    //nucleotide, an exponential distribution is used to achieve the
    //same effect more efficiently.
    int crossoverFragmentLength = g_randomNumbers->getCrossoverFragmentLength(g_simulationSettings->averageCrossoverLength);
    for (int i = 0; i < g_simulationSettings->genomeLength; ++i)
    {
        if (crossoverFragmentLength <= 0)
        {
            std::swap(sourceGenome, otherGenome);
            crossoverFragmentLength = g_randomNumbers->getCrossoverFragmentLength(g_simulationSettings->averageCrossoverLength);
        }

        if (i < int(sourceGenome->m_nucleotides.size()))
            m_nucleotides.push_back(sourceGenome->m_nucleotides[i]);
        else
            m_nucleotides.push_back(0);

        --crossoverFragmentLength;
    }

    mutate();
}


//The only type of mutation is point mutation, randomly applied to each nucleotide.
//Instead of calculating a random chance for every nucleotide (would be intensive),
//this code gets a number of mutations and then randomly distributes them around
//the genome.
void Genome::mutate()
{
    int genomeLength = int(m_nucleotides.size());
    int mutationCount = g_randomNumbers->getMutationCount(genomeLength,
                                                          g_environmentSettings->m_currentValues.m_mutationRate);

    std::vector<int> mutatedPositions;
    mutatedPositions.reserve(mutationCount);

    for (int i = 0; i < mutationCount; ++i)
    {
        //Find a unique location for each mutation.
        int mutationPosition;
        do
        {
            mutationPosition = g_randomNumbers->getRandomInt(0, genomeLength - 1);
        } while (std::find(mutatedPositions.begin(), mutatedPositions.end(), mutationPosition) != mutatedPositions.end());

        mutatedPositions.push_back(mutationPosition);
        changeOneNucleotide(mutationPosition);
    }
}

void Genome::changeOneNucleotide(int index)
{
    char originalNucleotide = m_nucleotides[index];
    char newNucleotide;
    do
    {
        newNucleotide = g_randomNumbers->getRandomZeroToThree();
    } while (originalNucleotide == newNucleotide);
    m_nucleotides[index] = newNucleotide;
}


//This function returns a gene index using a given starting point and a two number promoter.
//The gene index returned is after the promoter (does not include the promoter).
//FUTURE IMPROVEMENT: THIS USES A NAIVE STRING MATCHING ALGORITHM.  SOMETHING LIKE BOYER-MOORE COULD DO BETTER!
int Genome::getIndexFromPromoter(int searchingIndex, std::vector<char> * promoter) const
{
    //The starting point for the search is after the current gene ends.
    //The ending point is right before the current gene's start.
    //This range means the whole genome will be searched except for the current gene,
    //so that gene's reference to the promoter won't be found as as a result.
    int sizeOfBranchGene = 14 + g_simulationSettings->promoterLength * g_simulationSettings->maxChildrenPerBranch; //14 is the length of a branch gene, not including promoter references at the end.
    int startingPoint = loopIndex(searchingIndex + sizeOfBranchGene);
    int endingPoint = previousIndex(searchingIndex);

    for (int i = startingPoint; i != endingPoint; i = nextIndex(i))
    {
        bool matchFound = true;
        for (int j = 0; j < g_simulationSettings->promoterLength; ++j)
        {
            if ( (*promoter)[j] != m_nucleotides[loopIndex(i+j)] )
            {
                matchFound = false;
                break;
            }
        }

        if (matchFound)
            return i + g_simulationSettings->promoterLength;
    }

    //Return -1 if no match was found.
    return -1;
}



QString Genome::outputAsString() const
{
    QString output;

    for (std::vector<char>::const_iterator i = m_nucleotides.begin(); i != m_nucleotides.end(); ++i)
    {
        switch (*i)
        {
        case 0:  output += 'A'; break;
        case 1:  output += 'C'; break;
        case 2:  output += 'G'; break;
        case 3:  output += 'T'; break;
        default: output += 'X'; //This should never happen!
        }
    }

    return output;
}



//A group of 4 nucleotides can be read as a base-4 number of length 4.
//This can be either signed (-128 to 127) or unsigned (0 to 255).
int Genome::getUnsignedNumberFrom4Nucleotides(int index) const
{
    return 64 * getNucleotide(index) + 16 * getNucleotide(index+1) + 4 * getNucleotide(index+2) + getNucleotide(index+3);
}
int Genome::getSignedNumberFrom4Nucleotides(int index) const
{
    return getUnsignedNumberFrom4Nucleotides(index) - 128;
}



PlantPartType Genome::getTypeFrom2Nucleotides(int index) const
{
    char nucleotide1 = getNucleotide(index);
    char nucleotide2 = getNucleotide(index + 1);

    if (nucleotide2 >= 2)
        return NO_PART;

    switch (nucleotide1)
    {
    case 0:  return NO_PART;
    case 1:  return LEAF;
    case 2:  return SEEDPOD;
    default: return BRANCH;
    }
}


int Genome::countDifferences(Genome * other) const
{
    int differenceCount = abs(other->getGenomeLength() - int(m_nucleotides.size()));

    int shortestLength = std::min(other->getGenomeLength(), int(m_nucleotides.size()));

    for (int i = 0; i < shortestLength; ++i)
    {
        if (m_nucleotides[i] != other->getNucleotide(i))
            ++differenceCount;
    }

    return differenceCount;
}
