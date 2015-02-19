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
    ui->meanHeightOfFullyGrownPlantsLabel->setText(formatDoubleForDisplay(environment->getAverageHeightOfFullyGrownPlants(), 1, this->locale()));

    ui->heaviestPlantLabel->setText(formatDoubleForDisplay(environment->getHeaviestPlantMass(), 1, this->locale()));
    ui->meanMassOfFullyGrownPlantsLabel->setText(formatDoubleForDisplay(environment->getMeanMassOfFullyGrownPlants(), 1, this->locale()));

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

    ui->organismTypeInfoText->setInfoText("The historical genome below can either be an average genome for the specified point "
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
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setName("Height of tallest plant");
        ui->customPlot->graph(0)->setData(getTimeVector(TALLEST_PLANT),
                                          getDataVector(TALLEST_PLANT));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkGreen);
        ui->customPlot->graph(1)->setPen(graphPen);
        ui->customPlot->graph(1)->setName("Mean height of fully grown plants");
        ui->customPlot->graph(1)->setData(getTimeVector(GROWN_PLANT_HEIGHT),
                                          getDataVector(GROWN_PLANT_HEIGHT));

        turnOnLegend();
        ui->customPlot->yAxis->setLabel("height");
        break;

    case 1: //Heaviest plants
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setName("Mass of heaviest plant");
        ui->customPlot->graph(0)->setData(getTimeVector(HEAVIEST_PLANT),
                                          getDataVector(HEAVIEST_PLANT));

        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkGreen);
        ui->customPlot->graph(1)->setPen(graphPen);
        ui->customPlot->graph(1)->setName("Mean mass of fully grown plants");
        ui->customPlot->graph(1)->setData(getTimeVector(GROWN_PLANT_MASS),
                                          getDataVector(GROWN_PLANT_MASS));

        turnOnLegend();
        ui->customPlot->yAxis->setLabel("mass");
        break;

    case 2: //Population density
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setData(getTimeVector(POPULATION),
                                          getDataVector(POPULATION));

        ui->customPlot->yAxis->setLabel("population density");
        ui->customPlot->legend->setVisible(false);
        break;

    case 3: //Number of seeds
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setData(getTimeVector(SEED_COUNT),
                                          getDataVector(SEED_COUNT));

        ui->customPlot->legend->setVisible(false);
        ui->customPlot->yAxis->setLabel("seeds");
        break;

    case 4: //Energy per seed
        ui->customPlot->addGraph();
        graphPen.setColor(Qt::darkRed);
        ui->customPlot->graph(0)->setPen(graphPen);
        ui->customPlot->graph(0)->setData(getTimeVector(ENERGY_PER_SEED),
                                          getDataVector(ENERGY_PER_SEED));

        ui->customPlot->legend->setVisible(false);
        ui->customPlot->yAxis->setLabel("energy");
        break;
    }

    setGraphRange();
}

void StatsAndHistoryDialog::turnOnLegend()
{
    // set the placement of the legend (index 0 in the axis rect's inset layout) to not be
    // border-aligned (default), but freely, so we can reposition it anywhere:
    ui->customPlot->axisRect()->insetLayout()->setInsetPlacement(0, QCPLayoutInset::ipFree);
    m_draggingLegend = false;
    ui->customPlot->legend->setVisible(true);
}



void StatsAndHistoryDialog::setGraphRange()
{
    ui->customPlot->rescaleAxes();

    ui->customPlot->xAxis->setRangeLower(0.0);
    ui->customPlot->yAxis->setRangeLower(0.0);

    //Add a bit of buffer so the data doesn't go all the way to the top edge.
    double yRangeMax = ui->customPlot->yAxis->range().upper * 1.05;
    ui->customPlot->yAxis->setRangeUpper(yRangeMax);
    ui->customPlot->xAxis->setRangeUpper(m_environment->getElapsedTime());

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
    header += "Mean height of fully grown plants,";
    header += "Mass of heaviest plant,";
    header += "Mean mass of fully grown plants,";
    header += "Number of seeds,";
    header += "Mean energy per seed,";
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
    body += QString::number(g_stats->m_meanHeightOfFullyGrownPlants[i]) + ",";
    body += QString::number(g_stats->m_heaviestPlantMass[i]) + ",";
    body += QString::number(g_stats->m_meanMassOfFullyGrownPlants[i]) + ",";
    body += QString::number(g_stats->m_meanSeedsPerPlant[i]) + ",";
    body += QString::number(g_stats->m_meanEnergyPerSeed[i]) + ",";
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
    case GROWN_PLANT_HEIGHT:
        statVector = &(g_stats->m_meanHeightOfFullyGrownPlants);
        break;
    case HEAVIEST_PLANT:
        statVector = &(g_stats->m_heaviestPlantMass);
        break;
    case GROWN_PLANT_MASS:
        statVector = &(g_stats->m_meanMassOfFullyGrownPlants);
        break;
    case SEED_COUNT:
        statVector = &(g_stats->m_meanSeedsPerPlant);
        break;
    case ENERGY_PER_SEED:
        statVector = &(g_stats->m_meanEnergyPerSeed);
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
    case GROWN_PLANT_HEIGHT:
        statVector = &(g_stats->m_meanHeightOfFullyGrownPlants);
        break;
    case HEAVIEST_PLANT:
        statVector = &(g_stats->m_heaviestPlantMass);
        break;
    case GROWN_PLANT_MASS:
        statVector = &(g_stats->m_meanMassOfFullyGrownPlants);
        break;
    case SEED_COUNT:
        statVector = &(g_stats->m_meanSeedsPerPlant);
        break;
    case ENERGY_PER_SEED:
        statVector = &(g_stats->m_meanEnergyPerSeed);
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
    QPointF mousePoint((event->pos().x()-ui->customPlot->axisRect()->left())/(double)ui->customPlot->axisRect()->width(),
                       (event->pos().y()-ui->customPlot->axisRect()->top())/(double)ui->customPlot->axisRect()->height());
    rect.moveTopLeft(mousePoint-m_dragLegendOrigin);
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
    QPointF mousePoint((event->pos().x()-ui->customPlot->axisRect()->left())/(double)ui->customPlot->axisRect()->width(),
                       (event->pos().y()-ui->customPlot->axisRect()->top())/(double)ui->customPlot->axisRect()->height());
    m_dragLegendOrigin = mousePoint-ui->customPlot->axisRect()->insetLayout()->insetRect(0).topLeft();
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
