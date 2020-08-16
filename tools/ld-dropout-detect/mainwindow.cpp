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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::configure(bool _interactiveMode, bool _reverse, QVector<QString> _inputFilenames)
{
    interactiveMode = _interactiveMode;
    reverse = _reverse;
    inputFilenames = _inputFilenames;
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

    // Close the source files
    detectionSources.close();

    qApp->quit();
    return true;
}
