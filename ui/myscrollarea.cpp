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


#include "myscrollarea.h"
#include <QScrollBar>

MyScrollArea::MyScrollArea(QWidget * parent) :
    QScrollArea(parent)
{

    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarsChanged()));
    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrollBarsChanged()));
    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(scrollBarsChanged()));
    connect(horizontalScrollBar(), SIGNAL(rangeChanged(int,int)), this, SLOT(scrollBarsChanged()));
}

void MyScrollArea::resizeEvent(QResizeEvent * event)
{
    QScrollArea::resizeEvent(event);
    emit changed();
}


void MyScrollArea::scrollBarsChanged()
{
    emit changed();
}

