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


#include "environmentwidget.h"
#include <vector>
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QPolygonF>
#include <QLineF>
#include <QRectF>
#include <QRect>
#include <QFont>
#include <QWheelEvent>
#include <QLinearGradient>
#include <QTransform>
#include <math.h>
#include "../plant/organism.h"
#include "../plant/plantpart.h"
#include "../lighting/lighting.h"
#include "../program/environment.h"
#include "../settings/simulationsettings.h"
#include "../settings/environmentsettings.h"
#include "../program/randomnumbers.h"
#include "../program/point2d.h"

EnvironmentWidget::EnvironmentWidget(QWidget * parent, Environment * environment) :
    QFrame(parent),
    m_environment(environment),
    m_highlightedOrganism(0)
{
    //Specify the frame's style.
    setFrameStyle(QFrame::StyledPanel);
    setLineWidth(1);

    //Specify the widget's size.
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setMinimumSize(m_environment->getWidth() * g_simulationSettings->zoom, m_environment->getHeight() * g_simulationSettings->zoom);

    makeClouds();
}




//This function paints the simulation to the screen.
void EnvironmentWidget::paintEvent(QPaintEvent * event)
{
    QPainter painter(this);

    //If the display is off, just fill the area with grey, display a message and quit.
    if (!(g_simulationSettings->displayOn))
    {
        painter.fillRect(0, 0, width(), height(), QColor(200, 200, 200));
        paintMessage(&painter);
    }

    //Paint everything about the simulation in a separate function.
    else
        paintSimulation(&painter);

    //Now call the super class's paintEvent to draw the frame around the edge.
    QFrame::paintEvent(event);
}



//If shadows is false, then shadows will not be drawn, whether or not they are
//on.  If it is true, they'll only be drawn if they are on.
QImage EnvironmentWidget::paintSimulationToImage(bool highQuality, bool shadows)
{
    //Temporarily set the zoom setting and turn the shadows off.  These settings will
    //be restored to their original state at the end of this function
    double originalZoom = g_simulationSettings->zoom;
    bool originalShadows = g_simulationSettings->shadowsDrawn;

    if (highQuality)
        g_simulationSettings->zoom = 1.0;
    else
        g_simulationSettings->zoom = 0.5;

    if (!shadows)
        g_simulationSettings->shadowsDrawn = false;

    QImage simulationImage(m_environment->getWidth() * g_simulationSettings->zoom,
                           m_environment->getHeight() * g_simulationSettings->zoom,
                           QImage::Format_RGB32);
    QPainter painter(&simulationImage);
    paintSimulation(&painter);


    g_simulationSettings->zoom = originalZoom;
    g_simulationSettings->shadowsDrawn = originalShadows;

    return simulationImage;
}



//This function paints the whole simulation using the given painter.  It is used both to
//paint the widget to the screen (when the painter paints to this widget) and to draw the
//simulation to a QImage (when the painter paints to a pixmap).
void EnvironmentWidget::paintSimulation(QPainter * painter)
{
    //Scale all painting to the current zoom level.
    painter->scale(g_simulationSettings->zoom, g_simulationSettings->zoom);


    //Fill the background with the sky.
    QLinearGradient skyGradient(QPointF(0, m_environment->getHeight()), QPointF(0,0));
    skyGradient.setColorAt(0, g_simulationSettings->getSunIntensityAdjustedSkyBottomColor(g_environmentSettings->m_currentValues.m_sunIntensity));
    skyGradient.setColorAt(1, g_simulationSettings->getSunIntensityAdjustedSkyTopColor(g_environmentSettings->m_currentValues.m_sunIntensity));
    painter->fillRect(g_visibleRect, skyGradient);


    //Antialias everything from now on.  PERHAPS MAKE THIS A SETTING TO HELP OUT SLOWER MACHINES?
    painter->setRenderHint(QPainter::Antialiasing, true);


    //Paint the clouds
    if (g_simulationSettings->cloudsOn)
        paintClouds(painter);


    //Draw the plants.
    const std::list<Organism *> * organisms = m_environment->getOrganismList();
    for (std::list<Organism *>::const_iterator i = organisms->begin(); i != organisms->end(); ++i)
        drawOrganism(painter, *i);


    //Draw the shadows.
    if (g_simulationSettings->shadowsDrawn)
    {
        //Create a pixmap of the darkness color.
        QPixmap darkness(m_environment->getWidth() * g_simulationSettings->zoom,
                         m_environment->getHeight() * g_simulationSettings->zoom);
        darkness.fill(g_simulationSettings->nightTimeColor);

        //Create a pixmap to hold the shadows.
        QPixmap shadows(m_environment->getWidth() * g_simulationSettings->zoom,
                        m_environment->getHeight() * g_simulationSettings->zoom);
        QColor shadowColor(Qt::black);
        shadowColor.setAlphaF(1.0 - (g_lighting->getSunIntensity() / g_environmentSettings->m_currentValues.m_sunIntensity));
        shadows.fill(shadowColor);

        //Paint the shadow polygons onto the shadows pixmap
        QPainter shadowPainter(&shadows);
        shadowPainter.scale(g_simulationSettings->zoom, g_simulationSettings->zoom);
        shadowPainter.setRenderHint(QPainter::Antialiasing, g_simulationSettings->shadowAntialiasing);
        shadowPainter.setPen(Qt::NoPen);
        shadowColor.setAlphaF(g_simulationSettings->leafAbsorbance);
        shadowPainter.setBrush(QBrush(shadowColor));
        std::vector<QPolygonF> shadowPolygons;
        createShadowPolygons(&shadowPolygons);
        for (std::vector<QPolygonF>::iterator i = shadowPolygons.begin(); i != shadowPolygons.end(); ++i)
            shadowPainter.drawPolygon(*i);

        //Paint the shadows pixmap onto the darkness pixmap to create a final
        //pixmap that can be drawn to the screen.  In this final pixmap, total
        //darkness (no available light) is shown as g_simulationSettings->nightTimeColor.
        QPainter combinationPainter(&darkness);
        combinationPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        combinationPainter.drawPixmap(0, 0, shadows);

        //Draw the shadows onto the screen.
        painter->scale(1.0 / g_simulationSettings->zoom, 1.0 / g_simulationSettings->zoom);
        painter->drawPixmap(0, 0, darkness);
    }
}



void EnvironmentWidget::makeClouds()
{
    m_clouds.clear();

    //Determine the maximum number of steps needed to make a low (i.e. slow)
    //cloud move all the way to the right side of the environment.
    double minimumCloudSpeed = Cloud::getCloudScale(g_simulationSettings->minimumCloudElevation) *
            g_simulationSettings->cloudSpeed;
    int steps = m_environment->getWidth() / minimumCloudSpeed;

    int cloudsToMake = g_simulationSettings->cloudDensity * steps;
    for (int i = 0; i < cloudsToMake; ++i)
    {
        int movementSteps = g_randomNumbers->getRandomInt(0, steps);
        createOneCloud(movementSteps);
    }
}



//This function creates up to one cloud on the left hand side of the environment.
//It may not actually make a cloud, because the new cloud's elevation might be
//above the environment's top edge.
void EnvironmentWidget::createOneCloud(int movementSteps)
{
    //Get the cloud's elevation.  Low values are more common than high values
    //to make the clouds denser towards the horizon (because they'll be smaller).
    double elevation = g_simulationSettings->minimumCloudElevation + g_randomNumbers->getRandomExponential(g_simulationSettings->cloudDistributionLambda);

    double xPositionOfLeftEdge = Cloud::getCloudScale(elevation) * (g_simulationSettings->cloudSpeed * movementSteps - 55.0); //55.0 is the unscaled width of a cloud, as defined by the shapes in its constructor

    //Only bother actually making the cloud if it will be visible on the screen.  Otherwise, it's a waste.
    if (elevation < m_environment->getHeight() && xPositionOfLeftEdge < m_environment->getWidth())
        m_clouds.push_back(Cloud(elevation, movementSteps));
}


void EnvironmentWidget::paintPlants(QPainter * painter)
{
    const std::list<Organism *> * organisms = m_environment->getOrganismList();
    for (std::list<Organism *>::const_iterator i = organisms->begin(); i != organisms->end(); ++i)
        drawOrganism(painter, *i);
}


void EnvironmentWidget::paintClouds(QPainter * painter)
{
    for (size_t i = 0; i < m_clouds.size(); ++i)
    {
        QRectF cloudBoundingRect = m_clouds[i].getBoundingRect(m_environment->getHeight());
        if (cloudBoundingRect.top() < g_visibleRect.bottom() &&
                cloudBoundingRect.bottom() > g_visibleRect.top() &&
                cloudBoundingRect.left() < g_visibleRect.right() &&
                cloudBoundingRect.right() > g_visibleRect.left())
        {
            m_clouds[i].paintCloud(painter, m_environment->getHeight());
        }
    }
}


void EnvironmentWidget::moveClouds()
{
    double environmentWidth = m_environment->getWidth();
    double environmentHeight = m_environment->getHeight();

    //Possibly create a new cloud off the left side of the screen.
    if (g_randomNumbers->chanceOfTrue(g_simulationSettings->cloudDensity))
        createOneCloud(0);


    //Move the existing clouds.
    for (std::vector<Cloud>::iterator i = m_clouds.begin(); i != m_clouds.end();)
    {
        i->moveCloud();

        if (i->getLeftEdge() > environmentWidth ||
                i->getBottomEdge() > environmentHeight)
            i = m_clouds.erase(i);
        else
            ++i;
    }
}





void EnvironmentWidget::drawOrganism(QPainter * painter, const Organism * organism)
{
    m_environment->drawOrganism(painter, organism, m_highlightedOrganism);
}




//This function prints the 'Click...' message for when the simulation is hidden.
void EnvironmentWidget::paintMessage(QPainter * painter)
{
    QFont messageFont;
    messageFont.setPixelSize(24);
    painter->setFont(messageFont);

    QRect textRect(0, 0, width(), height());

    painter->drawText(textRect, Qt::AlignCenter, "Click 'Show simulation'\nto see the plants.");
}





void EnvironmentWidget::wheelEvent(QWheelEvent * event)
{
    //If the control key is not held down, don't do anything special.  Just call the
    //normal wheelEvent function (which should make the screen scroll up and down).
    if (!(event->modifiers().testFlag(Qt::ControlModifier)) )
    {
        QFrame::wheelEvent(event);
        return;
    }

    //Change the zoom depending on which way the mouse wheel was moved.
    double newZoom = g_simulationSettings->zoom;
    if (event->delta() > 0)
        newZoom += g_simulationSettings->zoomStep;
    else if (event->delta() < 0)
        newZoom -= g_simulationSettings->zoomStep;

    //Keep newZoom in range.
    if (newZoom < g_simulationSettings->minZoom)
        newZoom = g_simulationSettings->minZoom;
    if (newZoom > g_simulationSettings->maxZoom)
        newZoom = g_simulationSettings->maxZoom;

    //Send a message to the main window.
    emit changeZoomLevel(newZoom * 100.0); //Multiply by 100 because mainWindow expects a percentage.
}

void EnvironmentWidget::mousePressEvent(QMouseEvent * event)
{
    if (event->buttons() == Qt::LeftButton)
        mousePressOrMove(event);
    else if (event->buttons() == Qt::RightButton)
    {
        m_lastMousePosition = event->globalPos();
        this->setCursor(Qt::OpenHandCursor);
    }
}
void EnvironmentWidget::mouseMoveEvent(QMouseEvent * event)
{
    if (event->buttons() == Qt::LeftButton)
        mousePressOrMove(event);
    else if (event->buttons() == Qt::RightButton)
    {
        if (event->globalPos() != m_lastMousePosition)
            emit mouseDrag(m_lastMousePosition - event->globalPos());
        m_lastMousePosition = event->globalPos();
    }
}

void EnvironmentWidget::mousePressOrMove(QMouseEvent * event)
{
    m_highlightedOrganism = 0;

    //Ask the environment which (if any) organism is under that point.
    const Organism * organismUnderPoint = m_environment->findOrganismUnderPoint(getPointFromMouseEvent(event));

    m_highlightedOrganism = organismUnderPoint;
    update();
}



//Note: I originally had this as mousePressEvent, but that created a very strange bug.  When
//the user closed the infomation dialog using the 'X' button (as opposed to the 'close' button),
//This widget was registering mousePressEvents even when the user clicked well outside the
//widget.  Changing to mouseReleaseEvent seems to avoid the problem, though I'm not sure why.
void EnvironmentWidget::mouseReleaseEvent(QMouseEvent * event)
{
    this->setCursor(Qt::ArrowCursor);

    if (event->button() != Qt::LeftButton)
        return;

    //Ask the environment which (if any) organism is under that point.
    Organism * organismUnderPoint = m_environment->findOrganismUnderPoint(getPointFromMouseEvent(event));

    //Use a signal to make the MainWindow display the info dialog.
    if (organismUnderPoint != 0)
    {
        if (!g_simulationSettings->advancedMode || g_simulationSettings->clickMode == INFO)
            emit showOrganismInfoDialog(organismUnderPoint);
        else if (g_simulationSettings->clickMode == KILL)
            emit killOrganism(organismUnderPoint);
        else if (g_simulationSettings->clickMode == HELP)
            emit helpOrganism(organismUnderPoint);
    }

    m_highlightedOrganism = 0;
    update();
}


Point2D EnvironmentWidget::getPointFromMouseEvent(QMouseEvent * event)
{
    return Point2D(event->x() / g_simulationSettings->zoom,
                   m_environment->getHeight() - event->y() / g_simulationSettings->zoom);
}



void EnvironmentWidget::createShadowPolygons(std::vector<QPolygonF> * shadowPolygons)
{
    std::vector<PlantPart *> leaves;
    m_environment->addLeavesToVector(&leaves);

    double rotationAngleRadians = (m_environment->getSunAngle() - 90) * 0.01745329251994329576923690768489;
    double sine = sin(rotationAngleRadians);
    double cosine = -1.0 * cos(rotationAngleRadians);

    double envHeight = m_environment->getHeight();
    double maxShadowLength = std::max(double(m_environment->getWidth()), envHeight);
    double xOffset = sine * maxShadowLength;
    double yOffset = cosine * maxShadowLength;

    for (int i = 0; i < int(leaves.size()); ++i)
    {
        QPolygonF shadow;
        shadow << QPointF(leaves[i]->getStart().m_x, envHeight - leaves[i]->getStart().m_y);
        shadow << QPointF(leaves[i]->getEnd().m_x, envHeight - leaves[i]->getEnd().m_y);
        shadow << QPointF(leaves[i]->getEnd().m_x + xOffset, envHeight - (leaves[i]->getEnd().m_y + yOffset));
        shadow << QPointF(leaves[i]->getStart().m_x + xOffset, envHeight - (leaves[i]->getStart().m_y + yOffset));
        shadow << QPointF(leaves[i]->getStart().m_x, envHeight - leaves[i]->getStart().m_y);

        shadowPolygons->push_back(shadow);
    }
}

