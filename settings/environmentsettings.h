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


#ifndef ENVIRONMENTSETTINGS_H
#define ENVIRONMENTSETTINGS_H

#include "environmentvalues.h"

#ifndef Q_MOC_RUN
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class EnvironmentSettings
{
public:
    EnvironmentSettings();

    EnvironmentValues m_currentValues;

    void updateCurrentValues(long long currentTime);
    void createProgression(long long startingTime, EnvironmentValues startingValues,
                           long long targetTime, EnvironmentValues targetValues);
    void setStaticValues(EnvironmentValues staticValues);
    bool isProgressionActive() const {return m_progressionActive;}
    long long getStartingTime() const {return m_startingTime;}
    long long getTargetTime() const {return m_targetTime;}
    EnvironmentValues getStartingValues() const {return m_startingValues;}
    EnvironmentValues getTargetValues() const {return m_targetValues;}

private:
    long long m_startingTime;
    EnvironmentValues m_startingValues;
    long long m_targetTime;
    EnvironmentValues m_targetValues;
    bool m_progressionActive;

    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & m_currentValues;
        ar & m_startingTime;
        ar & m_startingValues;
        ar & m_targetTime;
        ar & m_targetValues;
        ar & m_progressionActive;
    }
};

#endif // ENVIRONMENTSETTINGS_H
