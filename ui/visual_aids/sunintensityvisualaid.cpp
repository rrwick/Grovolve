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


#include "sunintensityvisualaid.h"
#include <QPainter>
#include <QColor>
#include <math.h>

SunIntensityVisualAid::SunIntensityVisualAid(QWidget * parent, double intensity, double maxIntensity) :
    QWidget(parent), m_intensity(intensity), m_maxIntensity(maxIntensity)
{
    m_height = 60;
    m_width = 200;

    setFixedSize(m_width, m_height);
}



void SunIntensityVisualAid::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    QColor backgroundColor(Qt::yellow);
    double intensityFraction = pow(m_intensity / m_maxIntensity, 0.75);  //0.75 chosen for aesthetics of scaling.
    double hue = intensityFraction / 6.0;
    backgroundColor.setHslF(hue, 0.5, intensityFraction);

    painter.fillRect(0, 0, m_width, m_height, QBrush(backgroundColor));
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPixmap sunImage(":/icons/sun-256.png");
    int scaledDimensions = 254 * intensityFraction + 2;
    QPixmap scaledSunImage = sunImage.scaled(scaledDimensions, scaledDimensions, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    double topLeftX = 2.0 * m_width / 3.0 - scaledDimensions/2.0;
    double topLeftY = m_height / 2.0 - scaledDimensions/2.0;

    painter.drawPixmap(topLeftX, topLeftY, scaledSunImage);

}
