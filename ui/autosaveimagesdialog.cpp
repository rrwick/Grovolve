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


#include "autosaveimagesdialog.h"
#include "ui_autosaveimagesdialog.h"
#include "infotextwidget.h"
#include "../settings/simulationsettings.h"
#include "../program/globals.h"
#include <QFileDialog>

AutoSaveImagesDialog::AutoSaveImagesDialog(QWidget * parent) :
    QDialog(parent, Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
    ui(new Ui::AutoSaveImagesDialog)
{
    ui->setupUi(this);

    ui->topLabel->setFont(g_largeFont);

    if (g_simulationSettings->imageSavePath == "")
        m_imageSavePath = g_simulationSettings->rememberedPath;
    else
        m_imageSavePath = g_simulationSettings->imageSavePath;


    ui->imageSaveIntervalSpinBox->setValue(g_simulationSettings->imageSaveInterval);
    ui->highQualityRadioButton->setChecked(g_simulationSettings->imageSaveHighQuality);
    ui->lowQualityRadioButton->setChecked(!g_simulationSettings->imageSaveHighQuality);
    ui->saveLocationLabel->setText(m_imageSavePath);

    m_autoImageSaving = g_simulationSettings->autoImageSave;
    if (m_autoImageSaving)
        turnOn();
    else
        turnOff();


    QString saveLocationInfoText = "This is the file location on your computer where the images will be saved.<br><br>"
                                   "They will not be saved directly in this folder, but rather in a subdirectory "
                                   "named with the date and time of when the simulation was started.";
    ui->autoImageSaveSettingsGridLayout->addWidget(new InfoTextWidget(this, saveLocationInfoText), 0, 0);

    QString timeIntervalInfoText = "When the simulation clock reaches a multiple of this number, an image will be saved.<br><br>"
                                   "Small values give lots of frequent images, while large values give fewer infrequent images.";
    ui->autoImageSaveSettingsGridLayout->addWidget(new InfoTextWidget(this, timeIntervalInfoText), 3, 0);

    QString qualityInfoText = "High quality images are larger but will take up more space. Low quality images are smaller and take up less space.";
    ui->autoImageSaveSettingsGridLayout->addWidget(new InfoTextWidget(this, qualityInfoText), 4, 0);


    //Fix the dialog's size with a little bit of extra height in case a particularly
    //long file path is selected that takes multiple lines.
    setFixedWidth(sizeHint().width());
    setFixedHeight(sizeHint().height() + 40);

    connect(ui->onOffButton, SIGNAL(clicked()), this, SLOT(toggleOnOff()));
    connect(ui->changePathButton, SIGNAL(clicked()), this, SLOT(changePath()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(accept()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
}

AutoSaveImagesDialog::~AutoSaveImagesDialog()
{
    delete ui;
}


void AutoSaveImagesDialog::toggleOnOff()
{
    m_autoImageSaving = !m_autoImageSaving;

    if (m_autoImageSaving)
        turnOn();
    else
        turnOff();
}


void AutoSaveImagesDialog::turnOn()
{
    ui->autoImageSaveSettingsWidget->setEnabled(true);
    ui->topLabel->setText("Automatic image saving is on");
    ui->onOffButton->setText("Turn off");
}

void AutoSaveImagesDialog::turnOff()
{
    ui->autoImageSaveSettingsWidget->setEnabled(false);
    ui->topLabel->setText("Automatic image saving is off");
    ui->onOffButton->setText("Turn on");
}




void AutoSaveImagesDialog::changePath()
{
    QString newPath = QFileDialog::getExistingDirectory(this, "Directory in which to save images", m_imageSavePath);

    if (newPath != "")
    {
        m_imageSavePath = newPath;
        ui->saveLocationLabel->setText(m_imageSavePath);
    }
}




void AutoSaveImagesDialog::setSettingsFromWidgets()
{
    g_simulationSettings->autoImageSave = m_autoImageSaving;
    g_simulationSettings->imageSaveInterval = ui->imageSaveIntervalSpinBox->value();
    g_simulationSettings->imageSaveHighQuality = ui->highQualityRadioButton->isChecked();
    g_simulationSettings->imageSavePath = m_imageSavePath;
}



