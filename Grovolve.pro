# Copyright 2015 Ryan Wick

# This file is part of Grovolve.

# Grovolve is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# Grovolve is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with Grovolve.  If not, see <http://www.gnu.org/licenses/>.


QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = Grovolve
TEMPLATE = app

target.path += /usr/local/bin
INSTALLS += target

SOURCES += \
    program/main.cpp\
    program/environment.cpp \
    program/globals.cpp \
    program/randomnumbers.cpp \
    program/stats.cpp \
    program/saverandloader.cpp \
    plant/genome.cpp \
    plant/organism.cpp \
    plant/plantpart.cpp \
    lighting/lighting.cpp \
    settings/simulationsettings.cpp \
    settings/environmentsettings.cpp \
    settings/environmentvalues.cpp \
    ui/infotextwidget.cpp \
    ui/mainwindow.cpp \
    ui/environmentwidget.cpp \
    ui/settingsdialog.cpp \
    ui/singleorganismwidget.cpp \
    ui/environmentdialog.cpp \
    ui/quicksummarydialog.cpp \
    ui/aboutdialog.cpp \
    ui/visual_aids/gravityvisualaid.cpp \
    ui/visual_aids/mutationratevisualaid.cpp \
    ui/visual_aids/sunintensityvisualaid.cpp \
    ui/organisminfodialog.cpp \
    ui/qcustomplot.cpp \
    ui/autosaveimagesdialog.cpp \
    ui/startinggenomedialog.cpp \
    ui/waitingdialog.cpp \
    ui/statsandhistorydialog.cpp \
    ui/cloud.cpp \
    ui/recoverautosavefilesdialog.cpp \
    ui/myscrollarea.cpp

HEADERS  += \
    program/environment.h \
    program/globals.h \
    program/randomnumbers.h \
    program/stats.h \
    program/saverandloader.h \
    program/point2d.h \
    plant/genome.h \
    plant/organism.h \
    plant/plantpart.h \
    plant/seed.h \
    lighting/lighting.h \
    lighting/lightingpoint.h \
    lighting/shadowpoint.h \
    settings/simulationsettings.h \
    settings/environmentsettings.h \
    settings/environmentvalues.h \
    ui/mainwindow.h \
    ui/infotextwidget.h \
    ui/environmentwidget.h \
    ui/settingsdialog.h \
    ui/singleorganismwidget.h \
    ui/environmentdialog.h \
    ui/quicksummarydialog.h \
    ui/aboutdialog.h \
    ui/visual_aids/gravityvisualaid.h \
    ui/visual_aids/mutationratevisualaid.h \
    ui/visual_aids/sunintensityvisualaid.h \
    ui/organisminfodialog.h \
    ui/qcustomplot.h \
    ui/autosaveimagesdialog.h \
    ui/startinggenomedialog.h \
    ui/waitingdialog.h \
    ui/statsandhistorydialog.h \
    ui/cloud.h \
    ui/recoverautosavefilesdialog.h \
    ui/myscrollarea.h

FORMS    += \
    ui/mainwindow.ui \
    ui/settingsdialog.ui \
    ui/environmentdialog.ui \
    ui/quicksummarydialog.ui \
    ui/aboutdialog.ui \
    ui/organisminfodialog.ui \
    ui/autosaveimagesdialog.ui \
    ui/startinggenomedialog.ui \
    ui/waitingdialog.ui \
    ui/statsandhistorydialog.ui \
    ui/recoverautosavefilesdialog.ui

INCLUDEPATH += ui

CONFIG += c++11

# Uncomment this line if building for Windows XP
#QMAKE_LFLAGS_WINDOWS = /SUBSYSTEM:WINDOWS,5.01

win32:INCLUDEPATH += $$PWD/../boost_1_55_0/
win32:INCLUDEPATH += $$PWD/../tbb43_20141204oss/include/
win32:RC_FILE = images/myapp.rc

# 64 bit libraries
win32:contains(QMAKE_TARGET.arch, x86_64):{
LIBS += -L$$PWD/../boost_1_55_0/lib64-msvc-12.0/
LIBS += -L$$PWD/../tbb43_20141204oss/lib/intel64/vc12/
LIBS += -L$$PWD/../tbb43_20141204oss/bin/intel64/vc12/
}
# 32 bit libraries
win32:!contains(QMAKE_TARGET.arch, x86_64):{
LIBS += -L$$PWD/../boost_1_55_0/lib32-msvc-12.0/
LIBS += -L$$PWD/../tbb43_20141204oss/lib/ia32/vc12/
LIBS += -L$$PWD/../tbb43_20141204oss/bin/ia32/vc12/
}

macx:INCLUDEPATH += /usr/local/include/
macx:LIBS += -L/usr/local/lib
macx:LIBS += -lboost_iostreams
macx:LIBS += -lboost_serialization
macx:QMAKE_CXXFLAGS_WARN_ON = -Wall -Wno-unused-parameter
macx:ICON = images/application.icns

unix:QMAKE_CXXFLAGS += -std=c++11
unix:INCLUDEPATH += /usr/include/
unix:LIBS += -L/usr/lib
unix:LIBS += -lboost_iostreams
unix:LIBS += -lboost_serialization
unix:LIBS += -ltbb



RESOURCES += \
    images/images.qrc
