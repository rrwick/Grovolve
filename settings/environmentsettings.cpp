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


#include "environmentsettings.h"

EnvironmentSettings::EnvironmentSettings() :
    m_startingTime(0), m_targetTime(1), m_progressionActive(false)
{
}




void EnvironmentSettings::updateCurrentValues(long long currentTime)
{
    long long progressionLength = m_targetTime - m_startingTime;
    long long elapsedProgressionTime = currentTime - m_startingTime;

    double progress = double(elapsedProgressionTime) / progressionLength;
    if (progress > 1.0)
    {
        progress = 1.0;
        m_progressionActive = false;
    }
    if (progress < 0.0)
        progress = 0.0;

    m_currentValues.setValuesToIntermediate(m_startingValues, m_targetValues, progress);
}


void EnvironmentSettings::createProgression(long long startingTime, EnvironmentValues startingValues,
                                            long long targetTime, EnvironmentValues targetValues)
{
    m_startingTime = startingTime;
    m_startingValues = startingValues;
    m_targetTime = targetTime;
    m_targetValues = targetValues;
    m_progressionActive = true;
}


void EnvironmentSettings::setStaticValues(EnvironmentValues staticValues)
{
    m_currentValues = staticValues;
    m_progressionActive = false;
}
