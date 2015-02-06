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


#ifndef SAVERANDLOADER_H
#define SAVERANDLOADER_H

#include <QObject>
#include <QString>

class Environment;
class EnvironmentSettings;
class SimulationSettings;
class Stats;

class SaverAndLoader : public QObject
{
    Q_OBJECT

public:
    SaverAndLoader(QString fullFileName, Environment * environment,
                   EnvironmentSettings * environmentSettings,
                   SimulationSettings * simulationSettings, Stats * stats,
                   bool history = true) :
        m_fullFileName(fullFileName), m_environment(environment),
        m_environmentSettings(environmentSettings),
        m_simulationSettings(simulationSettings), m_stats(stats),
        m_history(history) {}

private:
    QString m_fullFileName;
    Environment * m_environment;
    EnvironmentSettings * m_environmentSettings;
    SimulationSettings * m_simulationSettings;
    Stats * m_stats;
    bool m_history;

public slots:
    void saveSimulation();
    void loadSimulation();

signals:
    void finishedSaving();
    void finishedLoading();
};

#endif // SAVERANDLOADER_H
