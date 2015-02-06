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


#ifndef RANDOMNUMBERS_H
#define RANDOMNUMBERS_H

#ifndef Q_MOC_RUN
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"
#include "boost/random/uniform_real_distribution.hpp"
#include "boost/random/uniform_smallint.hpp"
#include "boost/random/uniform_01.hpp"
#include "boost/random/exponential_distribution.hpp"
#include "boost/random/poisson_distribution.hpp"
#include "boost/random/exponential_distribution.hpp"
#endif // Q_MOC_RUN

class RandomNumbers
{
public:
    RandomNumbers();
    ~RandomNumbers();

    double getRandomDouble(double min, double max);
    int getRandomInt(int min, int max);
    double getRandomZeroToOne() {return (*m_randomZeroToOne)(m_random);}
    int getRandomZeroOrOne() {return (*m_randomZeroOrOne)(m_random);}
    char getRandomZeroToThree() {return (*m_randomZeroToThree)(m_random);}
    bool chanceOfTrue(double chance) {return getRandomZeroToOne() < chance;}
    int changeDoubleToProbabilisticInt(double input);
    double getRandomExponential(double lambda);
    bool fiftyPercentChance() {return getRandomZeroOrOne() == 0;}
    int getMutationCount(int nucleotides, double mutationChance);
    int getCrossoverFragmentLength(double meanFragmentLength);

private:
    boost::random::mt19937 m_random;
    boost::random::uniform_01<> * m_randomZeroToOne;
    boost::random::uniform_smallint<> * m_randomZeroToThree;
    boost::random::uniform_smallint<> * m_randomZeroOrOne;
};

#endif // RANDOMNUMBERS_H
