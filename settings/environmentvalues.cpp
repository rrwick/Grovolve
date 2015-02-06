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


#include "environmentvalues.h"
#include <QLocale>
#include "simulationsettings.h"

EnvironmentValues::EnvironmentValues()
{
    m_sunIntensity = 75.0;
    m_mutationRate = 0.0005;
    m_gravity = 1.0;
}


void EnvironmentValues::setValuesToIntermediate(EnvironmentValues start, EnvironmentValues target, double progress)
{
    m_sunIntensity = start.m_sunIntensity + progress * (target.m_sunIntensity - start.m_sunIntensity);
    m_mutationRate = start.m_mutationRate + progress * (target.m_mutationRate - start.m_mutationRate);
    m_gravity = start.m_gravity + progress * (target.m_gravity - start.m_gravity);
}


bool EnvironmentValues::operator==(EnvironmentValues other) const
{
    return (m_sunIntensity == other.m_sunIntensity &&
            m_mutationRate == other.m_mutationRate &&
            m_gravity == other.m_gravity);
}




QString EnvironmentValues::outputChanges(EnvironmentValues other, bool showOnlyFirst) const
{
    QString returnValue;

    outputChangeForOneValue("Sun intensity", m_sunIntensity, other.m_sunIntensity, &returnValue, showOnlyFirst);
    outputChangeForOneValue("Gravity", m_gravity, other.m_gravity, &returnValue, showOnlyFirst);
    outputChangeForOneValue("Mutation rate", m_mutationRate, other.m_mutationRate, &returnValue, showOnlyFirst, true);

    return returnValue;
}


void EnvironmentValues::outputChangeForOneValue(QString valueName, double oldValue, double newValue,
                                                QString * stringToAppend, bool showOnlyFirst,
                                                bool displayAsPercentage) const
{
    if (oldValue == newValue)
        return;

    if (displayAsPercentage)
    {
        oldValue *= 100.0;
        newValue *= 100.0;
    }

    if (stringToAppend->length() > 0)
        (*stringToAppend) += "<br>";

    (*stringToAppend) += valueName + " = ";

    QLocale addCommas(QLocale::English);

    (*stringToAppend) += addCommas.toString(oldValue);
    if (displayAsPercentage)
        (*stringToAppend) += "%";

    if (showOnlyFirst)
        return;

    (*stringToAppend) += " ";
    (*stringToAppend) += QChar(0x2192); //arrow
    (*stringToAppend) += " ";

    (*stringToAppend) += "<span style=\"  font-weight:600; color:";
    if (oldValue > newValue)
        (*stringToAppend) += g_simulationSettings->decreaseValueColor.name();
    else
        (*stringToAppend) += g_simulationSettings->increaseValueColor.name();
    (*stringToAppend) += ";\">";

    (*stringToAppend) += addCommas.toString(newValue);
    if (displayAsPercentage)
        (*stringToAppend) += "%";

    (*stringToAppend) += "</span>";
}
