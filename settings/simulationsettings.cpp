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


#include "simulationsettings.h"
#include <QDir>
#include <QFileInfo>

SimulationSettings::SimulationSettings()
{
    //Weights and densities
    leafDensity = 1.0;
    branchDensity = 2.0;
    seedpodDensity = 1.0;

    //Length and growth rate
    leafLength = 50.0;
    growRateGeneRatio = 1.5;
    seedpodLengthScale = 10.0;
    minimumPlantPartLength = 1.0;
    minimumGrowthRate = 1.0;
    growthRandomness = 0.02;
    maxPlantPartsPerOrganism = 1000;
    randomDeathRate = 0.0005;

    //Details for helped organisms.
    helpedDeathRate = 0.25; //Helped organisms are 1/4 as likely to randomly die.
    helpedBonusEnergy = 1.5; //Helped organisms get 50% more energy from photosynthesis.

    //Energy costs
    organismMaintenanceCost = 1.0;
    plantPartMaintenanceCost = 0.0;
    leafMaintenanceCost = 0.1;
    branchMaintenanceCost = 0.03;
    seedpodMaintenanceCost = 0.1;
    leafGrowthCost = 0.8;
    branchGrowthCost = 0.2;
    seedpodGrowthCost = 1.0;
    seedCreationCost = 1.0;

    //Seed-related
    newSeedsPerTickPerSeedpod = 0.5;
    newOrganismsPerTickPerSeed = 0.001;
    maxSeedAge = 100;

    //Physics-related
    branchStrengthFactor = 30.0;
    branchStrengthScalingPower = 2.0;
    torqueScalingFactor = 0.005;
    branchWidthGrowthIncrement = 0.5;

    //Lighting related
    leafAbsorbance = 0.4;
    dayLength = 60;
    nightLength = 40;
    maximumSunIntensity = 200.0;

    //Genome related
    genomeLength = 5000;
    averageCrossoverLength = 500.0;
    allowLoops = false;
    maxChildrenPerBranch = 7;
    promoterLength = 5;

    //Population related
    startingOrganismEnergy = 1000.0;

    //Environment size
    targetPopulationSize = 750;
    startingEnvironmentHeight = 600;
    startingEnvironmentWidth = 800;
    environmentHeightGap = 100;
    environmentHeightCheckInterval = 10;
    environmentWidthCheckInterval = 10000;
    minimumEnvironmentWidth = 500;
    maximumEnvironmentWidth = 16000; //This value was chosen because the shadow pixmap can only be 32767 wide, and a max zoom of 2.0 will require a pixmap twice as wide as the environment.

    //Colors
    skyBottomColor = QColor(230, 235, 245);
    skyTopColor = QColor(156, 180, 247);
    leafColor = QColor(12, 180, 0);
    branchFillColor = QColor(148, 102, 50);
    branchColorVariation = 7;
    leafColorVariation = 15;
    seedpodColor = QColor(50, 40, 30);
    nightTimeColor = QColor(0, 0, 10, 233);
    rainColor = QColor(51, 76, 127, 180);
    organismBorderColor = QColor(20, 10, 0);
    organismHighlightColor = QColor(200, 10, 10);
    increaseValueColor = QColor(0, 0, 0);
    decreaseValueColor = QColor(200, 0, 0);
    cloudColor = QColor(240, 240, 240);
    helpedColor = QColor(255, 255, 0);

    //Display settings
    displayOn = true;
    shadowsDrawn = false;
    zoom = 1.0;
    minZoom = 0.1;
    maxZoom = 2.0;
    zoomStep = 0.05;
    shadowAntialiasing = false;
    cloudsOn = true;
    advancedMode = false;
    clickMode = INFO;
    helpedBorderThickness = 2.0;
    cloudDensity = 0.0075;
    cloudSpeed = 0.2;
    minimumCloudElevation = 10.0;
    cloudScalingPower = 0.7;
    cloudDistributionLambda = 0.001;

    //Logging settings
    statLoggingInterval = 1000;
    maxLogEntries = 1000;
    minGraphSpan = 100000.0;
    minNucleotideGraphSpan = 25;
    autoImageSave = false;
    rememberedPath = QDir::homePath();
    imageSavePath = "";
    imageSaveInterval = 100000;
    imageSaveHighQuality = true;
    autosaveInterval = 100000;

    //Drawing sizes - all cosmetic with no effect on simulation
    leafThickness = 9.0;
    branchLineThickness = 5.0;
    seedpodThickness = 3.0; //The thickness of the seedpod stalk.
    seedpodLengthToBulbRadius = 3.0;
    singleOrganismWidgetBorder = 15.0;

    //Progression related
    simulationUpdateInterval = 10;

    //Sun
    sunriseAngle = 10;
    sunsetAngle = 170;



    //Gene 1 - initial branch
    startingGenome.addNucleotide(3); startingGenome.addNucleotide(0); //Branch
    startingGenome.addNucleotide(1); //Angle reference to parent
    startingGenome.addNucleotide(3); startingGenome.addNucleotide(1); startingGenome.addNucleotide(2); startingGenome.addNucleotide(2); //Angle of 90
    startingGenome.addNucleotide(0); startingGenome.addNucleotide(3); startingGenome.addNucleotide(0); startingGenome.addNucleotide(2); //Growth rate of 0.5
    startingGenome.addNucleotide(0); startingGenome.addNucleotide(0); startingGenome.addNucleotide(2); startingGenome.addNucleotide(2); //Length of 10
    startingGenome.addNucleotide(3); startingGenome.addNucleotide(3); startingGenome.addNucleotide(2); startingGenome.addNucleotide(3); startingGenome.addNucleotide(3); //Child 1
    startingGenome.addNucleotide(3); startingGenome.addNucleotide(2); startingGenome.addNucleotide(1); startingGenome.addNucleotide(2); startingGenome.addNucleotide(3); //Child 2
    for (int i = 0; i < (maxChildrenPerBranch-2) * promoterLength; ++i) //Remaining empty promoters
        startingGenome.addNucleotide(0);

    //Space between genes one and two
    for (int i = 0; i < 100; ++i)
        startingGenome.addNucleotide(0);


    //Gene 2 - leaf
    startingGenome.addNucleotide(3); startingGenome.addNucleotide(3); startingGenome.addNucleotide(2); startingGenome.addNucleotide(3); startingGenome.addNucleotide(3);//promoter
    startingGenome.addNucleotide(1); startingGenome.addNucleotide(0); //Leaf
    startingGenome.addNucleotide(1); //Angle reference to parent
    startingGenome.addNucleotide(2); startingGenome.addNucleotide(0); startingGenome.addNucleotide(0); startingGenome.addNucleotide(0); //Angle of 0
    startingGenome.addNucleotide(0); startingGenome.addNucleotide(3); startingGenome.addNucleotide(0); startingGenome.addNucleotide(2); //Growth rate of 0.5

    //Space between genes two and three
    for (int i = 0; i < 100; ++i)
        startingGenome.addNucleotide(0);

    //Gene 3 - seedpod
    startingGenome.addNucleotide(3); startingGenome.addNucleotide(2); startingGenome.addNucleotide(1); startingGenome.addNucleotide(2); startingGenome.addNucleotide(3); //promoter
    startingGenome.addNucleotide(2); startingGenome.addNucleotide(0); //Seedpod
    startingGenome.addNucleotide(1); //Angle reference to parent
    startingGenome.addNucleotide(2); startingGenome.addNucleotide(0); startingGenome.addNucleotide(0); startingGenome.addNucleotide(0); //Angle of 0
    startingGenome.addNucleotide(0); startingGenome.addNucleotide(0); startingGenome.addNucleotide(2); startingGenome.addNucleotide(2); //Growth rate of 0.1
    startingGenome.addNucleotide(0); startingGenome.addNucleotide(3); startingGenome.addNucleotide(0); startingGenome.addNucleotide(2); //Length of 50

    //Fill up the rest
    while (startingGenome.getGenomeLength() < genomeLength)
        startingGenome.addNucleotide(0);
}






void SimulationSettings::rememberPath(QString path)
{
    rememberedPath = QFileInfo(path).absoluteDir().absolutePath();
}



QColor SimulationSettings::getSunIntensityAdjustedSkyTopColor(double sunIntensity)
{
    return adjustSkyColor(sunIntensity, QColor(40, 40, 60), skyTopColor, QColor(Qt::white));
}
QColor SimulationSettings::getSunIntensityAdjustedSkyBottomColor(double sunIntensity)
{
    return adjustSkyColor(sunIntensity, QColor(50, 50, 70), skyBottomColor, QColor(Qt::white));
}
QColor SimulationSettings::getSunIntensityCloudColor(double sunIntensity)
{
    return adjustSkyColor(sunIntensity, QColor(70, 70, 70), cloudColor, QColor(247, 247, 247));
}



QColor SimulationSettings::adjustSkyColor(double sunIntensity, QColor lowIntensityColor, QColor middleColor, QColor highIntensityColor)
{
    double middleColorIntensity = 75.0;

    //Lighten the color for high sun intensities.
    if (sunIntensity >= middleColorIntensity)
    {
        double lighter = (sunIntensity - middleColorIntensity) / (maximumSunIntensity - middleColorIntensity);
        return blendColors(middleColor, highIntensityColor, lighter);
    }

    //Darken the color for low sun intensities.
    else
    {
        double darker = sunIntensity / middleColorIntensity;
        return blendColors(lowIntensityColor, middleColor, darker);
    }
}

QColor SimulationSettings::blendColors(QColor a, QColor b, double p) const
{
    return QColor(int(weightedMean(a.red(), b.red(), p)),
                  int(weightedMean(a.green(), b.green(), p)),
                  int(weightedMean(a.blue(), b.blue(), p)));
}
