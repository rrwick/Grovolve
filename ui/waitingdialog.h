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


#ifndef WAITINGDIALOG_H
#define WAITINGDIALOG_H

#include <QDialog>
#include <QString>
#include <QTimer>
#include "../program/globals.h"

namespace Ui {
class WaitingDialog;
}

class WaitingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitingDialog(QWidget * parent, QString message, bool showCounters, bool showHistoryCounter);
    ~WaitingDialog();

private:
    Ui::WaitingDialog * ui;
    QTimer m_updateTimer;

private slots:
    void updateLabels();
};

#endif // WAITINGDIALOG_H
