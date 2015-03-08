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


#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "../settings/simulationsettings.h"
#include "startinggenomedialog.h"


SettingsDialog::SettingsDialog(QWidget * parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);

    ui->mainTitleLabel->setFont(g_extraLargeFont);
    ui->startingConditionsTitleLabel->setFont(g_largeFont);
    ui->energyCostsTitleLabel->setFont(g_largeFont);
    ui->growthTitleLabel->setFont(g_largeFont);
    ui->densitiesTitleLabel->setFont(g_largeFont);
    ui->seedsTitleLabel->setFont(g_largeFont);
    ui->physicsTitleLabel->setFont(g_largeFont);
    ui->lightingTitleLabel->setFont(g_largeFont);
    ui->genomeTitleLabel->setFont(g_largeFont);
    ui->populationTitleLabel->setFont(g_largeFont);
    ui->displayTitleLabel->setFont(g_largeFont);

    setInfoTexts();

    m_tempStartingGenome = g_simulationSettings->startingGenome;

    connect(ui->startingGenomeButton, SIGNAL(clicked()), this, SLOT(openStartingGenomeDialog()));

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->restoreDefaultsButton, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}



void SettingsDialog::setInfoTexts()
{
    ui->startingEnvironmentWidthInfoText->setInfoText("The environment will start out this wide, but it will automatically adjust "
                                                      "as the simulation progresses to keep the number of plants near the target "
                                                      "population size.");

    ui->startingOrganismEnergyInfoText->setInfoText("This is the amount of energy given to each plant in the starting population. "
                                                    "Subsequent plants (generation 2 and onward) have their energy determined "
                                                    "by how much energy their parents put into their seeds.");

    ui->startingGenomeInfoText->setInfoText("This is the genome for each plant in the starting population. "
                                            "The default starting genome codes for a simple plant with one branch, "
                                            "one leaf and one seedpod. "
                                            "If the number of letters in the starting genome does not match the "
                                            "'genome length' setting below, then the starting genome will either "
                                            "be truncated or extended.");

    ui->organismMaintenanceCostInfoText->setInfoText("This amount of energy is deducted from each plant for every tick "
                                                     "of the clock. It is the fixed cost (unrelated to size) of keeping "
                                                     "a plant alive for a tick.");

    ui->plantPartMaintenanceCostInfoText->setInfoText("This amount of energy is deducted from each plant for each of their plant parts "
                                                      "(leaf, branch or seedpod) for every tick of the clock. It is the "
                                                      "fixed cost of maintaining a plant part for a tick.");

    ui->leafMaintenanceCostInfoText->setInfoText("For each of a plant's leaves, this value, multiplied by the leaf's length, "
                                                 "determines the energy deducted from the plant for each tick of the clock. "
                                                 "It is the length-dependent cost of maintaining a leaf for a tick.");

    ui->branchMaintenanceCostInfoText->setInfoText("For each of a plant's branches, this value, multiplied by the branch's area, "
                                                   "determines the energy deducted from the plant for each tick of the clock. "
                                                   "It is the area-dependent cost of maintaining a branch for a tick.");

    ui->seedpodMaintenanceCostInfoText->setInfoText("For each of a plant's seedpods, this value, multiplied by the seedpod's length, "
                                                    "determines the energy deducted from the plant for each tick of the clock. "
                                                    "It is the length-dependent cost of maintaining a seedpod for a tick.");

    ui->leafGrowthCostInfoText->setInfoText("This is the energy deducted from a plant for each unit length of leaf "
                                            "that is grown.");

    ui->branchGrowthCostInfoText->setInfoText("This is the energy deducted from a plant for each unit area of branch "
                                              "that is grown.");

    ui->seedpodGrowthCostInfoText->setInfoText("This is the energy deducted from a plant for each unit length of seedpod "
                                               "that is grown.");

    ui->seedCreationCostInfoText->setInfoText("This is the energy deducted from a plant for each seed that its seedpods create. "
                                              "This is in addition to the energy that a plant puts into its seeds for the "
                                              "following generation.");

    ui->leafLengthInfoText->setInfoText("This is length of all fully-grown leaves in the simulation.");

    ui->growRateGeneRatioInfoText->setInfoText("This is the scaling factor for the growth rate of plant parts. A plant part's "
                                               "growth rate is determined by multiplying this value by the growth rate value in "
                                               "its gene.");

    ui->growthRandomnessInfoText->setInfoText("This is the randomness that is added to all aspects of a plant part's growth: "
                                              "length, rate and angle. If set to 0%, two plants with the exact same genome will "
                                              "grow exactly the same.");

    ui->leafDensityInfoText->setInfoText("This is the linear density of leaves. A leaf's mass is equal to its length times "
                                         "this value.");

    ui->branchDensityInfoText->setInfoText("This is the area density of branches. A branch's mass is equal to its area times "
                                           "this value.");

    ui->seedpodDensityInfoText->setInfoText("This is the linear density of seedpods. A seedpod's mass is equal to its "
                                            "length times this value.");

    ui->newSeedsPerTickPerSeedpodInfoText->setInfoText("This is how many seeds (on average) each seedpod produces in a "
                                                       "given tick of the simulation clock. For example, 0.5 means that on average, "
                                                       "each seedpod will produce one seed for every two ticks of the clock.");

    ui->newOrganismsPerTickPerSeedInfoText->setInfoText("This is how many organisms are created for each seed present in a "
                                                        "tick of the simulation clock. For example, if this value is 2.0% and there are "
                                                        "1000 seeds, then 20 organisms will be created.");

    ui->maxSeedAgeInfoText->setInfoText("This is how long seeds last, as measured in number of ticks of the simulation clock.");

    ui->branchStrengthScalingPowerInfoText->setInfoText("A branch's strength is calculated by raising its width to a power (this "
                                                        "setting) and then multiplying it by a constant factor (the previous setting). "
                                                        "When the load on a branch exceeds its strength, it will grow in width.");

    ui->branchStrengthFactorInfoText->setInfoText("A branch's strength is calculated by raising its width to a power (the next "
                                                  "setting) and then multiplying it by a constant factor (this setting). When "
                                                  "the load on a branch exceeds its strength, it will grow in width.");

    ui->leafAbsorbanceInfoText->setInfoText("This is the fraction of light absorbed by each leaf, the rest being transmitted "
                                            "through the leaf. A value of 100% means leaves absorb all light, whereas a value "
                                            "of 0% means that all light is transmitted through leaves.");

    ui->dayLengthInfoText->setInfoText("This is the duration (in ticks of the clock) for the part of the day when the sun "
                                       "is shining. During this period, the sun moves across the sky. The full length of "
                                       "the day/night cycle is the sum of this setting and the following setting.");

    ui->nightLengthInfoText->setInfoText("This is the duration (in ticks of the clock) for the part of the day when the sun "
                                         "is not shining. The full length of the day/night cycle is the sum of this setting "
                                         "and the previous setting.");

    ui->genomeLengthInfoText->setInfoText("This is the fixed number of letters in each plant's genome.");

    ui->averageCrossoverLengthInfoText->setInfoText("During sexual recombination of two parent genomes to make a child genome, "
                                                    "this is the average length of a genome segment between crossover events. "
                                                    "High values lead to less frequent crossover while low values lead to more "
                                                    "frequent crossover.");

    ui->maxChildrenPerBranchInfoText->setInfoText("This is the number of possible plant parts that can grow from each branch. "
                                                  "Higher values allow for denser trees, whereas lower values only allow for "
                                                  "sparser trees.");

    ui->allowLoopsInfoText->setInfoText("If this is set to 'yes', then a plant part can have a gene that is the same as the gene of one "
                                        "of the plant parts below it (between it and the root). This allows repetitive structures to form. "
                                        "If set to 'no', plant parts cannot have a gene that is the same as the gene of one "
                                        "of the plant parts below it.");

    ui->targetPopulationSizeInfoText->setInfoText("The environment will automatically get wider or narrower to keep the population "
                                                  "at approximately this size, compensating for changes in population "
                                                  "density.");

    ui->randomDeathRateInfoText->setInfoText("This is the chance that any given organism will die on any given tick of the clock. "
                                             "A low value allows for long-lived organisms, while a high value makes organisms tend "
                                             "to have shorter lives.");

    ui->shadowAntialiasingInfoText->setInfoText("Antialiasing shadows makes the shadows appear smoother and nicer, but at a "
                                                "significant performance cost.");

    ui->cloudsInfoText->setInfoText("Clouds in the sky are a purely aesthetic feature that has no effect on the simulation.");
}



//These functions either set a spin box to a value or set the value to the spin box.  Pointers to
//these functions will be passed to loadOrSaveSettingsToOrFromWidgets, so that one function can
//take care of both save and load functionality.
void setOneSettingFromWidget(double * setting, QDoubleSpinBox * widget, bool percentage) {*setting = widget->value() / (percentage ? 100.0 : 1.0);}
void setOneSettingFromWidget(int * setting, QSpinBox * widget) {*setting = widget->value();}
void setOneWidgetFromSetting(double * setting, QDoubleSpinBox * widget, bool percentage) {widget->setValue(*setting * (percentage ? 100.0 : 1.0));}
void setOneWidgetFromSetting(int * setting, QSpinBox * widget) {widget->setValue(*setting);}



void SettingsDialog::setWidgetsFromSettings()
{
    loadOrSaveSettingsToOrFromWidgets(true, g_simulationSettings);
}


void SettingsDialog::setSettingsFromWidgets()
{
    loadOrSaveSettingsToOrFromWidgets(false, g_simulationSettings);
}



void SettingsDialog::loadOrSaveSettingsToOrFromWidgets(bool setWidgets,
                                                       SimulationSettings * settings)
{
    void (*doubleFunctionPointer)(double *, QDoubleSpinBox *, bool);
    void (*intFunctionPointer)(int *, QSpinBox *);
    if (setWidgets)
    {
        doubleFunctionPointer = setOneWidgetFromSetting;
        intFunctionPointer = setOneWidgetFromSetting;
    }
    else
    {
        doubleFunctionPointer = setOneSettingFromWidget;
        intFunctionPointer = setOneSettingFromWidget;
    }


    doubleFunctionPointer(&settings->startingOrganismEnergy, ui->startingOrganismEnergySpinBox, false);
    intFunctionPointer(&settings->startingEnvironmentWidth, ui->startingEnvironmentWidthSpinBox);

    doubleFunctionPointer(&settings->organismMaintenanceCost, ui->organismMaintenanceCostSpinBox, false);
    doubleFunctionPointer(&settings->plantPartMaintenanceCost, ui->plantPartMaintenanceCostSpinBox, false);
    doubleFunctionPointer(&settings->leafMaintenanceCost, ui->leafMaintenanceCostSpinBox, false);
    doubleFunctionPointer(&settings->branchMaintenanceCost, ui->branchMaintenanceCostSpinBox, false);
    doubleFunctionPointer(&settings->seedpodMaintenanceCost, ui->seedpodMaintenanceCostSpinBox, false);
    doubleFunctionPointer(&settings->leafGrowthCost, ui->leafGrowthCostSpinBox, false);
    doubleFunctionPointer(&settings->branchGrowthCost, ui->branchGrowthCostSpinBox, false);
    doubleFunctionPointer(&settings->seedpodGrowthCost, ui->seedpodGrowthCostSpinBox, false);
    doubleFunctionPointer(&settings->seedCreationCost, ui->seedCreationCostSpinBox, false);

    doubleFunctionPointer(&settings->leafLength, ui->leafLengthSpinBox, false);
    doubleFunctionPointer(&settings->growRateGeneRatio, ui->growRateGeneRatioSpinBox, false);
    doubleFunctionPointer(&settings->growthRandomness, ui->growthRandomnessSpinBox, true);

    doubleFunctionPointer(&settings->leafDensity, ui->leafDensitySpinBox, false);
    doubleFunctionPointer(&settings->seedpodDensity, ui->seedpodDensitySpinBox, false);
    doubleFunctionPointer(&settings->branchDensity, ui->branchDensitySpinBox, false);

    doubleFunctionPointer(&settings->newSeedsPerTickPerSeedpod, ui->newSeedsPerTickPerSeedpodSpinBox, false);
    doubleFunctionPointer(&settings->newOrganismsPerTickPerSeed, ui->newOrganismsPerTickPerSeedSpinBox, true);
    intFunctionPointer(&settings->maxSeedAge, ui->maxSeedAgeSpinBox);

    doubleFunctionPointer(&settings->branchStrengthFactor, ui->branchStrengthFactorSpinBox, false);
    doubleFunctionPointer(&settings->branchStrengthScalingPower, ui->branchStrengthScalingPowerSpinBox, false);

    doubleFunctionPointer(&settings->leafAbsorbance, ui->leafAbsorbanceSpinBox, true);
    intFunctionPointer(&settings->dayLength, ui->dayLengthSpinBox);
    intFunctionPointer(&settings->nightLength, ui->nightLengthSpinBox);

    intFunctionPointer(&settings->genomeLength, ui->genomeLengthSpinBox);
    doubleFunctionPointer(&settings->averageCrossoverLength, ui->averageCrossoverLengthSpinBox, false);
    intFunctionPointer(&settings->maxChildrenPerBranch, ui->maxChildrenPerBranchSpinBox);

    intFunctionPointer(&settings->targetPopulationSize, ui->targetPopulationSizeSpinBox);
    doubleFunctionPointer(&settings->randomDeathRate, ui->randomDeathRateSpinBox, true);

    //A couple of settings are not in a spin box, so they
    //have to be done manually, not with those function pointers.
    if (setWidgets)
    {
        ui->allowLoopsOnButton->setChecked(settings->allowLoops);
        ui->allowLoopsOffButton->setChecked(!settings->allowLoops);

        ui->shadowAntialiasingOnRadioButton->setChecked(settings->shadowAntialiasing);
        ui->shadowAntialiasingOffRadioButton->setChecked(!settings->shadowAntialiasing);

        ui->cloudsOnRadioButton->setChecked(settings->cloudsOn);
        ui->cloudsOffRadioButton->setChecked(!settings->cloudsOn);

        m_tempStartingGenome = settings->startingGenome;

    }
    else
    {
        settings->allowLoops = ui->allowLoopsOnButton->isChecked();
        settings->startingGenome = m_tempStartingGenome;
        g_simulationSettings->shadowAntialiasing = ui->shadowAntialiasingOnRadioButton->isChecked();
        g_simulationSettings->cloudsOn = ui->cloudsOnRadioButton->isChecked();
    }
}



void SettingsDialog::restoreDefaults()
{
    SimulationSettings defaultSettings;
    loadOrSaveSettingsToOrFromWidgets(true, &defaultSettings);
}


void SettingsDialog::openStartingGenomeDialog()
{
    StartingGenomeDialog startingGenomeDialog(this, ui->genomeLengthSpinBox->value());
    startingGenomeDialog.setTextFromGenome(m_tempStartingGenome);

    if (startingGenomeDialog.exec()) //The user clicked OK
        m_tempStartingGenome = startingGenomeDialog.getGenomeFromText();
}
