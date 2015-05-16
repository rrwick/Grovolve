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


#include "organisminfodialog.h"
#include "ui_organisminfodialog.h"
#include <QLocale>
#include <QFont>
#include "../plant/organism.h"
#include "../plant/genome.h"
#include "singleorganismwidget.h"
#include "../program/globals.h"

OrganismInfoDialog::OrganismInfoDialog(QWidget * parent, const Organism *organism,
                                       long long elapsedTime, bool advancedMode) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::OrganismInfoDialog),
    m_singleOrganismWidget(0)
{
    ui->setupUi(this);

    if (!advancedMode)
    {
        ui->extraInfoWidget->setVisible(false);
        this->setWindowTitle("Organism genome");
    }

    ui->genomeTextEdit->setFont(getMonospaceFont());
    ui->genomeTextEdit->setText(organism->getGenome()->outputAsString());

    ui->organismViewWidget->setOrganism(organism);
    ui->organismViewWidget->setHeightExtent(organism->getHighestDrawnPoint());
    ui->organismViewWidget->setRightExtent(organism->getRightmostDrawnPoint());
    ui->organismViewWidget->setLeftExtent(organism->getLeftmostDrawnPoint());

    ui->organismAgeLabel->setText(this->locale().toString(organism->getAge(elapsedTime)));
    ui->organismEnergyLabel->setText(formatDoubleForDisplay(organism->getEnergy(), 1, this->locale()));
    ui->organismHeightLabel->setText(formatDoubleForDisplay(organism->getHeight(), 1, this->locale()));
    ui->organismMassLabel->setText(formatDoubleForDisplay(organism->getMass(), 1, this->locale()));
    ui->organismGenerationLabel->setText(formatDoubleForDisplay(organism->getGeneration(), 1, this->locale()));
    ui->branchCountLabel->setText(this->locale().toString(organism->getBranchCount()));
    ui->leafCountLabel->setText(this->locale().toString(organism->getLeafCount()));
    ui->seedpodCountLabel->setText(this->locale().toString(organism->getSeedpodCount()));
    ui->totalPartsLabel->setText(this->locale().toString(organism->getPlantPartCount()));
    ui->maintenanceCostLabel->setText(formatDoubleForDisplay(organism->getMaintenanceCost(), 1, this->locale()));

    ui->photosynthesisEnergyLabel->setText(formatDoubleForDisplay(organism->getEnergyFromPhotosynthesis(), 1, this->locale()));
    ui->growthMaintenanceEnergyLabel->setText(formatDoubleForDisplay(organism->getEnergySpentOnGrowthAndMaintenance(), 1, this->locale()));
    ui->reproductionEnergyLabel->setText(formatDoubleForDisplay(organism->getEnergySpentOnReproduction(), 1, this->locale()));
}

OrganismInfoDialog::~OrganismInfoDialog()
{
    delete ui;
}
