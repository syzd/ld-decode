/************************************************************************

    diffdetector.h

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

#ifndef DIFFDETECTOR_H
#define DIFFDETECTOR_H

#include <QApplication>
#include <QDebug>
#include <QtGlobal>
#include <QtMath>

#include "lddecodemetadata.h"
#include "sourcevideo.h"
#include "datatypes/dropouts.h"

class DiffDetector
{
public:
    DiffDetector();

    Dropouts process(QVector<SourceVideo::Data> frameData, LdDecodeMetaData::VideoParameters videoParameters);

private:
    quint16 median(QVector<quint16> v);
    float convertLinearToBrightness(quint16 value, quint16 black16bIre, quint16 white16bIre, bool isSourcePal);
};

#endif // DIFFDETECTOR_H
