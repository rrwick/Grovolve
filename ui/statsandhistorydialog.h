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


#ifndef ENVIRONMENTINFODIALOG_H
#define ENVIRONMENTINFODIALOG_H

#include <QDialog>
#include <QString>
#include <vector>
#include <QCheckBox>
#include "qcustomplot.h"
#include "../program/globals.h"

class Environment;

namespace Ui {
class StatsAndHistoryDialog;
}

class StatsAndHistoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatsAndHistoryDialog(QWidget * parent, const Environment * const environment);
    ~StatsAndHistoryDialog();

    int getDisplayedTab() const;
    int getDisplayedGraph() const;
    int getHistoryType() const;
    void setDisplayedTab(int tabIndex);
    void setDisplayedGraph(int graphIndex);
    void setHistoryType(int historyType);

private:
    Ui::StatsAndHistoryDialog * ui;
    const Environment * m_environment;
    int m_sizeOfEachTick;

    void setGraphRange();
    QString makeHistoryInfoCSVHeaderLine();
    QString makeHistoryInfoCSVBodyLine(int index);
    QVector<double> getDataVector(GraphData graphData);
    QVector<double> getTimeVector(GraphData graphData);
    QString getYAxisLabel(GraphData graphData);
    int roundHistoryValue(int valueToRound);

private slots:
    void graphChanged(int newGraphIndex);
    void xRangeChanged(const QCPRange &newRange, const QCPRange &oldRange);
    void outputHistoryInfoButtonPressed();
    void outputCurrentInfoButtonPressed();
    void setGenomeHistoryInfoText();
    void genomeHistoryChanged();
    void setOrganismWidgetRange();
    void genomeHistorySpinBoxChanged();
    void genomeHistorySpinBoxEditingFinished();
};

#endif // ENVIRONMENTINFODIALOG_H
