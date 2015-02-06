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


#include "startinggenomedialog.h"
#include "ui_startinggenomedialog.h"
#include "../settings/simulationsettings.h"
#include "../program/randomnumbers.h"
#include "../program/globals.h"
#include <QRegExp>

StartingGenomeDialog::StartingGenomeDialog(QWidget *parent, int genomeLength) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::StartingGenomeDialog),
    m_genomeLength(genomeLength)
{
    ui->setupUi(this);

    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(ui->restoreDefaultButton, SIGNAL(clicked()), this, SLOT(restoreDefault()));
    connect(ui->randomGenomeButton, SIGNAL(clicked()), this, SLOT(randomGenome()));
}

StartingGenomeDialog::~StartingGenomeDialog()
{
    delete ui;
}



void StartingGenomeDialog::setTextFromGenome(Genome genome)
{
    ui->genomeTextEdit->setPlainText(genome.outputAsString());
}



Genome StartingGenomeDialog::getGenomeFromText()
{
    Genome returnGenome;
    QString genomeString = ui->genomeTextEdit->toPlainText();
    genomeString = genomeString.toUpper();

    for (int i = 0; i < genomeString.length(); ++i)
    {
        QChar letter = genomeString.at(i);

        if (letter == 'A')
            returnGenome.addNucleotide(0);
        else if (letter == 'B')
            returnGenome.addNucleotide(1);
        else if (letter == 'C')
            returnGenome.addNucleotide(2);
        else if (letter == 'D')
            returnGenome.addNucleotide(3);
    }

    return returnGenome;
}



void StartingGenomeDialog::restoreDefault()
{
    SimulationSettings newSettings;
    ui->genomeTextEdit->setPlainText(newSettings.startingGenome.outputAsString());
}

void StartingGenomeDialog::randomGenome()
{
    QString randomGenome;

    for (int i = 0; i < m_genomeLength; ++i)
    {
        switch(g_randomNumbers->getRandomZeroToThree())
        {
        case 0: randomGenome += 'A'; break;
        case 1: randomGenome += 'B'; break;
        case 2: randomGenome += 'C'; break;
        case 3: randomGenome += 'D'; break;
        }
    }

    ui->genomeTextEdit->setPlainText(randomGenome);
}
