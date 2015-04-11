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


#ifndef SIMULATIONSETTINGS_H
#define SIMULATIONSETTINGS_H

#include <QColor>
#include "../plant/genome.h"
#include "../program/globals.h"

#ifndef Q_MOC_RUN
#include "boost/archive/text_iarchive.hpp"
#include "boost/archive/text_oarchive.hpp"
#endif // Q_MOC_RUN
namespace boost {namespace serialization {class access;}}

class SimulationSettings
{
public:
    SimulationSettings();

    //Changable by a user
    double startingOrganismEnergy;
    Genome startingGenome;
    int targetPopulationSize;
    double organismMaintenanceCost;
    double plantPartMaintenanceCost; //Per PlantPart
    double leafMaintenanceCost; //Per unit length
    double branchMaintenanceCost; //Per unit area
    double seedpodMaintenanceCost; //Per unit length
    double leafGrowthCost; //Per unit length
    double branchGrowthCost; //Per unit area
    double seedpodGrowthCost; //Per unit length
    double seedCreationCost; //Per seed
    double leafLength; //The length of all fully-grown leaves.
    double growRateGeneRatio;
    double growthRandomness; //This setting adds randomness to lengths, angles and growth rates.  It is measured as a fraction (e.g. 0.05 = +/- 5% randomness)
    double leafDensity; //Weight per unit length
    double branchDensity; //Weight per square unit area
    double seedpodDensity; //Weight per unit length
    double newSeedsPerTickPerSeedpod;
    double newOrganismsPerTickPerSeed; //Controls the rate of new organism production
    int maxSeedAge;
    double branchStrengthFactor;
    double branchStrengthScalingPower;
    double leafAbsorbance;
    int genomeLength;
    double averageCrossoverLength;
    int maxChildrenPerBranch;
    bool allowLoops; //When true, a Branch can have a child with the same gene index as itself.
    double randomDeathRate;
    bool shadowAntialiasing;
    bool cloudsOn;
    int dayLength;
    int nightLength;
    int startingEnvironmentWidth;
    bool advancedMode; //Changed in main window
    ClickMode clickMode; //Changed in main window

    //Not changable by a user
    double seedpodLengthScale; //How many times smaller a seedpod is than a branch.  Included because very large seedpods are ugly.
    double minimumPlantPartLength; //Sets the minimum for the FINAL length of a PlantPart - the starting length can still be zero.
    double minimumGrowthRate;
    int maxPlantPartsPerOrganism;
    double sunriseAngle;
    double sunsetAngle;
    double torqueScalingFactor;
    double branchWidthGrowthIncrement;
    int startingEnvironmentHeight;
    int environmentHeightGap; //When the gap between the tallest plant and the environment height is less than this, the environment will increase in height.
    int environmentHeightCheckInterval; //The environment height isn't checked every tick, just at this interval.
    int environmentWidthCheckInterval; //The environment width isn't checked every tick, just at this interval.
    int minimumEnvironmentWidth;
    QColor skyBottomColor;
    QColor skyTopColor;
    QColor leafColor;
    QColor branchFillColor;
    QColor seedpodColor;
    QColor nightTimeColor;
    QColor rainColor;
    QColor organismBorderColor;
    QColor organismHighlightColor;
    QColor increaseValueColor;
    QColor decreaseValueColor;
    QColor cloudColor;
    QColor helpedColor;
    double helpedBorderThickness;
    int branchColorVariation;
    int leafColorVariation;
    bool displayOn;
    bool shadowsDrawn;
    double zoom;
    double minZoom;
    double maxZoom;
    double zoomStep;
    int statLoggingInterval;
    int maxLogEntries;
    double minGraphSpan;
    double minNucleotideGraphSpan;
    bool autoImageSave;
    QString rememberedPath;
    QString imageSavePath;
    int imageSaveInterval;
    bool imageSaveHighQuality;
    int autosaveInterval;
    double leafThickness;
    double branchLineThickness;
    double seedpodThickness; //Thickness of the seedpod stalk
    double seedpodLengthToBulbRadius;  //Ratio of seedpod length to the radius of its bulb
    double singleOrganismWidgetBorder; //When a single organism is viewed in the info dialog, this gives the space to its sides and above.
    int promoterLength;
    int simulationUpdateInterval;
    double maximumSunIntensity;
    double helpedDeathRate;
    double helpedBonusEnergy;
    double cloudDensity;
    double cloudSpeed;
    double minimumCloudElevation;
    double cloudScalingPower;
    double cloudDistributionLambda;

    void rememberPath(QString path);
    int getAverageNonStarvedAge() const {return int(0.5 + 1.0 / randomDeathRate);}
    int getLeafGeneLength() const {return 10;}
    int getSeedpodGeneLength() const {return 14;}
    int getBranchGeneLength() const {return 14 + promoterLength * maxChildrenPerBranch;}
    QColor getSunIntensityAdjustedSkyTopColor(double sunIntensity);
    QColor getSunIntensityAdjustedSkyBottomColor(double sunIntensity);
    QColor getSunIntensityCloudColor(double sunIntensity);


private:
    double weightedMean(double a, double b, double p) const {return a * (1-p) + b * p;}
    QColor blendColors(QColor a, QColor b, double p) const;
    QColor adjustSkyColor(double sunIntensity, QColor lowIntensityColor, QColor middleColor, QColor highIntensityColor);


    friend class boost::serialization::access;
    template<typename Archive>
    void serialize(Archive & ar, const unsigned)
    {
        ar & targetPopulationSize;
        ar & startingOrganismEnergy;
        ar & startingGenome;
        ar & organismMaintenanceCost;
        ar & plantPartMaintenanceCost;
        ar & leafMaintenanceCost;
        ar & branchMaintenanceCost;
        ar & seedpodMaintenanceCost;
        ar & leafGrowthCost;
        ar & branchGrowthCost;
        ar & seedpodGrowthCost;
        ar & seedCreationCost;
        ar & leafLength;
        ar & growRateGeneRatio;
        ar & growthRandomness;
        ar & leafDensity;
        ar & branchDensity;
        ar & seedpodDensity;
        ar & newSeedsPerTickPerSeedpod;
        ar & newOrganismsPerTickPerSeed;
        ar & maxSeedAge;
        ar & branchStrengthFactor;
        ar & branchStrengthScalingPower;
        ar & leafAbsorbance;
        ar & genomeLength;
        ar & averageCrossoverLength;
        ar & maxChildrenPerBranch;
        ar & allowLoops;
        ar & randomDeathRate;
        ar & shadowAntialiasing;
        ar & cloudsOn;
        ar & dayLength;
        ar & nightLength;
        ar & startingEnvironmentWidth;
        ar & advancedMode;
        ar & clickMode;
    }
};

#endif // SIMULATIONSETTINGS_H
