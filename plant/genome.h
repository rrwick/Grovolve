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


#ifndef GENOME_H
#define GENOME_H

#include <vector>
#include <QString>
#include "../program/globals.h"

#ifndef Q_MOC_RUN
#include "boost/serialization/vector.hpp"
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/shared_ptr.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class Genome
{
public:
    Genome() {}
    Genome(bool startingGenome, boost::shared_ptr<Genome> parent1, boost::shared_ptr<Genome> parent2);

    void mutate();
    void addNucleotide(char newNucleotide) {m_nucleotides.push_back(newNucleotide);}
    int getIndexFromPromoter(int startingPoint, std::vector<char> * promoter) const;
    int getGenomeLength() const {return int(m_nucleotides.size());}
    QString outputAsString() const;
    char getNucleotide(int index) const {return m_nucleotides[loopIndex(index)];}
    int getUnsignedNumberFrom4Nucleotides(int index) const;
    int getSignedNumberFrom4Nucleotides(int index) const;
    PlantPartType getTypeFrom2Nucleotides(int index) const;
    int countDifferences(Genome * other) const;
    int nextIndex(int index) const {return loopIndex(index + 1);}
    int previousIndex(int index) const {return loopIndex(index - 1);}
    bool operator==(Genome other) const {return m_nucleotides == other.m_nucleotides;}
    bool operator!=(Genome other) const {return !(*this == other);}

private:
    std::vector<char> m_nucleotides;

    //Since the genome is supposed to be effectively circular, this function is used to keep any
    //integer index in the genome's range.
    //http://stackoverflow.com/questions/12276675/modulus-with-negative-numbers-in-c
    int loopIndex(int index) const {int size = int(m_nucleotides.size()); return (index % size + size) % size;}

    void changeOneNucleotide(int index);

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_nucleotides;
    }
};

#endif // GENOME_H
