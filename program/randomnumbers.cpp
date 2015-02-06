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


#include "randomnumbers.h"
#include <QTime>

RandomNumbers::RandomNumbers()
{
    QTime midnight(0, 0, 0);
    int seed = midnight.msecsTo(QTime::currentTime());
    m_random.seed(seed);

    m_randomZeroToOne = new boost::random::uniform_01<>();
    m_randomZeroToThree = new boost::random::uniform_smallint<>(0, 3);
    m_randomZeroOrOne = new boost::random::uniform_smallint<>(0, 1);
}

RandomNumbers::~RandomNumbers()
{
    delete m_randomZeroToOne;
    delete m_randomZeroToThree;
    delete m_randomZeroOrOne;
}



double RandomNumbers::getRandomDouble(double min, double max)
{
    boost::random::uniform_real_distribution<> randomDoubleDistribution(min, max);
    return randomDoubleDistribution(m_random);
}

double RandomNumbers::getRandomExponential(double lambda)
{
    boost::random::exponential_distribution<> randomDoubleDistribution(lambda);
    return randomDoubleDistribution(m_random);
}


int RandomNumbers::getRandomInt(int min, int max)
{
    boost::random::uniform_int_distribution<> randomIntDistribution(min, max);
    return randomIntDistribution(m_random);
}


//This function returns an int from a double.
//The integer componenet of is always included.  Any fractional component represents the
//probability for another.
//E.g. 2.2 will have a 20% chance of returning 3 and an 80% chance of returning 2.
int RandomNumbers::changeDoubleToProbabilisticInt(double input)
{
    int count = int(input);

    double chanceOfAnother = input - count;
    if (chanceOfTrue(chanceOfAnother))
        ++count;

    return count;
}



//This function gives a mutation count given genome size and mutation rate.
//It is assumed that each nucleotide has an independent chance of mutating.
//While a binomial distribution is the exact distribution for this scenario,
//the Poisson approximation of binomial should be very close and is much
//faster to calculate.
int RandomNumbers::getMutationCount(int genomeSize, double mutationChance)
{
    boost::random::poisson_distribution<> binomialApprox(genomeSize * mutationChance);
    return binomialApprox(m_random);
}




//This function gives a crossover fragment length.  It is assumed that the
//chance of crossover is equal at each nucleotide.
int RandomNumbers::getCrossoverFragmentLength(double meanFragmentLength)
{
    boost::random::exponential_distribution<> fragLength(1.0 / meanFragmentLength);
    return fragLength(m_random);
}
