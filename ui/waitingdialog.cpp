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


#include "waitingdialog.h"
#include "ui_waitingdialog.h"
#include "../program/globals.h"

WaitingDialog::WaitingDialog(QWidget * parent, QString message, bool showCounters, bool showHistoryCounter) :
    QDialog(parent, Qt::FramelessWindowHint),
    ui(new Ui::WaitingDialog)
{
    ui->setupUi(this);

    //Enlarge one of the number-holding labels so the UI
    //elements don't shift around as the numbers grow.
    ui->organismLabel2->setText("                ");
    ui->organismLabel2->setMinimumWidth(ui->organismLabel2->sizeHint().width());

    if (!showCounters)
        ui->countersWidget->setVisible(false);
    if (!showHistoryCounter)
    {
        ui->historyOrganismLabel1->setVisible(false);
        ui->historyOrganismLabel2->setVisible(false);
    }

    ui->messageLabel->setFont(g_extraLargeFont);
    ui->messageLabel->setText(message);
    setWindowTitle(message);
    setFixedSize(sizeHint());

    g_organismsSavedOrLoaded = 0;
    g_seedsSavedOrLoaded = 0;
    g_historyOrganismsSavedOrLoaded = 0;

    connect(&m_updateTimer, SIGNAL(timeout()), this, SLOT(updateLabels()));

    m_updateTimer.start(40);
}

WaitingDialog::~WaitingDialog()
{
    delete ui;
}

void WaitingDialog::updateLabels()
{
    ui->organismLabel2->setText(this->locale().toString(g_organismsSavedOrLoaded));
    ui->seedLabel2->setText(this->locale().toString(g_seedsSavedOrLoaded));
    ui->historyOrganismLabel2->setText(this->locale().toString(g_historyOrganismsSavedOrLoaded));
}
