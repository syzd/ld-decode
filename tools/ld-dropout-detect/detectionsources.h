/************************************************************************

    detectionsources.cpp

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

#ifndef DETECTIONSOURCES_H
#define DETECTIONSOURCES_H

#include <QApplication>
#include <QDebug>
#include <QtGlobal>

#include "tbcsource.h"

class DetectionSources
{
public:
    DetectionSources();
    ~DetectionSources();

    bool open(QVector<QString> inputFilenames);
    void close();
    TbcSource *getDetectionSource(qint32 sourceNumber);
    qint32 getNumberOfSources();
    void setReverseFieldOrder();

private:
    QVector<TbcSource*> detectionSource;
};

#endif // DETECTIONSOURCES_H
