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


#ifndef STARTINGGENOMEDIALOG_H
#define STARTINGGENOMEDIALOG_H

#include <QDialog>
#include "../plant/genome.h"
#include <QKeyEvent>

namespace Ui {
class StartingGenomeDialog;
}

class StartingGenomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StartingGenomeDialog(QWidget * parent, int genomeLength);
    ~StartingGenomeDialog();

    void setTextFromGenome(Genome genome);
    Genome getGenomeFromText();

private:
    Ui::StartingGenomeDialog * ui;
    int m_genomeLength;

private slots:
    void restoreDefault();
    void randomGenome();
};

#endif // STARTINGGENOMEDIALOG_H
