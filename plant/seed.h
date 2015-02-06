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


#ifndef SEED_H
#define SEED_H

#include "genome.h"

#ifndef Q_MOC_RUN
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/shared_ptr.hpp"
#include "boost/serialization/shared_ptr.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class Seed
{
public:
    Seed() {}
    Seed(double energy, boost::shared_ptr<Genome> genome, long long birthDate, double generation)  :
        m_genome(genome), m_energy(energy), m_generation(generation), m_birthDate(birthDate), m_null(false) {}

    boost::shared_ptr<Genome> m_genome;

    void makeNull() {m_null = true;}
    void setGeneration(double newGeneration) {m_generation = newGeneration;}
    void resetBirthDate() {m_birthDate = 0;}
    long long getAge(long long currentDate) const {return currentDate - m_birthDate;}
    double getGeneration() const {return m_generation;}
    double getEnergy() const {return m_energy;}
    bool isNull() const {return m_null;}
    bool isNotNull() const {return !m_null;}

private:
    double m_energy;
    double m_generation;
    long long m_birthDate;
    bool m_null;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_genome;
        ar & m_energy;
        ar & m_generation;
        ar & m_birthDate;
        ar & m_null;

        ++g_seedsSavedOrLoaded;
    }
};

#endif // SEED_H
