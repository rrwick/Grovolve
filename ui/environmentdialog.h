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


#ifndef ENVIRONMENTDIALOG_H
#define ENVIRONMENTDIALOG_H

#include <QDialog>
#include <QAbstractSpinBox>
#include "../program/globals.h"
#include "../settings/environmentvalues.h"

class SunIntensityVisualAid;
class GravityVisualAid;
class MutationRateVisualAid;

namespace Ui {
class EnvironmentDialog;
}

class EnvironmentDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit EnvironmentDialog(QWidget * parent, long long elapsedTime);
    ~EnvironmentDialog();

    void setWidgetsFromSettings(EnvironmentValues values);

private:
    Ui::EnvironmentDialog * ui;
    long long m_elapsedTime;
    SunIntensityVisualAid * m_sunIntensityVisualAid;
    GravityVisualAid * m_gravityVisualAid;
    MutationRateVisualAid * m_mutationRateVisualAid;
    EnvironmentValues m_valuesWhenDialogOpened;

    EnvironmentValues getValuesFromWidgets();
    void setupSliders();
    void enableOrDisableNextButton();
    void formatSpinBox(QAbstractSpinBox * spinBox, double newValue, double originalValue);
    void formatSpinBoxForLargerValue(QAbstractSpinBox * spinBox);
    void formatSpinBoxForSmallerValue(QAbstractSpinBox * spinBox);
    void formatSpinBoxForSameValue(QAbstractSpinBox * spinBox);
    void increaseSpinBoxSize(QAbstractSpinBox * spinBox);
    double sliderProgressFromSpinBoxProgress(double spinBoxProgress);
    double spinBoxProgressFromSliderProgress(double sliderProgress);
    double roundToThousandth(double numToRound);

private slots:
    void setSlidersFromSpinBoxes();
    void setSpinBoxesFromSliders();
    void sunIntensityChanged();
    void gravityChanged();
    void mutationRateChanged();
    void backButtonPressed();
    void nextButtonPressed();
    void finishButtonPressed();
    void immediatelyButtonPressed();
    void graduallyButtonPressed();
    void restoreDefaults();
};

#endif // ENVIRONMENTDIALOG_H
