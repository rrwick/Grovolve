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


#include "environmentdialog.h"
#include "ui_environmentdialog.h"
#include <QScrollBar>
#include <QHBoxLayout>
#include <QString>
#include <QLabel>
#include <QProgressBar>
#include <cmath>
#include "visual_aids/sunintensityvisualaid.h"
#include "visual_aids/gravityvisualaid.h"
#include "visual_aids/mutationratevisualaid.h"
#include "infotextwidget.h"
#include "../settings/environmentsettings.h"
#include "../settings/simulationsettings.h"
#include <QLineEdit>

EnvironmentDialog::EnvironmentDialog(QWidget *parent, long long elapsedTime) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::EnvironmentDialog),
    m_elapsedTime(elapsedTime)
{
    ui->setupUi(this);

    ui->currentStatusTitleLabel->setFont(g_extraLargeFont);
    ui->settingsTitleLabel->setFont(g_extraLargeFont);
    ui->pendingChangesTitleLabel->setFont(g_largeFont);
    ui->pendingChangesLabel->setFont(g_extraLargeFont);

    ui->sunIntensitySpinBox->setMaximum(g_simulationSettings->maximumSunIntensity);

    //Create each visual aid and add them to their frames.
    m_sunIntensityVisualAid = new SunIntensityVisualAid(this, ui->sunIntensitySpinBox->value(), ui->sunIntensitySpinBox->maximum());
    ui->sunIntensityFrame->layout()->addWidget(m_sunIntensityVisualAid);
    ui->sunIntensityFrame->setFixedSize(ui->sunIntensityFrame->sizeHint());
    m_gravityVisualAid = new GravityVisualAid(this, ui->gravitySpinBox->value(), ui->gravitySpinBox->maximum());
    ui->gravityFrame->layout()->addWidget(m_gravityVisualAid);
    ui->gravityFrame->setFixedSize(ui->gravityFrame->sizeHint());
    m_mutationRateVisualAid = new MutationRateVisualAid(this, ui->mutationRateSpinBox->value(), ui->mutationRateSpinBox->maximum());
    ui->mutationRateFrame->layout()->addWidget(m_mutationRateVisualAid);
    ui->mutationRateFrame->setFixedSize(ui->mutationRateFrame->sizeHint());

    //By fixing the spinboxes sizes, they won't change preferred size when they are bolded/unbolded.
    //A tiny bit of size is added so that they can fit bolded text well.
    increaseSpinBoxSize(ui->sunIntensitySpinBox);
    increaseSpinBoxSize(ui->gravitySpinBox);
    increaseSpinBoxSize(ui->mutationRateSpinBox);
    increaseSpinBoxSize(ui->timeForGradualChangeSpinBox);

    //Start with the gradual change widget disabled.  If the user selects the gradual change
    //radio button, it will enable.
    ui->timeForGradualChangeWidget->setEnabled(false);

    //By giving this label a minimum height based on the spin box that can show/hide,
    //we prevent the widgets from moving slightly when the user changes the radio buttons.
    ui->timeForChangesLabel->setFixedHeight(ui->timeForGradualChangeSpinBox->sizeHint().height() * 1.5);

    //Create the info text widgets, and insert them to the left of the label for each setting.
    ui->sunIntensityInfoText->setInfoText("The sun is the source of all energy for the plants. This setting controls how "
                                          "bright the sun is, and therefore how much energy is available.");
    ui->gravityInfoText->setInfoText("Plants support their weight by growing thicker branches. Increasing or decreasing "
                                     "the gravity makes the plants weigh more or less and therefore require more or less "
                                     "support.");
    ui->mutationRateInfoText->setInfoText("This is the chance that any given letter in the genome will randomly change when a "
                                          "seed is produced. A higher rate can allow for faster evolution but "
                                          "can also result in an abundance of deleterious mutations.");
    ui->immediatelyOrGraduallyInfoText->setInfoText("Changes can either occur immediately or gradually take effect over "
                                                    "a specified amount of time.<br><br>"
                                                    "Large immediate changes may lead to extinction, but those "
                                                    "same changes may not cause extinction if applied gradually.");
    ui->timeForGradualChangeInfoText->setInfoText("The time is measured by the simulation clock. "
                                                  "How long the change will take in real time depends on your computer's speed.");



    if (g_environmentSettings->isProgressionActive())
    {
        ui->currentStatusLabel->setText("The environment settings are gradually changing:");

        ui->changeProgressBar->setMinimum(g_environmentSettings->getStartingTime());
        ui->changeProgressBar->setMaximum(g_environmentSettings->getTargetTime());
        ui->changeProgressBar->setValue(elapsedTime);

        QString startingSettings = "<b>Starting settings:</b><br>";
        startingSettings += g_environmentSettings->getStartingValues().outputChanges(g_environmentSettings->getTargetValues(), true);
        ui->startingSettingsLabel->setText(startingSettings);

        QString targetSettings = "<b>Final settings:</b><br>";
        targetSettings += g_environmentSettings->getTargetValues().outputChanges(g_environmentSettings->getStartingValues(), true);
        ui->targetSettingsLabel->setText(targetSettings);

        QLocale addCommas(QLocale::English);
        QString timeRemainingNumber = addCommas.toString(g_environmentSettings->getTargetTime() - elapsedTime);
        ui->timeRemainingLabel->setText("Time remaining in gradual change: " + timeRemainingNumber);
    }
    else
    {
        ui->currentStatusLabel->setText("<html>The environment settings are constant (i.e. not gradually changing).");
        ui->progressBarWidget->setVisible(false);
    }

    ui->screen2Widget->setVisible(false);


    adjustSize();
    setMinimumSize(sizeHint());


    setWidgetsFromSettings(g_environmentSettings->m_currentValues);
    setupSliders();
    setSlidersFromSpinBoxes();
    m_valuesWhenDialogOpened = getValuesFromWidgets();

    connect(ui->sunIntensitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(sunIntensityChanged()));
    connect(ui->gravitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(gravityChanged()));
    connect(ui->mutationRateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(mutationRateChanged()));

    connect(ui->sunIntensitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(setSlidersFromSpinBoxes()));
    connect(ui->gravitySpinBox, SIGNAL(valueChanged(double)), this, SLOT(setSlidersFromSpinBoxes()));
    connect(ui->mutationRateSpinBox, SIGNAL(valueChanged(double)), this, SLOT(setSlidersFromSpinBoxes()));

    connect(ui->sunIntensitySlider, SIGNAL(valueChanged(int)), this, SLOT(setSpinBoxesFromSliders()));
    connect(ui->gravitySlider, SIGNAL(valueChanged(int)), this, SLOT(setSpinBoxesFromSliders()));
    connect(ui->mutationRateSlider, SIGNAL(valueChanged(int)), this, SLOT(setSpinBoxesFromSliders()));

    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->backButton, SIGNAL(clicked()), this, SLOT(backButtonPressed()));
    connect(ui->nextButton, SIGNAL(clicked()), this, SLOT(nextButtonPressed()));
    connect(ui->finishButton, SIGNAL(clicked()), this, SLOT(finishButtonPressed()));
    connect(ui->immediatelyButton, SIGNAL(clicked()), this, SLOT(immediatelyButtonPressed()));
    connect(ui->graduallyButton, SIGNAL(clicked()), this, SLOT(graduallyButtonPressed()));

    connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));

    sunIntensityChanged();
    gravityChanged();
    mutationRateChanged();
}



EnvironmentDialog::~EnvironmentDialog()
{
    delete ui;
}



//This button brings up screen 1.
void EnvironmentDialog::backButtonPressed()
{
    ui->screen1Widget->setVisible(true);
    ui->screen2Widget->setVisible(false);
    ui->backButton->setEnabled(false);
    ui->nextButton->setEnabled(true);
    ui->finishButton->setEnabled(false);
    ui->restoreDefaultsButton->setEnabled(true);
}

void EnvironmentDialog::nextButtonPressed()
{
    ui->screen1Widget->setVisible(false);
    ui->screen2Widget->setVisible(true);
    ui->backButton->setEnabled(true);
    ui->nextButton->setEnabled(false);
    ui->restoreDefaultsButton->setEnabled(false);

    if (ui->immediatelyButton->isChecked() || ui->graduallyButton->isChecked())
        ui->finishButton->setEnabled(true);

    EnvironmentValues pendingChanges = getValuesFromWidgets();
    QString pendingChangesString = m_valuesWhenDialogOpened.outputChanges(pendingChanges);
    ui->pendingChangesLabel->setText(pendingChangesString);
}

void EnvironmentDialog::finishButtonPressed()
{
    if (ui->immediatelyButton->isChecked())
        g_environmentSettings->setStaticValues(getValuesFromWidgets());
    else
    {
        long long targetTime = m_elapsedTime + ui->timeForGradualChangeSpinBox->value();
        g_environmentSettings->createProgression(m_elapsedTime, g_environmentSettings->m_currentValues,
                                                 targetTime, getValuesFromWidgets());
    }

    accept();
}



void EnvironmentDialog::immediatelyButtonPressed()
{
    ui->timeForGradualChangeWidget->setEnabled(false);
    ui->timeForGradualChangeSpinBox->findChild<QLineEdit*>()->deselect();  //On Windows, this wasn't deselecting on its own after using the mouse wheel, and it looked weird to have a disabled spin box with a selection.
    ui->finishButton->setEnabled(true);
}


void EnvironmentDialog::graduallyButtonPressed()
{
    ui->timeForGradualChangeWidget->setEnabled(true);
    ui->finishButton->setEnabled(true);
}





void EnvironmentDialog::setWidgetsFromSettings(EnvironmentValues values)
{
    ui->sunIntensitySpinBox->setValue(values.m_sunIntensity);
    ui->mutationRateSpinBox->setValue(values.m_mutationRate * 100.0); //Change to percentage
    ui->gravitySpinBox->setValue(values.m_gravity);
}



EnvironmentValues EnvironmentDialog::getValuesFromWidgets()
{
    EnvironmentValues returnValues;

    returnValues.m_sunIntensity = ui->sunIntensitySpinBox->value();
    returnValues.m_mutationRate = ui->mutationRateSpinBox->value() / 100.0; //Change from percentage
    returnValues.m_gravity = ui->gravitySpinBox->value();

    return returnValues;
}






void EnvironmentDialog::sunIntensityChanged()
{
    double newIntensity = ui->sunIntensitySpinBox->value();
    m_sunIntensityVisualAid->m_intensity = newIntensity;
    m_sunIntensityVisualAid->update();

    formatSpinBox(ui->sunIntensitySpinBox, newIntensity, m_valuesWhenDialogOpened.m_sunIntensity);

    enableOrDisableNextButton();
}



void EnvironmentDialog::gravityChanged()
{
    double newGravity = ui->gravitySpinBox->value();
    m_gravityVisualAid->m_gravity = newGravity;
    m_gravityVisualAid->update();

    formatSpinBox(ui->gravitySpinBox, newGravity, m_valuesWhenDialogOpened.m_gravity);

    enableOrDisableNextButton();
}


void EnvironmentDialog::mutationRateChanged()
{
    double newMutationRate = ui->mutationRateSpinBox->value();
    m_mutationRateVisualAid->m_mutationRate = newMutationRate;
    m_mutationRateVisualAid->update();

    formatSpinBox(ui->mutationRateSpinBox, newMutationRate / 100.0, m_valuesWhenDialogOpened.m_mutationRate);

    enableOrDisableNextButton();
}





void EnvironmentDialog::enableOrDisableNextButton()
{
    if (getValuesFromWidgets() == m_valuesWhenDialogOpened)
        ui->nextButton->setEnabled(false);
    else
        ui->nextButton->setEnabled(true);
}


void EnvironmentDialog::formatSpinBox(QAbstractSpinBox * spinBox, double newValue, double originalValue)
{
    if (newValue > originalValue)
        formatSpinBoxForLargerValue(spinBox);
    else if (newValue < originalValue)
        formatSpinBoxForSmallerValue(spinBox);
    else
        formatSpinBoxForSameValue(spinBox);
}

void EnvironmentDialog::formatSpinBoxForLargerValue(QAbstractSpinBox * spinBox)
{
    QFont boldFont;
    boldFont.setBold(true);
    spinBox->setFont(boldFont);
    spinBox->setStyleSheet("color: " + g_simulationSettings->increaseValueColor.name());
}

void EnvironmentDialog::formatSpinBoxForSmallerValue(QAbstractSpinBox * spinBox)
{
    QFont boldFont;
    boldFont.setBold(true);
    spinBox->setFont(boldFont);
    spinBox->setStyleSheet("color: " + g_simulationSettings->decreaseValueColor.name());
}

void EnvironmentDialog::formatSpinBoxForSameValue(QAbstractSpinBox * spinBox)
{
    QFont normalFont;
    spinBox->setFont(normalFont);
    spinBox->setStyleSheet("");
}


void EnvironmentDialog::increaseSpinBoxSize(QAbstractSpinBox * spinBox)
{
    spinBox->setFixedSize(spinBox->sizeHint().width() + 2, spinBox->sizeHint().height() + 2);
}


void EnvironmentDialog::setupSliders()
{
    ui->sunIntensitySlider->setMinimum(int(ui->sunIntensitySpinBox->minimum() * 10));
    ui->sunIntensitySlider->setMaximum(int(ui->sunIntensitySpinBox->maximum() * 10));

    ui->gravitySlider->setMinimum(int(ui->gravitySpinBox->minimum() * 100));
    ui->gravitySlider->setMaximum(int(ui->gravitySpinBox->maximum() * 100));

    //The mutation rate slider is not linear, so it does not directly correspond to the
    //spin box values.
    ui->mutationRateSlider->setMinimum(0);
    ui->mutationRateSlider->setMaximum(10000);
}


void EnvironmentDialog::setSlidersFromSpinBoxes()
{
    ui->sunIntensitySlider->setValue(int(ui->sunIntensitySpinBox->value() * 10 + 0.5));
    ui->gravitySlider->setValue(int(ui->gravitySpinBox->value() * 100 + 0.5));

    double mutationRateProgress = ui->mutationRateSpinBox->value() / ui->mutationRateSpinBox->maximum();
    double mutationRateSliderProgress = sliderProgressFromSpinBoxProgress(mutationRateProgress);
    ui->mutationRateSlider->setValue(int(mutationRateSliderProgress * 10000.0 + 0.5));
}

void EnvironmentDialog::setSpinBoxesFromSliders()
{
    double newSunIntensity = double(ui->sunIntensitySlider->value()) / 10.0;
    if (ui->sunIntensitySpinBox->value() != newSunIntensity)
        ui->sunIntensitySpinBox->setValue(newSunIntensity);

    double newGravity = double(ui->gravitySlider->value()) / 100.0;
    if (ui->gravitySpinBox->value() != newGravity)
        ui->gravitySpinBox->setValue(newGravity);

    double newMutationRateSliderProgress = double(ui->mutationRateSlider->value()) / 10000.0;
    double newMutationRateProgress = spinBoxProgressFromSliderProgress(newMutationRateSliderProgress);
    double newMutationRate = newMutationRateProgress * ui->mutationRateSpinBox->maximum();

    //If the values round to the same number (to 0.001, the precision of the spin box), then
    //do not set the spin box value.  This prevents the value from being set while the user
    //is typing a number in.
    if (roundToThousandth(ui->mutationRateSpinBox->value()) != roundToThousandth(newMutationRate))
        ui->mutationRateSpinBox->setValue(newMutationRate);
}



//These two functions are inverses of each other.  They were chosen so that the mutation
//rate slider has the right feel to it, covering a wide range of values.
double EnvironmentDialog::spinBoxProgressFromSliderProgress(double sliderProgress)
{
    return 0.9 * pow(sliderProgress, 3.0) + 0.1 * sliderProgress;
}
double EnvironmentDialog::sliderProgressFromSpinBoxProgress(double spinBoxProgress)
{
    double a = (405.0 * spinBoxProgress) + 3.0 * pow(18225 * spinBoxProgress * spinBoxProgress + 3, 0.5);
    return (pow(a, 2.0/3.0) - 3) / (9 * pow(a, 1.0/3.0));
}

double EnvironmentDialog::roundToThousandth(double numToRound)
{
    return int(numToRound * 1000.0 + 0.5);
}



void EnvironmentDialog::restoreDefaults()
{
    EnvironmentValues defaultSettings;
    setWidgetsFromSettings(defaultSettings);
}
