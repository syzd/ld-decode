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

    initialiseGui();
}

MainWindow::~MainWindow()
{
    // Might need to delete sourceLabel objects here?

    delete ui;
}

void MainWindow::initialiseGui()
{
    // Defaults
    interactiveMode = false;
    reverse = false;
    currentVbiFrameNumber = 1;
    sourcesReady = false;

    // Allow the next and previous frame buttons to auto-repeat
    ui->previousFramePushButton->setAutoRepeat(true);
    ui->previousFramePushButton->setAutoRepeatDelay(500);
    ui->previousFramePushButton->setAutoRepeatInterval(1);
    ui->nextFramePushButton->setAutoRepeat(true);
    ui->nextFramePushButton->setAutoRepeatDelay(500);
    ui->nextFramePushButton->setAutoRepeatInterval(1);

    // Set up overlay mode combobox
    ui->overlayComboBox->addItem("None");
    ui->overlayComboBox->addItem("Current dropouts");
    ui->overlayComboBox->addItem("Clip detector");
    // Single source clip analysis
    // luma/chroma diff
    // luma diff
    // chroma diff
    // others?
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
    currentVbiFrameNumber = detectionSources.getMinimumVbiFrameNumber();
    qDebug() << "Initial current frame number is" << currentVbiFrameNumber;

    // Are we running in interactive mode?
    if (interactiveMode) {
        // Clear any existing tabs
        ui->frameDisplayTabWidget->clear();

        // Generate a display widget for each source
        sourceLabel.resize(detectionSources.getTotalNumberOfSources());

        // Add a tab for each available source
        for (qint32 i = 0; i < detectionSources.getTotalNumberOfSources(); i++) {
            sourceLabel[i] = new QLabel;
            ui->frameDisplayTabWidget->addTab(sourceLabel[i], "Source #" + QString::number(i));
        }

        // Set the GUI widget frame ranges
        ui->frameNumberSpinBox->setRange(detectionSources.getMinimumVbiFrameNumber(), detectionSources.getMaximumVbiFrameNumber());
        ui->frameNumberSpinBox->setValue(currentVbiFrameNumber);
        ui->frameNumberHorizontalSlider->setRange(detectionSources.getMinimumVbiFrameNumber(), detectionSources.getMaximumVbiFrameNumber());
        ui->frameNumberHorizontalSlider->setValue(currentVbiFrameNumber);

        // Add an image to the current source tab
        sourcesReady = true;
        updateFrameViewer();
    } else {
        // Non-interactive mode
        quit();
    }

    return true;
}

// Update a source frame viewer with a frame image
void MainWindow::updateFrameViewer()
{
    qint32 sourceNumber = ui->frameDisplayTabWidget->currentIndex();

    if (sourceNumber >= 0) {
        qDebug() << "Updating frame viewer for source" << sourceNumber << "VBI frame number" << currentVbiFrameNumber;
        QImage frameImage = detectionSources.getDetectionSource(sourceNumber)->getFrameImage(currentVbiFrameNumber);

        // Only highlight the frame image if the frame is available from the source
        if (detectionSources.getDetectionSource(sourceNumber)->isFrameAvailable(currentVbiFrameNumber)) {
            // Get the frame dropout data
            Dropouts dropouts;

            if (ui->overlayComboBox->currentText() == "Current dropouts") {
                // Show current dropouts (without any additional detection)
                // Don't need to do anything here
                 dropouts = detectionSources.getDetectionSource(sourceNumber)->getFrameDropouts(currentVbiFrameNumber);
            } else if (ui->overlayComboBox->currentText() == "Clip detector") {
                // Get dropouts from clip detector
                ClipDetector clipDetector;
                dropouts = clipDetector.process(detectionSources.getDetectionSource(sourceNumber)->getFrameData(currentVbiFrameNumber),
                                                detectionSources.getDetectionSource(sourceNumber)->getVideoParameters());
            }

            // Show the dropouts in debug
            qDebug() << "Dropouts:" << dropouts;

            // Highlight dropout data on the image
            if (ui->overlayComboBox->currentText() != "None") {
                // Create a painter object
                QPainter imagePainter;
                imagePainter.begin(&frameImage);

                // Draw the drop out data for the frame
                imagePainter.setPen(Qt::red);
                for (qint32 dropoutIndex = 0; dropoutIndex < dropouts.size(); dropoutIndex++) {
                    imagePainter.drawLine(dropouts.startx()[dropoutIndex], dropouts.frameLine()[dropoutIndex] - 1,
                                          dropouts.endx()[dropoutIndex], dropouts.frameLine()[dropoutIndex] - 1);
                }

                // End the painter object
                imagePainter.end();
            }
        }

        // Display the QImage in the GUI
        sourceLabel[sourceNumber]->setPixmap(QPixmap::fromImage(frameImage));
    }
}

// GUI action slots ---------------------------------------------------------------------------------------------------

void MainWindow::on_frameNumberSpinBox_editingFinished()
{
    currentVbiFrameNumber = ui->frameNumberSpinBox->value();
    ui->frameNumberHorizontalSlider->setValue(currentVbiFrameNumber);
}

void MainWindow::on_previousFramePushButton_clicked()
{
    currentVbiFrameNumber--;
    if (currentVbiFrameNumber < detectionSources.getMinimumVbiFrameNumber()) currentVbiFrameNumber = detectionSources.getMinimumVbiFrameNumber();

    ui->frameNumberSpinBox->setValue(currentVbiFrameNumber);
    ui->frameNumberHorizontalSlider->setValue(currentVbiFrameNumber);
}

void MainWindow::on_nextFramePushButton_clicked()
{
    currentVbiFrameNumber++;
    if (currentVbiFrameNumber > detectionSources.getMaximumVbiFrameNumber()) currentVbiFrameNumber = detectionSources.getMaximumVbiFrameNumber();

    ui->frameNumberSpinBox->setValue(currentVbiFrameNumber);
    ui->frameNumberHorizontalSlider->setValue(currentVbiFrameNumber);
}

void MainWindow::on_frameNumberHorizontalSlider_valueChanged(int value)
{
    (void)value;

    currentVbiFrameNumber = ui->frameNumberHorizontalSlider->value();
    ui->frameNumberSpinBox->setValue(currentVbiFrameNumber);

    if (sourcesReady) updateFrameViewer();
}

void MainWindow::on_frameDisplayTabWidget_currentChanged(int index)
{
    qDebug() << "Action tab changed - new index is" << index;
    if (sourcesReady) updateFrameViewer();
}

void MainWindow::on_actionExit_triggered()
{
    quit();
}

void MainWindow::on_overlayComboBox_currentIndexChanged(const QString &arg1)
{
    (void)arg1;
    if (sourcesReady) updateFrameViewer();
}
