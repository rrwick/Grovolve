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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <vector>
#include <math.h>
#include <QPolygonF>
#include <QPointF>
#include <QTime>
#include <QString>
#include <QScrollBar>
#include <QFileDialog>
#include <QLocale>
#include <QFileInfo>
#include <QProgressDialog>
#include <QDir>
#include <QCoreApplication>
#include <QMessageBox>
#include <QFont>
#include <QThread>
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/uniform_int_distribution.hpp"
#include "boost/random/uniform_real_distribution.hpp"
#include "../settings/simulationsettings.h"
#include "../settings/environmentsettings.h"
#include "../program/randomnumbers.h"
#include "../plant/organism.h"
#include "../plant/plantpart.h"
#include "settingsdialog.h"
#include "environmentdialog.h"
#include "environmentwidget.h"
#include "../program/environment.h"
#include "../lighting/lighting.h"
#include "../program/stats.h"
#include "organisminfodialog.h"
#include "quicksummarydialog.h"
#include "aboutdialog.h"
#include "statsandhistorydialog.h"
#include "autosaveimagesdialog.h"
#include "waitingdialog.h"
#include "recoverautosavefilesdialog.h"
#include "../program/saverandloader.h"
#include <fstream>
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "tbb/task_scheduler_init.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_lastDisplayedEnvironmentInfoTab(0), m_lastDisplayedEnvironmentInfoGraph(0),
    m_lastDisplayedEnvironmentInfoHistoryType(0), m_resumeSimulationAfterSave(false),
    m_justSaved(false),
    m_autosavePath(QDir::temp().path() + "/Grovolve-" + QString::number(QCoreApplication::applicationPid()) + ".grov")
{
    tbb::task_scheduler_init init(tbb::task_scheduler_init::automatic);

    ui->setupUi(this);

    QApplication::setWindowIcon(QIcon(QPixmap(":/icons/tree-512.png")));

    ui->actionBasic->setChecked(true);

    //Create the globals
    g_simulationSettings = new SimulationSettings();
    g_environmentSettings = new EnvironmentSettings();
    g_randomNumbers = new RandomNumbers();
    g_lighting = new Lighting();
    g_stats = new Stats();

    //Save the large font in a variable to be applied later to various UI elements.
    //This isn't done in the UI editor, as different platforms seem to have very different
    //base font sizes.
    QFont font;
    g_largeFont.setPointSize(font.pointSize() * 11 / 8);
    g_extraLargeFont.setPointSize(font.pointSize() * 7 / 4);

    ui->simulationHiddenLabel->setFont(g_extraLargeFont);
    ui->simulationHiddenLabel->setVisible(false);

    m_environment = new Environment();
    m_environmentWidget = new EnvironmentWidget(this, m_environment);
    ui->scrollArea->setWidget(m_environmentWidget);

    ui->shadowsInfoText->setInfoText("Showing shadows allows you to see the flow of light in the simulation. "
                                     "Dark areas receive less light and bright areas more light. "
                                     "Leaves in shadows will absorb less light and therefore produce less energy.");
    ui->clickModeInfoText->setInfoText("You can interact with the simulation by either killing "
                                       "plants or helping them.  When helped, plants live longer and are more "
                                       "likely to produce offspring.");

    connect(ui->actionBasic, SIGNAL(triggered(bool)), this, SLOT(switchBasicMode()));
    connect(ui->actionAdvanced, SIGNAL(triggered(bool)), this, SLOT(switchAdvancedMode()));
    connect(ui->shadowsButton, SIGNAL(clicked()), this, SLOT(toggleShadows()));
    connect(ui->clickModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(switchClickMode(int)));
    connect(ui->actionSettings, SIGNAL(triggered()), this, SLOT(openSettingsDialog()));
    connect(ui->actionQuick_summary, SIGNAL(triggered()), this, SLOT(openQuickSummaryDialog()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
    connect(ui->environmentButton, SIGNAL(clicked()), this, SLOT(openEnvironmentDialog()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(advanceOneTick()));
    connect(ui->startStopSimulationButton, SIGNAL(clicked()), this, SLOT(startStopSimulationButtonPressed()));
    connect(ui->resetButton, SIGNAL(clicked()), this, SLOT(resetSimulation()));
    connect(ui->zoomSpinBox, SIGNAL(valueChanged(double)), this, SLOT(changeZoomLevel(double)));
    connect(m_environmentWidget, SIGNAL(changeZoomLevel(double)), this, SLOT(changeZoomLevel(double)));
    connect(ui->actionSave_current_image, SIGNAL(triggered()), this, SLOT(saveImageToFileManual()));
    connect(ui->actionSave_settings, SIGNAL(triggered()), this, SLOT(saveSettings()));
    connect(ui->actionLoad_settings, SIGNAL(triggered()), this, SLOT(loadSettingsPrompt()));
    connect(ui->actionSave_simulation, SIGNAL(triggered()), this, SLOT(saveSimulationManual()));
    connect(ui->actionLoad_simulation, SIGNAL(triggered()), this, SLOT(loadSimulationPrompt()));
    connect(m_environmentWidget, SIGNAL(showOrganismInfoDialog(const Organism*)), this, SLOT(openOrganismInfoDialog(const Organism*)));
    connect(m_environmentWidget, SIGNAL(killOrganism(Organism*)), this, SLOT(killOrganism(Organism*)));
    connect(m_environmentWidget, SIGNAL(helpOrganism(Organism*)), this, SLOT(helpOrganism(Organism*)));
    connect(ui->informationButton, SIGNAL(clicked()), this, SLOT(openStatsAndHistoryDialog()));
    connect(ui->actionAutomatically_save_images, SIGNAL(triggered()), this, SLOT(openAutoSaveImagesDialog()));
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui->actionRecover_autosave_files, SIGNAL(triggered()), this, SLOT(openRecoverFilesDialog()));
    connect(m_environmentWidget, SIGNAL(mouseDrag(QPoint)), this, SLOT(mouseDrag(QPoint)));
    connect(ui->speedSlider, SIGNAL(valueChanged(int)), this, SLOT(simulationSpeedChanged()));


    //The spin box for setting the zoom should have the same step size as using
    //the mouse wheel to set the zoom.
    ui->zoomSpinBox->setValue(g_simulationSettings->zoom * 100.0);
    ui->zoomSpinBox->setSingleStep(g_simulationSettings->zoomStep * 100.0);  //Multiply by 100 because the spin box uses percentage.
    ui->zoomSpinBox->setMinimum(g_simulationSettings->minZoom * 100.0);  //Multiply by 100 because the spin box uses percentage.
    ui->zoomSpinBox->setMaximum(g_simulationSettings->maxZoom * 100.0);  //Multiply by 100 because the spin box uses percentage.

    //The 'Show/hide shadows' button can change widths when its text changes, so we'll lock its
    //size here.
    ui->shadowsButton->setFixedSize(ui->shadowsButton->sizeHint());

    switchBasicAdvancedMode();
    ui->startStopSimulationButton->setFocus();
}

MainWindow::~MainWindow()
{
    delete g_stats;
    delete g_lighting;
    delete m_environment;
    delete g_randomNumbers;
    delete g_simulationSettings;
    delete ui;
}



void MainWindow::toggleShadows()
{
    g_simulationSettings->shadowsDrawn = !(g_simulationSettings->shadowsDrawn);

    if (g_simulationSettings->shadowsDrawn)
        ui->shadowsButton->setText("Hide shadows");
    else
        ui->shadowsButton->setText("Show shadows");

    m_environmentWidget->update();
}



void MainWindow::simulationSpeedChanged()
{
    if (simulationIsRunning())
    {
        //The simulation is hidden for maximum speed.
        //All slower speeds show the simulation in progress.
        if (ui->speedSlider->value() == 2)
            turnDisplayOff();
        else
            turnDisplayOn();
    }

    switch (ui->speedSlider->value())
    {
    case 0:
        m_timer.setInterval(100);
        break;

    case 1:
    case 2:
        m_timer.setInterval(0);
        break;
    }
}


void MainWindow::turnDisplayOn()
{
    toggleDisplay(true);
}
void MainWindow::turnDisplayOff()
{
    toggleDisplay(false);
}

//This function turns the simulation display on and off.  It takes care of the related
//things like resizing environmentWidget and remembering/recalling the scroll bar positions.
void MainWindow::toggleDisplay(bool displayOn)
{
    g_simulationSettings->displayOn = displayOn;

    if (g_simulationSettings->displayOn) //Display just turned on.
    {
        ui->simulationHiddenLabel->setVisible(false);
        ui->scrollArea->setVisible(true);
    }
    else  //Display just turned off.
    {
        ui->zoomSpinBox->findChild<QLineEdit*>()->deselect();  //On Windows, this wasn't deselecting on its own after using the mouse wheel, and it looked weird to have a disabled spin box with a selection.

        ui->simulationHiddenLabel->setVisible(true);
        ui->scrollArea->setVisible(false);
    }

    ui->zoomWidget->setEnabled(g_simulationSettings->displayOn);
    ui->shadowsButton->setEnabled(g_simulationSettings->displayOn);
    ui->shadowsInfoText->setEnabled(g_simulationSettings->displayOn);

    m_environmentWidget->update();
}



void MainWindow::openSettingsDialog()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    SettingsDialog settingsDialog(this);
    settingsDialog.setWidgetsFromSettings();

    SimulationSettings settingsBefore = *g_simulationSettings;

    if (settingsDialog.exec()) //The user clicked OK
    {
        settingsDialog.setSettingsFromWidgets();
        resetIfTimeIsZeroAndStartingSettingsChanged(settingsBefore, *g_simulationSettings);
        if (!settingsBefore.cloudsOn && g_simulationSettings->cloudsOn)
            m_environmentWidget->makeClouds();
        m_environmentWidget->update();
    }

    if (simulationRunningAtFunctionStart)
        startSimulation();
}


void MainWindow::resetIfTimeIsZeroAndStartingSettingsChanged(SimulationSettings settingsBefore,
                                                             SimulationSettings settingsAfter)
{
    if (m_environment->getElapsedTime() == 0 &&
            (settingsBefore.targetPopulationSize != settingsAfter.targetPopulationSize ||
             settingsBefore.startingEnvironmentWidth != settingsAfter.startingEnvironmentWidth ||
             settingsBefore.startingOrganismEnergy != settingsAfter.startingOrganismEnergy ||
             settingsBefore.startingGenome != settingsAfter.startingGenome))
        resetSimulation();
}



void MainWindow::openEnvironmentDialog()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    EnvironmentDialog environmentDialog(this, m_environment->getElapsedTime());
    environmentDialog.exec();
    m_environmentWidget->update();

    if (simulationRunningAtFunctionStart)
        startSimulation();
}



void MainWindow::openAutoSaveImagesDialog()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    QString imageSavePathBefore = g_simulationSettings->imageSavePath;

    AutoSaveImagesDialog autoSaveImagesDialog(this);

    if (autoSaveImagesDialog.exec()) //The user clicked OK
    {
        autoSaveImagesDialog.setSettingsFromWidgets();

        //If the image save path was changed, also change the default save path.
        if (imageSavePathBefore != g_simulationSettings->imageSavePath)
            g_simulationSettings->rememberedPath = autoSaveImagesDialog.m_imageSavePath;

        if (g_simulationSettings->autoImageSave)
            ui->actionAutomatically_save_images->setIcon(QIcon(QPixmap(":/icons/tick-128.png")));
        else
            ui->actionAutomatically_save_images->setIcon(QIcon(QPixmap(":/icons/cross-128.png")));
    }

    if (simulationRunningAtFunctionStart)
        startSimulation();
}



void MainWindow::openRecoverFilesDialog()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    RecoverAutosaveFilesDialog recoverAutosaveFilesDialog(this);
    recoverAutosaveFilesDialog.exec();

    if (simulationRunningAtFunctionStart)
        startSimulation();
}

void MainWindow::openQuickSummaryDialog()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    QuickSummaryDialog quickSummaryDialog(this);
    quickSummaryDialog.exec();

    if (simulationRunningAtFunctionStart)
        startSimulation();
}

void MainWindow::openAboutDialog()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    AboutDialog aboutDialog(this);
    aboutDialog.exec();

    if (simulationRunningAtFunctionStart)
        startSimulation();
}



void MainWindow::openOrganismInfoDialog(const Organism * organism)
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    OrganismInfoDialog organismInfoDialog(this, organism, m_environment->getElapsedTime(), g_simulationSettings->advancedMode);
    organismInfoDialog.exec();

    if (simulationRunningAtFunctionStart)
        startSimulation();
}



void MainWindow::openStatsAndHistoryDialog()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    StatsAndHistoryDialog environmentInfoDialog(this, m_environment);

    environmentInfoDialog.setDisplayedTab(m_lastDisplayedEnvironmentInfoTab);
    environmentInfoDialog.setDisplayedGraph(m_lastDisplayedEnvironmentInfoGraph);
    environmentInfoDialog.setHistoryType(m_lastDisplayedEnvironmentInfoHistoryType);

    environmentInfoDialog.exec();

    m_lastDisplayedEnvironmentInfoTab = environmentInfoDialog.getDisplayedTab();
    m_lastDisplayedEnvironmentInfoGraph = environmentInfoDialog.getDisplayedGraph();
    m_lastDisplayedEnvironmentInfoHistoryType = environmentInfoDialog.getHistoryType();

    if (simulationRunningAtFunctionStart)
        startSimulation();
}


void MainWindow::startStopSimulationButtonPressed()
{
    if (m_timer.isActive())
        startStopSimulation(false);
    else
        startStopSimulation(true);
}


//This slot starts or stops the simulation, based on the bool it gets.
//It also checks or unchecks the button in the UI.  This is redundant if
//the user started or stopped the simulation, but necessary if some other
//part of the program did.
void MainWindow::startStopSimulation(bool start)
{
    if (start)
    {
        ui->startStopSimulationButton->setText("Pause");
        ui->startStopSimulationLabel->setPixmap(QPixmap::fromImage(QImage(":/icons/pause-256.png")));

        m_environment->setLastStartTime(QDateTime::currentDateTime());
        m_timer.start();

        //If the speed is set to maximum, hide the simulation now
        if (ui->speedSlider->value() == 2)
            turnDisplayOff();
    }
    else //Stop
    {
        if (m_environment->getElapsedTime() == 0)
            ui->startStopSimulationButton->setText("Start");
        else
            ui->startStopSimulationButton->setText("Resume");
        ui->startStopSimulationLabel->setPixmap(QPixmap::fromImage(QImage(":/icons/play-256.png")));

        if (m_timer.isActive())
        {
            double elapsedSeconds = m_environment->getLastStartTime().msecsTo(QDateTime::currentDateTime()) / 1000.0;
            m_environment->addToElapsedRealWorldSeconds(elapsedSeconds);
        }

        m_timer.stop();

        //If the speed is set to maximum, show the simulation now
        if (ui->speedSlider->value() == 2)
            turnDisplayOn();
    }
}




void MainWindow::advanceOneTick()
{
    if (checkForExtinction())
        return;

    m_justSaved = false;

    //At defined intervals, update the environment settings
    if (g_environmentSettings->isProgressionActive() &&
            m_environment->getElapsedTime() % g_simulationSettings->simulationUpdateInterval == 0)
        g_environmentSettings->updateCurrentValues(m_environment->getElapsedTime());

    //Advance the simulation by one tick.
    m_environment->advanceOneTick();

    if (m_environment->possiblyChangeEnvironmentSize())
        setEnvironmentSize();

    if (m_environment->getElapsedTime() % g_simulationSettings->autosaveInterval == 0)
        saveSimulationAutomatic();

    //Draw the simulation to screen, if appropriate.
    if (g_simulationSettings->displayOn)
        m_environmentWidget->update();

    //Save the simulation as an image file, if necessary.
    if (g_simulationSettings->autoImageSave &&
            m_environment->getElapsedTime() % g_simulationSettings->imageSaveInterval == 0)
        saveImageToFileAutomatic();

    updateTimeDisplay();

    if (g_simulationSettings->cloudsOn)
        m_environmentWidget->moveClouds();
}



void MainWindow::setEnvironmentSize()
{
    int newWidth = m_environment->getWidth() * g_simulationSettings->zoom;
    int newHeight = m_environment->getHeight() * g_simulationSettings->zoom;

    m_environmentWidget->setMinimumSize(newWidth, newHeight);
    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->maximum());
}




void MainWindow::updateTimeDisplay()
{
    QLocale addCommas(QLocale::English);
    QString elapedTimeText = addCommas.toString(m_environment->getElapsedTime());
    ui->timeLabel2->setText(elapedTimeText);
}




void MainWindow::resetSimulation()
{
    stopSimulation();

    if (m_environment->populationIsNotExtinct() && m_environment->getElapsedTime() > 0)
    {
        if (QMessageBox::No == QMessageBox::question(this, "Reset simulation?", "Are you sure you want to\n"
                                                     "reset the current simulation?\n\n"
                                                     "If you click 'Yes', all unsaved\n"
                                                     "progress will be lost."))
            return;
    }

    m_environment->reset();

    setEnvironmentSize();
    m_environmentWidget->makeClouds();

    turnDisplayOn();
    m_environmentWidget->update();

    ui->timeLabel2->setText(QString::number(m_environment->getElapsedTime()));
    ui->startStopSimulationButton->setText("Start");
}



void MainWindow::saveImageToFileAutomatic()
{
    QString saveFileName = getPaddedTimeString();
    if (g_simulationSettings->imageSaveHighQuality)
        saveFileName += ".png";
    else
        saveFileName += ".jpg";

    QString savePath = g_simulationSettings->imageSavePath + "/" + "Grovolve_" + m_environment->getDateAndTimeOfSimStart();
    QDir dir;
    dir.mkpath(savePath);

    saveImageToFile2(savePath + "/" + saveFileName,
                     g_simulationSettings->imageSaveHighQuality,
                     false);
}


void MainWindow::saveImageToFileManual()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    QString defaultSaveFileName = m_environment->getDateAndTimeOfSimStart() + "_" + getPaddedTimeString();
    defaultSaveFileName += ".png";
    QString defaultSaveFileNameAndPath = g_simulationSettings->rememberedPath + "/" + defaultSaveFileName;

    QString fullFileName = QFileDialog::getSaveFileName(this, "Save image", defaultSaveFileNameAndPath, "PNG image (*.png)");

    if (fullFileName != "") //User did not hit cancel
    {
        g_simulationSettings->rememberPath(fullFileName);
        saveImageToFile2(fullFileName, true, true);
    }

    if (simulationRunningAtFunctionStart)
        startSimulation();
}

QString MainWindow::getPaddedTimeString()
{
    return QString("%1").arg(m_environment->getElapsedTime(), 12, 10, QChar('0'));
}


void MainWindow::saveImageToFile2(QString saveFileName, bool highQuality, bool shadows)
{
    ui->controlsWidget->setEnabled(false);

    WaitingDialog waitingDialog(this, "Saving image...", false, false);
    waitingDialog.show();
    waitingDialog.setWindowModality(Qt::WindowModal);
    QApplication::processEvents();

    QImage saveImage = m_environmentWidget->paintSimulationToImage(highQuality, shadows);
    saveImage.save(saveFileName, 0, 50);

    ui->controlsWidget->setEnabled(true);
}


void MainWindow::changeZoomLevel(double newZoomLevel)
{
    ui->zoomSpinBox->setValue(newZoomLevel);

    double oldZoom = g_simulationSettings->zoom;
    g_simulationSettings->zoom = newZoomLevel / 100.0;
    double relativeZoomChange = g_simulationSettings->zoom / oldZoom;
    if (relativeZoomChange == 1.0)
        return;

    double hP = ui->scrollArea->horizontalScrollBar()->pageStep();
    double hM = ui->scrollArea->horizontalScrollBar()->maximum();
    double hTotal = hP + hM;
    double hV = ui->scrollArea->horizontalScrollBar()->value();

    double vP = ui->scrollArea->verticalScrollBar()->pageStep();
    double vM = ui->scrollArea->verticalScrollBar()->maximum();
    double vTotal = vP + vM;
    double vV = ui->scrollArea->verticalScrollBar()->value();

    if (g_simulationSettings->displayOn)
    {
        double previousHorizontalCenterFraction;
        if (ui->scrollArea->horizontalScrollBar()->maximum() == 0)
            previousHorizontalCenterFraction = 0.5;
        else
            previousHorizontalCenterFraction = (hV + (hP / 2.0)) / hTotal;

        double previousVerticalCenterFraction;
        if (ui->scrollArea->verticalScrollBar()->maximum() == 0)
            previousVerticalCenterFraction = 1.0;
        else
            previousVerticalCenterFraction = (vV + vP) / vTotal;

        //Resize the environment widget.
        m_environmentWidget->setMinimumSize(m_environment->getWidth() * g_simulationSettings->zoom,
                                            m_environment->getHeight() * g_simulationSettings->zoom);

        double newHTotal = hTotal * relativeZoomChange;
        hP = ui->scrollArea->horizontalScrollBar()->pageStep();
        if (ui->scrollArea->verticalScrollBar()->isVisible())
            hP -= ui->scrollArea->verticalScrollBar()->width();
        hM += newHTotal - hTotal;
        if (hM < 0)
            hM = 0;
        ui->scrollArea->horizontalScrollBar()->setValue((hP + hM) * previousHorizontalCenterFraction - (hP / 2.0) + 0.5);

        double newVTotal = vTotal * relativeZoomChange;
        vP = ui->scrollArea->verticalScrollBar()->pageStep();
        vM += newVTotal - vTotal;
        if (vM < 0)
            vM = 0;
        int newVValue = (vP + vM) * previousVerticalCenterFraction - vP + 0.5;
        ui->scrollArea->verticalScrollBar()->setValue(newVValue);
    }
}



double MainWindow::getVisibleCentreX()
{
    //If the scroll bar's maximum is zero, that means it isn't displayed.  Therefore,
    //simply use half way through the environmentWidget.
    if (ui->scrollArea->horizontalScrollBar()->maximum() == 0)
        return m_environment->getWidth() * g_simulationSettings->zoom / 2.0;
    else
        return ui->scrollArea->horizontalScrollBar()->value() + ui->scrollArea->horizontalScrollBar()->pageStep() / 2.0;
}

double MainWindow::getVisibleBottomY()
{
    //If the scroll bar's maximum is zero, then it isn't displayed.  Therefore,
    //simply use the bottom of the environmentWidget.
    if (ui->scrollArea->verticalScrollBar()->maximum() == 0)
        return m_environment->getHeight();
    else
        return ui->scrollArea->verticalScrollBar()->value() + ui->scrollArea->verticalScrollBar()->pageStep();
}




bool MainWindow::checkForExtinction()
{
    if (m_environment->populationIsExtinct())
    {
        stopSimulation();
        QMessageBox::information(this, "Extinction!", "All plants have died and the\npopulation is now extinct.\n\n"
                                                      "You must reset or load a\nsimulation to continue.");
        return true;
    }
    return false;
}



void MainWindow::closeEvent(QCloseEvent * event)
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    if (m_environment->getElapsedTime() == 0 ||
            m_environment->populationIsExtinct() ||
            m_justSaved)
    {
        event->accept();
        QFile::remove(m_autosavePath);
    }

    else if (QMessageBox::No == QMessageBox::question(this, "Close program?",
                                                      "Are you sure you want to exit\n"
                                                      "the program?\n\n"
                                                      "If you click 'Yes', all unsaved\n"
                                                      "progress will be lost."))
    {
        event->ignore();
        if (simulationRunningAtFunctionStart)
            startSimulation();
    }

    else
    {
        event->accept();
        QFile::remove(m_autosavePath);
    }
}



void MainWindow::saveSimulationManual()
{
    m_resumeSimulationAfterSave = simulationIsRunning();
    stopSimulation();

    QString defaultSaveFileName = m_environment->getDateAndTimeOfSimStart() + "_" + getPaddedTimeString();
    defaultSaveFileName += ".grov";
    QString defaultSaveFileNameAndPath = g_simulationSettings->rememberedPath + "/" + defaultSaveFileName;

    //If the program is in advanced mode, the user is given the option of saving without history.
    bool history = true;
    QString fullFileName;
    if (g_simulationSettings->advancedMode)
    {
        QString filterWithHistory = "Grovolve simulation (*.grov)";
        QString filterWithoutHistory = "Grovolve simulation without history (*.grov)";
        QString chosenFilter;
        fullFileName = QFileDialog::getSaveFileName(this, "Save simulation", defaultSaveFileNameAndPath, filterWithHistory + ";;" + filterWithoutHistory, &chosenFilter);
        history = chosenFilter == filterWithHistory;
    }
    else
        fullFileName = QFileDialog::getSaveFileName(this, "Save simulation", defaultSaveFileNameAndPath, "Grovolve simulation (*.grov)");

    if (fullFileName != "") //User did not hit cancel
    {
        g_simulationSettings->rememberPath(fullFileName);
        saveSimulation(fullFileName, history, "Saving...");
    }
    else //User hit cancel
    {
        if (m_resumeSimulationAfterSave)
            startSimulation();
    }
}



void MainWindow::saveSimulationAutomatic()
{
    m_resumeSimulationAfterSave = true;
    stopSimulation();
    saveSimulation(m_autosavePath, true, "Autosaving...");
}



void MainWindow::saveSimulation(QString fullFileName, bool history, QString message)
{
    ui->controlsWidget->setEnabled(false);

    WaitingDialog * waitingDialog = new WaitingDialog(this, message, true, history);
    waitingDialog->setWindowModality(Qt::WindowModal);
    waitingDialog->show();

    QThread * thread = new QThread;
    SaverAndLoader * saverAndLoader = new SaverAndLoader(fullFileName, m_environment, g_environmentSettings,
                                                         g_simulationSettings, g_stats, history);
    saverAndLoader->moveToThread(thread);

    connect(thread, SIGNAL(started()), saverAndLoader, SLOT(saveSimulation()));
    connect(saverAndLoader, SIGNAL(finishedSaving()), thread, SLOT(quit()));
    connect(saverAndLoader, SIGNAL(finishedSaving()), saverAndLoader, SLOT(deleteLater()));
    connect(saverAndLoader, SIGNAL(finishedSaving()), this, SLOT(finishedSaving()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), waitingDialog, SLOT(deleteLater()));

    thread->start();

    m_justSaved = true;
}


void MainWindow::finishedSaving()
{
    ui->controlsWidget->setEnabled(true);
    if (m_resumeSimulationAfterSave)
        startSimulation();
}



void MainWindow::loadSimulationPrompt()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    QString fullFileName = QFileDialog::getOpenFileName(this, "Load simulation", g_simulationSettings->rememberedPath, "Grovolve simulation (*.grov)");

    if (fullFileName != "") //User did not hit cancel
    {
        g_simulationSettings->rememberPath(fullFileName);
        try
        {
            loadSimulation(fullFileName);
        }
        catch (...)
        {
            QMessageBox::critical(this, "Error loading file", "An error occurred when loading the file.\n\n"
                                                              "The file could be corrupt or it could be of the wrong type.");
        }
    }
    else //Cancelled
    {
        if (simulationRunningAtFunctionStart)
            startSimulation();
    }
}


void MainWindow::loadSimulation(QString fullFileName)
{
    m_environmentWidget->setVisible(false);
    ui->timeLabel2->setText("0");
    ui->controlsWidget->setEnabled(false);

    WaitingDialog * waitingDialog = new WaitingDialog(this, "Loading", true, true);
    waitingDialog->setWindowModality(Qt::WindowModal);
    waitingDialog->show();

    QThread * thread = new QThread;
    SaverAndLoader * saverAndLoader = new SaverAndLoader(fullFileName, m_environment, g_environmentSettings,
                                                         g_simulationSettings, g_stats);
    saverAndLoader->moveToThread(thread);

    connect(thread, SIGNAL(started()), saverAndLoader, SLOT(loadSimulation()));
    connect(saverAndLoader, SIGNAL(finishedLoading()), thread, SLOT(quit()));
    connect(saverAndLoader, SIGNAL(finishedLoading()), saverAndLoader, SLOT(deleteLater()));
    connect(saverAndLoader, SIGNAL(finishedLoading()), this, SLOT(finishedLoading()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), waitingDialog, SLOT(deleteLater()));

    thread->start();
}


void MainWindow::finishedLoading()
{
    //If the save file has no history, reset some things and log the first stats now.
    if (g_stats->m_time.size() == 0 && m_environment->getElapsedTime() == 0)
    {
        m_environment->logStats();
        m_environment->resetTime();
        m_environment->resetAllGenerations();
    }

    turnDisplayOn();
    ui->controlsWidget->setEnabled(true);
    updateTimeDisplay();
    m_environmentWidget->setVisible(true);
    setEnvironmentSize();
    m_justSaved = true;

    if (g_simulationSettings->cloudsOn)
        m_environmentWidget->makeClouds();
    m_environmentWidget->update();

    setClickMode();
}



void MainWindow::saveSettings()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    QString defaultSaveFileName = m_environment->getDateAndTimeOfSimStart();
    defaultSaveFileName += ".grovset";
    QString defaultSaveFileNameAndPath = g_simulationSettings->rememberedPath + "/" + defaultSaveFileName;

    QString fullFileName = QFileDialog::getSaveFileName(this, "Save settings", defaultSaveFileNameAndPath, "Grovolve settings (*.grovset)");

    if (fullFileName != "") //User did not hit cancel
    {
        g_simulationSettings->rememberPath(fullFileName);

        std::ofstream ofs(fullFileName.toLocal8Bit().data(), std::ios_base::out | std::ios::binary);
        boost::iostreams::filtering_ostream out;
        out.push(boost::iostreams::gzip_compressor());
        out.push(ofs);
        boost::archive::text_oarchive ar(out);

        ar << *g_environmentSettings << *g_simulationSettings;
    }

    if (simulationRunningAtFunctionStart)
        startSimulation();
}




void MainWindow::loadSettingsPrompt()
{
    bool simulationRunningAtFunctionStart = simulationIsRunning();
    stopSimulation();

    QString fullFileName = QFileDialog::getOpenFileName(this, "Load settings", g_simulationSettings->rememberedPath, "Grovolve settings (*.grovset)");

    if (fullFileName != "") //User did not hit cancel
    {
        g_simulationSettings->rememberPath(fullFileName);
        try
        {
            loadSettings(fullFileName);
        }
        catch (...)
        {
            QMessageBox::critical(this, "Error loading file", "An error occurred when loading the file.\n\n"
                                                              "The file could be corrupt or it could be of the wrong type.");
        }
    }

    if (simulationRunningAtFunctionStart)
        startSimulation();
}

void MainWindow::loadSettings(QString fullFileName)
{
    g_simulationSettings->rememberPath(fullFileName);
    SimulationSettings settingsBefore = *g_simulationSettings;

    std::ifstream ifs(fullFileName.toLocal8Bit().data(), std::ios_base::in | std::ios_base::binary);
    boost::iostreams::filtering_istream in;
    in.push(boost::iostreams::gzip_decompressor());
    in.push(ifs);
    boost::archive::text_iarchive ar(in);

    EnvironmentSettings loadedEnvironmentSettings;

    ar >> loadedEnvironmentSettings >> *g_simulationSettings;

    //Only the current values are loaded in this function.  This is because we do not want
    //to recreate any progressions that might be active in loadedEnvironmentSettings.
    g_environmentSettings->m_currentValues = loadedEnvironmentSettings.m_currentValues;

    resetIfTimeIsZeroAndStartingSettingsChanged(settingsBefore, *g_simulationSettings);

    switchBasicAdvancedMode();
    setClickMode();
}


QString MainWindow::getExecutableName()
{
    return QFileInfo(QCoreApplication::applicationFilePath()).fileName();
}


void MainWindow::switchBasicMode()
{
    g_simulationSettings->advancedMode = false;
    switchBasicAdvancedMode();
}


void MainWindow::switchAdvancedMode()
{
    g_simulationSettings->advancedMode = true;
    switchBasicAdvancedMode();
}


void MainWindow::switchBasicAdvancedMode()
{
    bool advancedMode = g_simulationSettings->advancedMode;

    ui->actionBasic->setChecked(!advancedMode);
    ui->actionAdvanced->setChecked(advancedMode);

    ui->otherAdvancedControlsWidget->setVisible(advancedMode);
    ui->clickModeWidget->setVisible(advancedMode);

    ui->actionLoad_settings->setVisible(advancedMode);
    ui->actionSave_settings->setVisible(advancedMode);
    ui->actionSave_current_image->setVisible(advancedMode);

    ui->menuBar->clear();
    ui->menuBar->addMenu(ui->menuFile);
    ui->menuBar->addMenu(ui->menuMode);
    if (advancedMode)
        ui->menuBar->addMenu(ui->menuTools);
    ui->menuBar->addMenu(ui->menuHelp);
}



void MainWindow::setClickMode()
{
    ui->clickModeComboBox->setCurrentIndex(g_simulationSettings->clickMode);
}

void MainWindow::switchClickMode(int newClickMode)
{
    switch (newClickMode)
    {
    case 1:
        g_simulationSettings->clickMode = KILL;
        break;
    case 2:
        g_simulationSettings->clickMode = HELP;
        break;
    default:
        g_simulationSettings->clickMode = INFO;
        break;

    }
}


void MainWindow::killOrganism(Organism * organism)
{
    m_environment->killOrganism(organism);
    m_environmentWidget->update();
}

void MainWindow::helpOrganism(Organism * organism)
{
    m_environment->helpOrganism(organism);
    m_environmentWidget->update();
}


void MainWindow::mouseDrag(QPoint change)
{
    ui->scrollArea->horizontalScrollBar()->setValue(ui->scrollArea->horizontalScrollBar()->value() + change.x());
    ui->scrollArea->verticalScrollBar()->setValue(ui->scrollArea->verticalScrollBar()->value() + change.y());
}
