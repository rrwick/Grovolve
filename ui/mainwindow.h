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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <QCloseEvent>
#include <QDateTime>
#include <QPoint>
#include "../program/globals.h"
#include "../settings/simulationsettings.h"
#include "../lighting/lighting.h"

class EnvironmentWidget;
class Environment;
class EnvironmentValues;
class Organism;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget * parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow * ui;
    Environment * m_environment;
    EnvironmentWidget * m_environmentWidget;
    QTimer m_timer;
    int m_lastDisplayedEnvironmentInfoTab;
    int m_lastDisplayedEnvironmentInfoGraph;
    int m_lastDisplayedEnvironmentInfoHistoryType;
    bool m_lastDisplayedLogScale;

    bool m_resumeSimulationAfterSave;
    long long m_saveImageToFileNextTime;
    long long m_saveImageToFileInterval;
    bool m_justSaved;
    QString m_autosavePath;

    void saveImageToFileAutomatic();
    void saveImageToFile2(QString saveFileName, bool highQuality, bool shadows);
    double getVisibleCentreX();
    double getVisibleBottomY();
    void startSimulation() {startStopSimulation(true);}
    void stopSimulation() {startStopSimulation(false);}
    bool simulationIsRunning() {return m_timer.isActive();}
    bool checkForExtinction();
    QString getExecutableName();
    void resetIfTimeIsZeroAndStartingSettingsChanged(SimulationSettings settingsBefore, SimulationSettings settingsAfter);
    void updateTimeDisplay();
    QString getPaddedTimeString();
    void setEnvironmentSize();
    void loadSimulation(QString fullFileName);
    void loadSettings(QString fullFileName);

protected:
    void closeEvent(QCloseEvent * event);

public slots:
    void changeZoomLevel(double newZoomLevel);
    void openOrganismInfoDialog(const Organism * organism);
    void finishedSaving();
    void finishedLoading();

private slots:
    void startStopSimulationButtonPressed();
    void startStopSimulation(bool start);
    void advanceOneTick();
    void resetSimulation();
    void openSettingsDialog();
    void openEnvironmentDialog();
    void openQuickSummaryDialog();
    void openAboutDialog();
    void openStatsAndHistoryDialog();
    void openAutoSaveImagesDialog();
    void openRecoverFilesDialog();
    void switchBasicMode();
    void switchAdvancedMode();
    void switchBasicAdvancedMode();
    void toggleShadows();
    void turnDisplayOn();
    void turnDisplayOff();
    void toggleDisplay(bool displayOn);
    void simulationSpeedChanged();
    void setClickMode();
    void switchClickMode(int newClickMode);
    void killOrganism(Organism * organism);
    void helpOrganism(Organism * organism);
    void saveImageToFileManual();
    void saveSimulationManual();
    void saveSimulationAutomatic();
    void saveSimulation(QString fullFileName, bool history, QString message);
    void saveSettings();
    void loadSimulationPrompt();
    void loadSettingsPrompt();
    void mouseDrag(QPoint change);
    void scrollAreaChanged();
};

#endif // MAINWINDOW_H
