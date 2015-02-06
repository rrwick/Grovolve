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


#ifndef GRAVITYVISUALAID_H
#define GRAVITYVISUALAID_H

#include <QWidget>

class GravityVisualAid : public QWidget
{
    Q_OBJECT
public:
    explicit GravityVisualAid(QWidget * parent, double gravity, double maxGravity);
    
    double m_gravity;
    double m_maxGravity;

protected:
    void paintEvent(QPaintEvent *);

private:
    int m_height;
    int m_width;
    
};

#endif // GRAVITYVISUALAID_H
