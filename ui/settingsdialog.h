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


#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include "../program/globals.h"
#include "infotextwidget.h"
#include "../plant/genome.h"

namespace Ui {
class SettingsDialog;
}

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = 0);
    ~SettingsDialog();

    void setWidgetsFromSettings();
    void setSettingsFromWidgets();

private:
    Ui::SettingsDialog * ui;
    Genome m_tempStartingGenome;

    void setInfoTexts();
    void loadOrSaveSettingsToOrFromWidgets(bool setWidgets,
                                           SimulationSettings *settings);

private slots:
    void restoreDefaults();
    void openStartingGenomeDialog();
};

#endif // SETTINGSDIALOG_H
