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


#include "quicksummarydialog.h"
#include "ui_quicksummarydialog.h"
#include "../program/globals.h"
#include <QPushButton>

QuickSummaryDialog::QuickSummaryDialog(QWidget *parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::QuickSummaryDialog)
{
    ui->setupUi(this);

    setFixedSize(sizeHint());

    ui->genomeTextEdit->setFont(getMonospaceFont());

    //Not sure why this is required... I thought this would be automatic.
    connect(ui->buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(close()));
}

QuickSummaryDialog::~QuickSummaryDialog()
{
    delete ui;
}
