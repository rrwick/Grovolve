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


#include "singleorganismwidget.h"
#include <QPainter>
#include <QTransform>
#include <QLinearGradient>
#include "../plant/organism.h"
#include "../settings/simulationsettings.h"
#include "../program/environment.h"

SingleOrganismWidget::SingleOrganismWidget(QWidget * parent) :
    QWidget(parent), m_organism(0)
{
    //Prevent the widget from getting too narrow.  This prevents the organism from
    //being drawn excessively small or even inverted (if it has a negative space in
    //which to be drawn).
    setMinimumWidth(g_simulationSettings->singleOrganismWidgetBorder * 5);
}



void SingleOrganismWidget::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);

    //Fill the background with the sky color.
    QLinearGradient skyGradient(QPointF(0, height()), QPointF(0,0));
    skyGradient.setColorAt(0, g_simulationSettings->skyBottomColor);
    skyGradient.setColorAt(1, g_simulationSettings->skyTopColor);
    painter.fillRect(0, 0, width(), height(), skyGradient);

    if (m_organism == 0)
        return;

    painter.setRenderHint(QPainter::Antialiasing, true);

    //Set up the necessary transformation such that the organism is displayed in its entirity
    //in this widget.
    QTransform transform;
    double availableWidth = width() - 2 * g_simulationSettings->singleOrganismWidgetBorder;
    double availableHeight = height() - g_simulationSettings->singleOrganismWidgetBorder;

    double organismWidth = m_rightExtent - m_leftExtent;
    double horizontalScaleToFit = availableWidth / organismWidth;
    double verticalScaleToFit = availableHeight / m_heightExtent;

    double finalScale;
    if (horizontalScaleToFit < verticalScaleToFit)
        //In this case, the drawing is limited by the available width.
        //Height at the top of the widget will go unused.
        finalScale = horizontalScaleToFit;
    else
        //In this case, the drawing is limited by the available height.
        //Width on both sides of the widget will go usused.
        finalScale = verticalScaleToFit;

    double requiredWidth = organismWidth * finalScale;
    double excessWidth = width() - requiredWidth;
    double shiftForHorizontalCentering = excessWidth / 2.0;

    double rightShift = -1.0 * m_leftExtent * finalScale + shiftForHorizontalCentering;
    double downShift = height();

    transform.translate(rightShift, downShift);
    transform.scale(finalScale, finalScale);

    painter.setTransform(transform);

    //Paint the organism.
    m_organism->drawOrganism(&painter, 0.0);

}
