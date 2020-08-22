/************************************************************************

    mainwindow.h

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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPainter>

#include "detectionsources.h"
#include "detectors/clipdetector.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void configure(bool _interactiveMode, bool _reverse, QVector<QString> _inputFilenames);
    void quit();
    bool process();

private slots:
    void on_frameNumberSpinBox_editingFinished();
    void on_previousFramePushButton_clicked();
    void on_nextFramePushButton_clicked();
    void on_frameNumberHorizontalSlider_valueChanged(int value);
    void on_frameDisplayTabWidget_currentChanged(int index);
    void on_actionExit_triggered();

    void on_overlayComboBox_currentIndexChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;

    bool interactiveMode;
    bool reverse;
    bool sourcesReady;
    QVector<QString> inputFilenames;
    DetectionSources detectionSources;
    QVector<QLabel*> sourceLabel;
    qint32 currentVbiFrameNumber;

    void initialiseGui();
    void updateFrameViewer();
};
#endif // MAINWINDOW_H
