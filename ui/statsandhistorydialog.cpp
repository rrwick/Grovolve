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


#include "statsandhistorydialog.h"
#include "ui_statsandhistorydialog.h"
#include "../program/environment.h"
#include "../program/stats.h"
#include "../settings/simulationsettings.h"
#include "../plant/organism.h"
#include "infotextwidget.h"
#include "singleorganismwidget.h"
#include <QPen>
#include <QFileDialog>
#include <QFont>

StatsAndHistoryDialog::StatsAndHistoryDialog(QWidget * parent, const Environment * const environment) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::StatsAndHistoryDialog), m_environment(environment)
{
    ui->setupUi(this);

    ui->currentPopulationTitle->setFont(g_largeFont);
    ui->accumulatedTotalsTitle->setFont(g_largeFont);
    ui->genomeTitle->setFont(g_largeFont);
    ui->organismTitle->setFont(g_largeFont);
    ui->graphDataComboBox->setFont(g_largeFont);

    ui->organismCountLabel->setText(this->locale().toString(int(environment->getOrganismCount())));
    ui->environmentWidthLabel->setText(this->locale().toString(int(environment->getWidth())));
    ui->populationDensityLabel->setText(formatDoubleForDisplay(environment->getPopulationDensity(), 3, this->locale()));

    ui->seedCountLabel->setText(this->locale().toString(int(environment->getSeedCount())));
    ui->meanSeedsPerPlantLabel->setText(formatDoubleForDisplay(environment->getMeanSeedsPerPlant(), 1, this->locale()));

    ui->organismsSproutedLabel->setText(this->locale().toString(g_stats->m_numberOfOrganismsSprouted));
    ui->organismsStarvedLabel->setText(this->locale().toString(g_stats->m_numberOfOrganismsDiedFromStarvation));
    ui->organismsDiedFromBadLuckLabel->setText(this->locale().toString(g_stats->m_numberOfOrganismsDiedFromBadLuck));
    ui->seedsGeneratedLabel->setText(this->locale().toString(g_stats->m_numberOfSeedsGenerated));

    ui->averageGenerationLabel->setText(formatDoubleForDisplay(environment->getAverageGeneration(), 1, this->locale()));

    ui->tallestPlantLabel->setText(formatDoubleForDisplay(environment->getTallestPlantHeight(), 1, this->locale()));
    ui->heaviestPlantLabel->setText(formatDoubleForDisplay(environment->getHeaviestPlantMass(), 1, this->locale()));

    ui->meanEnergyPerSeedLabel->setText(formatDoubleForDisplay(environment->getAverageEnergyPerSeed(), 1, this->locale()));

    int elapsedSeconds = environment->getElapsedRealWorldSeconds();
    int elapsedMinutes = elapsedSeconds / 60;
    elapsedSeconds %= 60;
    int elapsedHours = elapsedMinutes / 60;
    elapsedMinutes %= 60;
    int elapsedDays = elapsedHours / 24;
    elapsedHours %= 24;
    QString elapsedTimeString;
    if (elapsedDays > 1)
        elapsedTimeString += QString::number(elapsedDays) + " days, ";
    if (elapsedDays == 1)
        elapsedTimeString += QString::number(elapsedDays) + " day, ";
    if (elapsedHours > 1)
        elapsedTimeString += QString::number(elapsedHours) + " hours, ";
    else if (elapsedHours == 1)
        elapsedTimeString += QString::number(elapsedHours) + " hour, ";
    if (elapsedMinutes > 0)
        elapsedTimeString += QString::number(elapsedMinutes) + " min, ";
    elapsedTimeString += QString::number(elapsedSeconds) + " sec";
    ui->realWorldTimeLabel->setText(elapsedTimeString);

    ui->organismTypeInfoText->setInfoText("The genome below can either be an average genome for the specified point "
                                          "in history or a randomly-chosen organism at that point in history.");
    ui->historyOrganismInfoText->setInfoText("This is what an organism would look like if it had the genome shown to the left.");
    setGenomeHistoryInfoText();

    setMinimumHeight(sizeHint().height());
    setMinimumWidth(1.4 * sizeHint().width());

    ui->customPlot->xAxis->setLabel("time");
    ui->customPlot->setInteraction(QCP::iRangeDrag, true);
    ui->customPlot->setInteraction(QCP::iRangeZoom, true);
    ui->customPlot->axisRect()->setRangeDrag(Qt::Horizontal);
    ui->customPlot->axisRect()->setRangeZoom(Qt::Horizontal);

    //Move legend to the top left to begin.  It can then be dragged around
    //by the user.
    QRectF rect = ui->customPlot->axisRect()->insetLayout()->insetRect(0);
    rect.moveTopLeft(QPointF(0.0, 0.0));
    ui->customPlot->axisRect()->insetLayout()->setInsetRect(0, rect);
    ui->customPlot->replot();

    int maxSliderPosition = g_stats->logEntries() - 1;
    ui->genomeHistoryTimeSlider->setMaximum(maxSliderPosition);
    ui->genomeHistoryTimeSlider->setValue(maxSliderPosition);
    m_sizeOfEachTick = g_simulationSettings->statLoggingInterval * environment->getLogIntervalMultiplier();
    ui->genomeHistoryTimeSpinBox->setSingleStep(m_sizeOfEachTick);
    ui->genomeHistoryTimeSpinBox->setMinimum(0);
    ui->genomeHistoryTimeSpinBox->setMaximum(maxSliderPosition * m_sizeOfEachTick);

    if (maxSliderPosition == 0)
    {
        ui->genomeHistoryWidget->setEnabled(false);
        ui->genomeHistoryTimeSlider->setEnabled(false);
    }

    genomeHistoryChanged();

    setOrganismWidgetRange();

    resetGraphXRange();

    ui->historyGenomeTextEdit->setFont(getMonospaceFont());

    connect(ui->graphDataComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(graphChanged(int)));
    connect(ui->customPlot->xAxis, SIGNAL(rangeChanged(QCPRange, QCPRange)), this, SLOT(xRangeChanged(QCPRange, QCPRange)));
    connect(ui->outputCurrentInfoCSVFileButton, SIGNAL(clicked()), this, SLOT(outputCurrentInfoButtonPressed()));
    connect(ui->outputHistoryInfoCSVFileButton, SIGNAL(clicked()), this, SLOT(outputHistoryInfoButtonPressed()));
    connect(ui->genomeHistoryTimeSlider, SIGNAL(valueChanged(int)), this, SLOT(genomeHistoryChanged()));
    connect(ui->historyTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setOrganismWidgetRange()));
    connect(ui->historyTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setGenomeHistoryInfoText()));
    connect(ui->historyTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(genomeHistoryChanged()));
    connect(ui->genomeHistoryTimeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(genomeHistorySpinBoxChanged()));
    connect(ui->genomeHistoryTimeSpinBox, SIGNAL(editingFinished()), this, SLOT(genomeHistorySpinBoxEditingFinished()));
    connect(ui->customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoveSignal(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mousePressSignal(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleaseSignal(QMouseEvent*)));
    connect(ui->customPlot, SIGNAL(beforeReplot()), this, SLOT(beforeReplot()));
    connect(ui->logScaleCheckBox, SIGNAL(toggled(bool)), this, SLOT(logScaleChanged()));
}

StatsAndHistoryDialog::~StatsAndHistoryDialog()
{
    delete ui;
}



void StatsAndHistoryDialog::setGenomeHistoryInfoText()
{
    if (ui->historyTypeComboBox->currentIndex() == 0)
    {
        ui->historyGenomeInfoText->setInfoText("This is the average genome at the point in history specified above. "
                                               "It is determined by finding the most common value (i.e. mode) at each point in "
                                               "the genome.  It therefore does not represent the exact genome of any particular "
                                               "organism at that point in history, but is rather representative of "
                                               "the whole population.");
    }
    else
    {
        ui->historyGenomeInfoText->setInfoText("This is the genome of a randomly-selected organism at the point in history specified above. ");
    }
}


void StatsAndHistoryDialog::graphChanged(int newGraphIndex)
{
    ui->customPlot->clearGraphs();

    QPen graphPen;
    graphPen.setWidthF(2.5);

    switch (newGraphIndex)
    {
    case 0: //Tallest plants
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::black);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setName("Height of tallest plant");
        ui->customPlot->graph(0)->setData(getTimeVector(TALLEST_PLANT),
                                          getDataVector(TALLEST_PLANT));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(1)->setPen(graphPen);
        ui->customPlot->graph(1)->setName("99th percentile plant height");
        ui->customPlot->graph(1)->setData(getTimeVector(NINETY_NINTH_PERCENTILE_PLANT_HEIGHT),
                                          getDataVector(NINETY_NINTH_PERCENTILE_PLANT_HEIGHT));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkGreen);
        ui->customPlot->graph(2)->setPen(graphPen);
        ui->customPlot->graph(2)->setName("95th percentile plant height");
        ui->customPlot->graph(2)->setData(getTimeVector(NINETY_FIFTH_PERCENTILE_PLANT_HEIGHT),
                                          getDataVector(NINETY_FIFTH_PERCENTILE_PLANT_HEIGHT));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkBlue);
        ui->customPlot->graph(3)->setPen(graphPen);
        ui->customPlot->graph(3)->setName("90th percentile plant height");
        ui->customPlot->graph(3)->setData(getTimeVector(NINETIETH_PERCENTILE_PLANT_HEIGHT),
                                          getDataVector(NINETIETH_PERCENTILE_PLANT_HEIGHT));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkYellow);
        ui->customPlot->graph(4)->setPen(graphPen);
        ui->customPlot->graph(4)->setName("Median plant height");
        ui->customPlot->graph(4)->setData(getTimeVector(MEDIAN_PLANT_HEIGHT),
                                          getDataVector(MEDIAN_PLANT_HEIGHT));

        turnOnLegend();
        ui->customPlot->yAxis->setLabel("height");
        break;

    case 1: //Heaviest plants
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::black);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setName("Mass of heaviest plant");
        ui->customPlot->graph(0)->setData(getTimeVector(HEAVIEST_PLANT),
                                          getDataVector(HEAVIEST_PLANT));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(1)->setPen(graphPen);
        ui->customPlot->graph(1)->setName("99th percentile plant mass");
        ui->customPlot->graph(1)->setData(getTimeVector(NINETY_NINTH_PERCENTILE_PLANT_MASS),
                                          getDataVector(NINETY_NINTH_PERCENTILE_PLANT_MASS));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkGreen);
        ui->customPlot->graph(2)->setPen(graphPen);
        ui->customPlot->graph(2)->setName("95th percentile plant mass");
        ui->customPlot->graph(2)->setData(getTimeVector(NINETY_FIFTH_PERCENTILE_PLANT_MASS),
                                          getDataVector(NINETY_FIFTH_PERCENTILE_PLANT_MASS));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkBlue);
        ui->customPlot->graph(3)->setPen(graphPen);
        ui->customPlot->graph(3)->setName("90th percentile plant mass");
        ui->customPlot->graph(3)->setData(getTimeVector(NINETIETH_PERCENTILE_PLANT_MASS),
                                          getDataVector(NINETIETH_PERCENTILE_PLANT_MASS));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkYellow);
        ui->customPlot->graph(4)->setPen(graphPen);
        ui->customPlot->graph(4)->setName("Median plant mass");
        ui->customPlot->graph(4)->setData(getTimeVector(MEDIAN_PLANT_MASS),
                                          getDataVector(MEDIAN_PLANT_MASS));

        turnOnLegend();
        ui->customPlot->yAxis->setLabel("mass");
        break;

    case 2: //Plant energy
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::black);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setName("Most plant energy");
        ui->customPlot->graph(0)->setData(getTimeVector(MOST_PLANT_ENERGY),
                                          getDataVector(MOST_PLANT_ENERGY));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(1)->setPen(graphPen);
        ui->customPlot->graph(1)->setName("99th percentile plant energy");
        ui->customPlot->graph(1)->setData(getTimeVector(NINETY_NINTH_PERCENTILE_PLANT_ENERGY),
                                          getDataVector(NINETY_NINTH_PERCENTILE_PLANT_ENERGY));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkGreen);
        ui->customPlot->graph(2)->setPen(graphPen);
        ui->customPlot->graph(2)->setName("95th percentile plant energy");
        ui->customPlot->graph(2)->setData(getTimeVector(NINETY_FIFTH_PERCENTILE_PLANT_ENERGY),
                                          getDataVector(NINETY_FIFTH_PERCENTILE_PLANT_ENERGY));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkBlue);
        ui->customPlot->graph(3)->setPen(graphPen);
        ui->customPlot->graph(3)->setName("90th percentile plant energy");
        ui->customPlot->graph(3)->setData(getTimeVector(NINETIETH_PERCENTILE_PLANT_ENERGY),
                                          getDataVector(NINETIETH_PERCENTILE_PLANT_ENERGY));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkYellow);
        ui->customPlot->graph(4)->setPen(graphPen);
        ui->customPlot->graph(4)->setName("Median plant energy");
        ui->customPlot->graph(4)->setData(getTimeVector(MEDIAN_PLANT_ENERGY),
                                          getDataVector(MEDIAN_PLANT_ENERGY));

        turnOnLegend();
        ui->customPlot->yAxis->setLabel("energy");
        break;

    case 3: //Population density
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setData(getTimeVector(POPULATION),
                                          getDataVector(POPULATION));

        ui->customPlot->yAxis->setLabel("population density");
        ui->customPlot->legend->setVisible(false);
        break;

    case 4: //Number of seeds
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setData(getTimeVector(SEED_COUNT),
                                          getDataVector(SEED_COUNT));

        ui->customPlot->legend->setVisible(false);
        ui->customPlot->yAxis->setLabel("seeds");
        break;

    case 5: //Energy per seed
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setData(getTimeVector(ENERGY_PER_SEED),
                                          getDataVector(ENERGY_PER_SEED));

        ui->customPlot->legend->setVisible(false);
        ui->customPlot->yAxis->setLabel("energy");
        break;

    case 6: //Rates
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setName("Sprout rate");
        ui->customPlot->graph(0)->setData(getTimeVector(SPROUT_RATE),
                                          getDataVector(SPROUT_RATE));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkGreen);
        ui->customPlot->graph(1)->setPen(graphPen);
        ui->customPlot->graph(1)->setName("Rate of non-starvation death");
        ui->customPlot->graph(1)->setData(getTimeVector(NON_STARVATION_DEATH_RATE),
                                          getDataVector(NON_STARVATION_DEATH_RATE));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkBlue);
        ui->customPlot->graph(2)->setPen(graphPen);
        ui->customPlot->graph(2)->setName("Rate of starvation death");
        ui->customPlot->graph(2)->setData(getTimeVector(STARVATION_DEATH_RATE),
                                          getDataVector(STARVATION_DEATH_RATE));

        turnOnLegend();
        ui->customPlot->yAxis->setLabel("rate");
        break;
    }

    setGraphYRange();
}

void StatsAndHistoryDialog::turnOnLegend()
{
    // set the placement of the legend (index 0 in the axis rect's inset layout) to not be
    // border-aligned (default), but freely, so we can reposition it anywhere:
    ui->customPlot->axisRect()->insetLayout()->setInsetPlacement(0, QCPLayoutInset::ipFree);
    m_draggingLegend = false;
    ui->customPlot->legend->setVisible(true);
}



void StatsAndHistoryDialog::resetGraphXRange()
{
    ui->customPlot->xAxis->setRangeLower(0.0);
    ui->customPlot->xAxis->setRangeUpper(m_environment->getElapsedTime());

    ui->customPlot->replot();
}


void StatsAndHistoryDialog::setGraphYRange()
{
    ui->customPlot->yAxis->rescale();

    bool logScale = ui->logScaleCheckBox->isChecked();

    if (logScale)
        ui->customPlot->yAxis->setScaleType(QCPAxis::stLogarithmic);
    else
        ui->customPlot->yAxis->setScaleType(QCPAxis::stLinear);

    double yRangeMax;
    double yRangeMin;

    //If there isn't enough data to display yet, just set the range
    //to a default.
    if (ui->customPlot->graph(0)->data()->size() < 2)
    {
        yRangeMax = 10.0;
        if (logScale)
            yRangeMin = 1.0;
        else
            yRangeMin = 0.0;
    }

    else if (logScale)
    {
        yRangeMax = ui->customPlot->yAxis->range().upper * 1.2;
        yRangeMin = ui->customPlot->yAxis->range().lower / 1.2;
    }
    else
    {
        yRangeMax = ui->customPlot->yAxis->range().upper * 1.05;
        yRangeMin = 0.0;
    }

    ui->customPlot->yAxis->setRangeLower(yRangeMin);
    ui->customPlot->yAxis->setRangeUpper(yRangeMax);

    ui->customPlot->replot();
}





void StatsAndHistoryDialog::xRangeChanged(const QCPRange &newRange, const QCPRange &oldRange)
{
    double currentSpan = ui->customPlot->xAxis->range().upper - ui->customPlot->xAxis->range().lower;
    double minimumSpan = g_simulationSettings->minGraphSpan;
    double elapsedTime = m_environment->getElapsedTime();

    if (minimumSpan > elapsedTime)
        minimumSpan = elapsedTime;

    if (minimumSpan == 0)
        ui->customPlot->xAxis->setRange(0.0, 1.0);

    else if (currentSpan < minimumSpan)
    {
        double middleOfOldRange = (oldRange.upper - oldRange.lower) / 2.0 + oldRange.lower;
        ui->customPlot->xAxis->setRange(middleOfOldRange - minimumSpan / 2.0,
                                        middleOfOldRange + minimumSpan / 2.0);
    }
    else if (currentSpan > double(elapsedTime))
        ui->customPlot->xAxis->setRange(0.0, double(elapsedTime));
    else if (newRange.lower < 0.0)
        ui->customPlot->xAxis->setRange(0.0, currentSpan);
    else if (newRange.upper > elapsedTime)
        ui->customPlot->xAxis->setRange(double(elapsedTime) - currentSpan, double(elapsedTime));
}




int StatsAndHistoryDialog::getDisplayedTab() const
{
    return ui->tabWidget->currentIndex();
}

int StatsAndHistoryDialog::getDisplayedGraph() const
{
    return ui->graphDataComboBox->currentIndex();
}

int StatsAndHistoryDialog::getHistoryType() const
{
    return ui->historyTypeComboBox->currentIndex();
}

bool StatsAndHistoryDialog::getLogScale() const
{
    return ui->logScaleCheckBox->isChecked();
}

void StatsAndHistoryDialog::setDisplayedTab(int tabIndex)
{
    ui->tabWidget->setCurrentIndex(tabIndex);
}

void StatsAndHistoryDialog::setDisplayedGraph(int graphIndex)
{
    ui->graphDataComboBox->setCurrentIndex(graphIndex);
    graphChanged(graphIndex);
}

void StatsAndHistoryDialog::setHistoryType(int historyType)
{
    ui->historyTypeComboBox->setCurrentIndex(historyType);
    genomeHistoryChanged();
}

void StatsAndHistoryDialog::setLogScale(bool logScale)
{
    ui->logScaleCheckBox->setChecked(logScale);
    logScaleChanged();
}


void StatsAndHistoryDialog::outputHistoryInfoButtonPressed()
{
    if (g_stats->m_time.size() == 0)
    {
        QMessageBox::information(this, "No data to output", "There is no data to output to file.\n\n"
                                                            "Run the simulation for a while to generate\n"
                                                            "data that can be outputted.");
        return;
    }

    QString saveFileNameAndPath = QFileDialog::getSaveFileName(this, "Save population history data", g_simulationSettings->rememberedPath,
                                                               "Comma-separated values file (*.csv)");
    if (saveFileNameAndPath != "")
    {
        QFile saveFile(saveFileNameAndPath);
        if (saveFile.open(QIODevice::WriteOnly))
        {
            g_simulationSettings->rememberPath(saveFileNameAndPath);

            QTextStream stream(&saveFile);
            stream << makeHistoryInfoCSVHeaderLine() << endl;
            int timeCount = int(g_stats->m_time.size());
            for (int i = 0; i < timeCount; ++i)
                stream << makeHistoryInfoCSVBodyLine(i) << endl;
        }
        else
            QMessageBox::critical(this, "Error saving file", "An error was encountered while\n"
                                                             "attempting to save to file.");
        close();
    }
}


QString StatsAndHistoryDialog::makeHistoryInfoCSVHeaderLine()
{
    QString header;
    header += "Time,";
    header += "Population density,";
    header += "Height of tallest plant,";
    header += "99th percentile plant height,";
    header += "95th percentile plant height,";
    header += "90th percentile plant height,";
    header += "Median plant height,";
    header += "Mass of heaviest plant,";
    header += "99th percentile plant mass,";
    header += "95th percentile plant mass,";
    header += "90th percentile plant mass,";
    header += "Median plant mass,";
    header += "Most plant energy,";
    header += "99th percentile plant energy,";
    header += "95th percentile plant energy,";
    header += "90th percentile plant energy,";
    header += "Median plant energy,";
    header += "Mean seeds per plant,";
    header += "Mean energy per seed,";
    header += "Plant sprout rate,";
    header += "Non-starvation death rate,";
    header += "Starvation death rate,";
    header += "Average genome,";
    header += "Randomly selected genome";
    return header;
}
QString StatsAndHistoryDialog::makeHistoryInfoCSVBodyLine(int i)
{
    QString body;
    body += QString::number(g_stats->m_time[i]) + ",";
    body += QString::number(g_stats->m_populationDensity[i]) + ",";
    body += QString::number(g_stats->m_tallestPlantHeight[i]) + ",";
    body += QString::number(g_stats->m_99thPercentilePlantHeight[i]) + ",";
    body += QString::number(g_stats->m_95thPercentilePlantHeight[i]) + ",";
    body += QString::number(g_stats->m_90thPercentilePlantHeight[i]) + ",";
    body += QString::number(g_stats->m_medianPlantHeight[i]) + ",";
    body += QString::number(g_stats->m_heaviestPlantMass[i]) + ",";
    body += QString::number(g_stats->m_99thPercentilePlantMass[i]) + ",";
    body += QString::number(g_stats->m_95thPercentilePlantMass[i]) + ",";
    body += QString::number(g_stats->m_90thPercentilePlantMass[i]) + ",";
    body += QString::number(g_stats->m_medianPlantMass[i]) + ",";
    body += QString::number(g_stats->m_mostPlantEnergy[i]) + ",";
    body += QString::number(g_stats->m_99thPercentilePlantEnergy[i]) + ",";
    body += QString::number(g_stats->m_95thPercentilePlantEnergy[i]) + ",";
    body += QString::number(g_stats->m_90thPercentilePlantEnergy[i]) + ",";
    body += QString::number(g_stats->m_medianPlantEnergy[i]) + ",";
    body += QString::number(g_stats->m_meanSeedsPerPlant[i]) + ",";
    body += QString::number(g_stats->m_meanEnergyPerSeed[i]) + ",";
    body += QString::number(g_stats->m_sproutRate[i]) + ",";
    body += QString::number(g_stats->m_badLuckDeathRate[i]) + ",";
    body += QString::number(g_stats->m_starvationRate[i]) + ",";
    body += g_stats->m_averageGenomeOrganism[i]->getGenome()->outputAsString() + ",";
    body += g_stats->m_randomGenomeOrganism[i]->getGenome()->outputAsString();
    return body;
}



void StatsAndHistoryDialog::outputCurrentInfoButtonPressed()
{
    if (m_environment->getOrganismCount() == 0)
    {
        QMessageBox::information(this, "No data to output", "There are no organisms currently alive, "
                                                            "and therefore no data to output.");
        return;
    }

    QString saveFileNameAndPath = QFileDialog::getSaveFileName(this, "Save current population data", g_simulationSettings->rememberedPath,
                                                               "Comma-separated values file (*.csv)");
    if (saveFileNameAndPath != "")
    {
        QFile saveFile(saveFileNameAndPath);
        if (saveFile.open(QIODevice::WriteOnly))
        {
            g_simulationSettings->rememberPath(saveFileNameAndPath);
            QTextStream stream(&saveFile);
            stream << m_environment->outputAllInfoOnCurrentPopulation() << endl;
        }
        else
            QMessageBox::critical(this, "Error saving file", "An error was encountered while\n"
                                                             "attempting to save to file.");
        close();
    }
}


QVector<double> StatsAndHistoryDialog::getDataVector(GraphData graphData)
{
    std::vector<double> * statVector;
    switch (graphData)
    {
    case POPULATION:
        statVector = &(g_stats->m_populationDensity);
        break;

    case TALLEST_PLANT:
        statVector = &(g_stats->m_tallestPlantHeight);
        break;
    case NINETY_NINTH_PERCENTILE_PLANT_HEIGHT:
        statVector = &(g_stats->m_99thPercentilePlantHeight);
        break;
    case NINETY_FIFTH_PERCENTILE_PLANT_HEIGHT:
        statVector = &(g_stats->m_95thPercentilePlantHeight);
        break;
    case NINETIETH_PERCENTILE_PLANT_HEIGHT:
        statVector = &(g_stats->m_90thPercentilePlantHeight);
        break;
    case MEDIAN_PLANT_HEIGHT:
        statVector = &(g_stats->m_medianPlantHeight);
        break;

    case HEAVIEST_PLANT:
        statVector = &(g_stats->m_heaviestPlantMass);
        break;
    case NINETY_NINTH_PERCENTILE_PLANT_MASS:
        statVector = &(g_stats->m_99thPercentilePlantMass);
        break;
    case NINETY_FIFTH_PERCENTILE_PLANT_MASS:
        statVector = &(g_stats->m_95thPercentilePlantMass);
        break;
    case NINETIETH_PERCENTILE_PLANT_MASS:
        statVector = &(g_stats->m_90thPercentilePlantMass);
        break;
    case MEDIAN_PLANT_MASS:
        statVector = &(g_stats->m_medianPlantMass);
        break;

    case MOST_PLANT_ENERGY:
        statVector = &(g_stats->m_mostPlantEnergy);
        break;
    case NINETY_NINTH_PERCENTILE_PLANT_ENERGY:
        statVector = &(g_stats->m_99thPercentilePlantEnergy);
        break;
    case NINETY_FIFTH_PERCENTILE_PLANT_ENERGY:
        statVector = &(g_stats->m_95thPercentilePlantEnergy);
        break;
    case NINETIETH_PERCENTILE_PLANT_ENERGY:
        statVector = &(g_stats->m_90thPercentilePlantEnergy);
        break;
    case MEDIAN_PLANT_ENERGY:
        statVector = &(g_stats->m_medianPlantEnergy);
        break;

    case SEED_COUNT:
        statVector = &(g_stats->m_meanSeedsPerPlant);
        break;
    case ENERGY_PER_SEED:
        statVector = &(g_stats->m_meanEnergyPerSeed);
        break;
    case SPROUT_RATE:
        statVector = &(g_stats->m_sproutRate);
        break;
    case NON_STARVATION_DEATH_RATE:
        statVector = &(g_stats->m_badLuckDeathRate);
        break;
    case STARVATION_DEATH_RATE:
        statVector = &(g_stats->m_starvationRate);
        break;
    default:
        return QVector<double>();
    }

    QVector<double> returnVector;
    for (size_t i = 0; i < statVector->size(); ++i)
    {
        if ((*statVector)[i] > 0)
            returnVector.push_back((*statVector)[i]);
    }

    return returnVector;
}


QVector<double> StatsAndHistoryDialog::getTimeVector(GraphData graphData)
{
    std::vector<double> * statVector;
    switch (graphData)
    {
    case POPULATION:
        statVector = &(g_stats->m_populationDensity);
        break;

    case TALLEST_PLANT:
        statVector = &(g_stats->m_tallestPlantHeight);
        break;
    case NINETY_NINTH_PERCENTILE_PLANT_HEIGHT:
        statVector = &(g_stats->m_99thPercentilePlantHeight);
        break;
    case NINETY_FIFTH_PERCENTILE_PLANT_HEIGHT:
        statVector = &(g_stats->m_95thPercentilePlantHeight);
        break;
    case NINETIETH_PERCENTILE_PLANT_HEIGHT:
        statVector = &(g_stats->m_90thPercentilePlantHeight);
        break;
    case MEDIAN_PLANT_HEIGHT:
        statVector = &(g_stats->m_medianPlantHeight);
        break;

    case HEAVIEST_PLANT:
        statVector = &(g_stats->m_heaviestPlantMass);
        break;
    case NINETY_NINTH_PERCENTILE_PLANT_MASS:
        statVector = &(g_stats->m_99thPercentilePlantMass);
        break;
    case NINETY_FIFTH_PERCENTILE_PLANT_MASS:
        statVector = &(g_stats->m_95thPercentilePlantMass);
        break;
    case NINETIETH_PERCENTILE_PLANT_MASS:
        statVector = &(g_stats->m_90thPercentilePlantMass);
        break;
    case MEDIAN_PLANT_MASS:
        statVector = &(g_stats->m_medianPlantMass);
        break;

    case MOST_PLANT_ENERGY:
        statVector = &(g_stats->m_mostPlantEnergy);
        break;
    case NINETY_NINTH_PERCENTILE_PLANT_ENERGY:
        statVector = &(g_stats->m_99thPercentilePlantEnergy);
        break;
    case NINETY_FIFTH_PERCENTILE_PLANT_ENERGY:
        statVector = &(g_stats->m_95thPercentilePlantEnergy);
        break;
    case NINETIETH_PERCENTILE_PLANT_ENERGY:
        statVector = &(g_stats->m_90thPercentilePlantEnergy);
        break;
    case MEDIAN_PLANT_ENERGY:
        statVector = &(g_stats->m_medianPlantEnergy);
        break;

    case SEED_COUNT:
        statVector = &(g_stats->m_meanSeedsPerPlant);
        break;
    case ENERGY_PER_SEED:
        statVector = &(g_stats->m_meanEnergyPerSeed);
        break;

    case SPROUT_RATE:
        statVector = &(g_stats->m_sproutRate);
        break;
    case NON_STARVATION_DEATH_RATE:
        statVector = &(g_stats->m_badLuckDeathRate);
        break;
    case STARVATION_DEATH_RATE:
        statVector = &(g_stats->m_starvationRate);
        break;
    default:
        return QVector<double>();
    }

    QVector<double> returnVector;
    for (size_t i = 0; i < statVector->size(); ++i)
    {
        if ((*statVector)[i] > 0)
            returnVector.push_back(g_stats->m_time[i]);
    }

    return returnVector;
}




void StatsAndHistoryDialog::setOrganismWidgetRange()
{
    double heightExtent, rightExtent, leftExtent;

    HistoryOrganismType historyOrganismType;
    if (ui->historyTypeComboBox->currentIndex() == 0)
        historyOrganismType = AVERAGE_GENOME;
    else
        historyOrganismType = RANDOM_ORGANISM;

    heightExtent = g_stats->getHistoryOrganismHeightExtent(historyOrganismType);
    leftExtent = g_stats->getHistoryOrganismLeftExtent(historyOrganismType);
    rightExtent = g_stats->getHistoryOrganismRightExtent(historyOrganismType);

    double sideExtent = std::max(rightExtent, -1.0 * leftExtent);

    ui->historyOrganismWidget->setHeightExtent(heightExtent);
    ui->historyOrganismWidget->setLeftExtent(-1.0 * sideExtent);
    ui->historyOrganismWidget->setRightExtent(sideExtent);
}



void StatsAndHistoryDialog::genomeHistoryChanged()
{
    int position = ui->genomeHistoryTimeSlider->value();

    ui->genomeHistoryTimeSpinBox->setValue(position * m_sizeOfEachTick);

    Organism * historyOrganism;
    if (ui->historyTypeComboBox->currentIndex() == 0)
        historyOrganism = g_stats->m_averageGenomeOrganism[position];
    else
        historyOrganism = g_stats->m_randomGenomeOrganism[position];

    if (historyOrganism != 0)
        ui->historyGenomeTextEdit->setText(historyOrganism->getGenome()->outputAsString());
    else
        ui->historyGenomeTextEdit->setText("");

    ui->historyOrganismWidget->setOrganism(historyOrganism);
    ui->historyOrganismWidget->update();
}



void StatsAndHistoryDialog::genomeHistorySpinBoxChanged()
{
    int value = ui->genomeHistoryTimeSpinBox->value();
    int roundedValue = roundHistoryValue(value);

    //If the value changed when rounded, don't do anything now,
    //as that means the user is typing a value in and we don't
    //we'll handle stuff when the editing is finished.
    if (value != roundedValue)
        return;

    ui->genomeHistoryTimeSpinBox->setValue(roundedValue);
    ui->genomeHistoryTimeSlider->setValue(roundedValue / m_sizeOfEachTick);
}

void StatsAndHistoryDialog::genomeHistorySpinBoxEditingFinished()
{
    int roundedValue = roundHistoryValue(ui->genomeHistoryTimeSpinBox->value());
    ui->genomeHistoryTimeSpinBox->setValue(roundedValue);
    ui->genomeHistoryTimeSlider->setValue(roundedValue / m_sizeOfEachTick);
}


int StatsAndHistoryDialog::roundHistoryValue(int valueToRound)
{
    double ticks = double(valueToRound) / m_sizeOfEachTick;
    int roundedTicks = int(ticks + 0.5);
    return roundedTicks * m_sizeOfEachTick;
}


//The following code was taken from:
//http://www.qcustomplot.com/index.php/support/forum/481
void StatsAndHistoryDialog::mouseMoveSignal(QMouseEvent *event)
{
    if (m_draggingLegend)
    {
        QRectF rect = ui->customPlot->axisRect()->insetLayout()->insetRect(0);
        // since insetRect is in axisRect coordinates (0..1), we transform the mouse position:
        QPointF mousePoint((event->pos().x() - ui->customPlot->axisRect()->left()) / double(ui->customPlot->axisRect()->width()),
                           (event->pos().y() - ui->customPlot->axisRect()->top()) / double(ui->customPlot->axisRect()->height()));

        rect.moveTopLeft(mousePoint - m_dragLegendOrigin);

        //Keep the legend in the axis rect
        double legendWidthRatio = double(ui->customPlot->legend->outerRect().width()) / ui->customPlot->axisRect()->width();
        double legendHeightRatio = double(ui->customPlot->legend->outerRect().height()) / ui->customPlot->axisRect()->height();
        if (rect.top() < 0.0)
            rect.moveTop(0.0);
        if (rect.left() < 0.0)
            rect.moveLeft(0.0);
        if (rect.top() > 1.0 - legendHeightRatio)
            rect.moveTop(1.0 - legendHeightRatio);
        if (rect.left() > 1.0 - legendWidthRatio)
            rect.moveLeft(1.0 - legendWidthRatio);

        ui->customPlot->axisRect()->insetLayout()->setInsetRect(0, rect);
        ui->customPlot->replot();
    }

}
void StatsAndHistoryDialog::mousePressSignal(QMouseEvent *event)
{
    if (ui->customPlot->legend->selectTest(event->pos(), false) > 0)
    {
        m_draggingLegend = true;
        // since insetRect is in axisRect coordinates (0..1), we transform the mouse position:
        QPointF mousePoint((event->pos().x() - ui->customPlot->axisRect()->left()) / double(ui->customPlot->axisRect()->width()),
                           (event->pos().y() - ui->customPlot->axisRect()->top()) / double(ui->customPlot->axisRect()->height()));
        m_dragLegendOrigin = mousePoint - ui->customPlot->axisRect()->insetLayout()->insetRect(0).topLeft();
    }
}
void StatsAndHistoryDialog::mouseReleaseSignal(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_draggingLegend = false;
}
void StatsAndHistoryDialog::beforeReplot()
{
    // this is to prevent the legend from stretching if the plot is stretched.
    // Since we've set the inset placement to be ipFree, the width/height of the legend
    // is also defined in axisRect coordinates (0..1) and thus would stretch.
    // This is due to change in a future release (probably QCP 2.0) since it's basically a design mistake.
    ui->customPlot->legend->setMaximumSize(ui->customPlot->legend->minimumSizeHint());
}


void StatsAndHistoryDialog::logScaleChanged()
{
    setGraphYRange();
}
