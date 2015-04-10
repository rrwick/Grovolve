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


#ifndef ENVIRONMENTWIDGET_H
#define ENVIRONMENTWIDGET_H

#include <QFrame>
#include <QPolygonF>
#include <QLineF>
#include <QImage>
#include <QMouseEvent>
#include <QPainterPath>
#include <QPoint>
#include <vector>
#include "../program/globals.h"
#include "../program/point2d.h"
#include "cloud.h"

class Environment;
class Lighting;
class Settings;
class QPainter;
class Organism;

class EnvironmentWidget : public QFrame
{
    Q_OBJECT
public:
    explicit EnvironmentWidget(QWidget * parent, Environment * environment);

    QImage paintSimulationToImage(bool highQuality, bool shadows);

    void makeClouds();
    void moveClouds();

protected:
    void paintEvent(QPaintEvent * event);
    void wheelEvent(QWheelEvent * event);
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent * event);

private:
    Environment * const m_environment;
    std::vector<Cloud> m_clouds;
    const Organism * m_highlightedOrganism;
    QPoint m_lastMousePosition;
    QRectF m_visibleRect;

    void paintSimulation(QPainter * painter);
    void paintMessage(QPainter * painter);
    void drawOrganism(QPainter * painter, const Organism *organism);
    void createShadowPolygons(std::vector<QPolygonF> * shadowPolygons);
    void paintPlants(QPainter * painter);
    void paintClouds(QPainter * painter);
    void createOneCloud(int movementSteps);
    void mousePressOrMove(QMouseEvent * event);
    Point2D getPointFromMouseEvent(QMouseEvent * event);

signals:
    void changeZoomLevel(double newZoomLevel);
    void showOrganismInfoDialog(const Organism * organism);
    void killOrganism(Organism * organism);
    void helpOrganism(Organism * organism);
    void mouseDrag(QPoint change);
};

#endif // ENVIRONMENTWIDGET_H
