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


#ifndef MUTATIONRATEVISUALAID_H
#define MUTATIONRATEVISUALAID_H

#include <QWidget>
#include <vector>
#include <QPainter>
#include <QPainterPath>

class MutationRateVisualAid : public QWidget
{
    Q_OBJECT
public:
    explicit MutationRateVisualAid(QWidget * parent, double mutationRate, double maxMutationRate);
    
    double m_mutationRate;
    double m_maxMutationRate;

protected:
    void paintEvent(QPaintEvent *);

private:
    int m_height;
    int m_width;
    int m_baseCount;
    int m_maxUVRays;
    std::vector<QPainterPath> m_rayLines;
    std::vector<QPainterPath> m_arrowHeads;

    void paintDNA(QPainter * painter);
    void paintUV(QPainter * painter);
    QPainterPath makeUVRay();
    QPainterPath makeUVArrowHead();
};

#endif // MUTATIONRATEVISUALAID_H
