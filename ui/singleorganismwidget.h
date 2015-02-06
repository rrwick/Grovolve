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


#ifndef SINGLEORGANISMWIDGET_H
#define SINGLEORGANISMWIDGET_H

#include <QWidget>

class Organism;

class SingleOrganismWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SingleOrganismWidget(QWidget *parent);

    void setOrganism(const Organism * newOrganism) {m_organism = newOrganism;}
    void setHeightExtent(double newHeightExtent) {m_heightExtent = newHeightExtent;}
    void setRightExtent(double newRightExtent) {m_rightExtent = newRightExtent;}
    void setLeftExtent(double newLeftExtent) {m_leftExtent = newLeftExtent;}

private:
    const Organism * m_organism;
    double m_heightExtent;
    double m_rightExtent;
    double m_leftExtent;

protected:
    void paintEvent(QPaintEvent * event);
};

#endif // SINGLEORGANISMWIDGET_H
