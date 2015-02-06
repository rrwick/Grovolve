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


#ifndef SUNINTENSITYVISUALAID_H
#define SUNINTENSITYVISUALAID_H

#include <QWidget>

class SunIntensityVisualAid : public QWidget
{
    Q_OBJECT
public:
    explicit SunIntensityVisualAid(QWidget * parent, double intensity, double maxIntensity);

    double m_intensity;
    double m_maxIntensity;

protected:
    void paintEvent(QPaintEvent *);

private:
    int m_height;
    int m_width;
};

#endif // SUNINTENSITYVISUALAID_H
