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


#ifndef ENVIRONMENTVALUES_H
#define ENVIRONMENTVALUES_H

#include <QString>
#include "../program/globals.h"

#ifndef Q_MOC_RUN
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class EnvironmentValues
{
public:
    EnvironmentValues();

    double m_sunIntensity;
    double m_mutationRate;
    double m_gravity;

    void setValuesToIntermediate(EnvironmentValues start, EnvironmentValues target, double progress);
    QString outputChanges(EnvironmentValues other, bool showOnlyFirst = false) const;
    bool operator==(EnvironmentValues other) const;

private:
    void outputChangeForOneValue(QString valueName, double oldValue, double newValue,
                                 QString * stringToAppend, bool showOnlyFirst, bool displayAsPercentage = false) const;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_sunIntensity;
        ar & m_mutationRate;
        ar & m_gravity;
    }
};

#endif // ENVIRONMENTVALUES_H
