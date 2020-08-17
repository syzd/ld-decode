/************************************************************************

    mainwindow.cpp

    ld-dropout-detect - LaserDisc dropout detection tools
    Copyright (C) 2020 Simon Inns

    This file is part of ld-decode-tools.

    ld-dropout-detect is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Defaults
    interactiveMode = false;
    reverse = false;
    currentFrameNumber = 1;
}

MainWindow::~MainWindow()
{
    // Might need to delete sourceLabel objects here?

    delete ui;
}

void MainWindow::configure(bool _interactiveMode, bool _reverse, QVector<QString> _inputFilenames)
{
    interactiveMode = _interactiveMode;
    reverse = _reverse;
    inputFilenames = _inputFilenames;
}

void MainWindow::quit()
{
    // Close any open sources
    detectionSources.close();

    if (!interactiveMode) {
        // Queue a quit ready for the exec loop if running non-interactive
        if (!interactiveMode) QMetaObject::invokeMethod(qApp, "quit", Qt::QueuedConnection);
    } else {
        qApp->quit();
    }
}

bool MainWindow::process()
{
    // Open the source files
    qInfo() << "Opening source files...";
    if (!detectionSources.open(inputFilenames)) {
        qInfo() << "Opening source TBC files failed.  Giving up!";
        qApp->quit();
        return false;
    }
    if (reverse) {
        qInfo() << "Source field order is reversed";
        detectionSources.setReverseFieldOrder();
    }

    // Set the current frame number
    currentFrameNumber = detectionSources.getMinimumVbiFrameNumber();

    // Are we running in interactive mode?
    if (interactiveMode) {
        // Clear any existing tabs
        ui->frameDisplayTabWidget->clear();

        // Generate a display widget for each source
        sourceLabel.resize(detectionSources.getNumberOfSources());

        // Add a tab for each available source
        for (qint32 i = 0; i < detectionSources.getNumberOfSources(); i++) {
            sourceLabel[i] = new QLabel;
            ui->frameDisplayTabWidget->addTab(sourceLabel[i], "Source #" + QString::number(i));

            // Add an image to the tab
            updateFrameViewer(i, currentFrameNumber);
        }

    } else {
        // Non-interactive mode
        quit();
    }

    return true;
}

// Update a source frame viewer with a frame image
void MainWindow::updateFrameViewer(qint32 sourceNumber, qint32 frameNumber)
{
    qDebug() << "Updated frame viewer for source" << sourceNumber << "frame number" << frameNumber;
    QImage frameImage = detectionSources.getDetectionSource(sourceNumber)->getFrameData(frameNumber);
    sourceLabel[sourceNumber]->setPixmap(QPixmap::fromImage(frameImage));
}
