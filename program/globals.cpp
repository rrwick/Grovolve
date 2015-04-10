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


#include "globals.h"
#include <QFontInfo>

SimulationSettings * g_simulationSettings;
EnvironmentSettings * g_environmentSettings;
RandomNumbers * g_randomNumbers;
Lighting * g_lighting;
Stats * g_stats;
QFont g_largeFont;
QFont g_extraLargeFont;

int g_organismsSavedOrLoaded;
int g_seedsSavedOrLoaded;
int g_historyOrganismsSavedOrLoaded;

QRectF g_visibleRect;


QString formatDoubleForDisplay(double num, double decimalPlacesToDisplay, QLocale locale)
{
    QString withCommas = locale.toString(num, 'f');

    QString final;
    bool pastDecimalPoint = false;
    int numbersPastDecimalPoint = 0;
    for (int i = 0; i < withCommas.length(); ++i)
    {
        final += withCommas[i];

        if (pastDecimalPoint)
            ++numbersPastDecimalPoint;

        if (numbersPastDecimalPoint >= decimalPlacesToDisplay)
            return final;

        if (withCommas[i] == locale.decimalPoint())
            pastDecimalPoint = true;
    }
    return final;
}

//http://stackoverflow.com/questions/18896933/qt-qfont-selection-of-a-monospace-font-doesnt-work
bool isFixedPitch(const QFont & font)
{
    const QFontInfo fi(font);
    return fi.fixedPitch();
}
QFont getMonospaceFont()
{
    QFont font("monospace");
    if (isFixedPitch(font))
        return font;
    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font))
        return font;
    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font))
        return font;
    font.setFamily("courier");
    return font;
}
