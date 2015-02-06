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


#ifndef ORGANISMINFODIALOG_H
#define ORGANISMINFODIALOG_H

#include <QDialog>
#include <QString>

class Environment;
class Organism;
class SingleOrganismWidget;

namespace Ui {
class OrganismInfoDialog;
}

class OrganismInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OrganismInfoDialog(QWidget * parent, const Organism * organism, long long elapsedTime, bool advancedMode);
    ~OrganismInfoDialog();

private:
    Ui::OrganismInfoDialog * ui;
    SingleOrganismWidget * m_singleOrganismWidget;

    void makeOneGraph();
    void makeThreeGraphs();
};

#endif // ORGANISMINFODIALOG_H
