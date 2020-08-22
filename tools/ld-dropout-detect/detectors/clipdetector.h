/************************************************************************

    clipdetector.cpp

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

#ifndef CLIPDETECTOR_H
#define CLIPDETECTOR_H

#include <QApplication>
#include <QDebug>
#include <QtGlobal>

#include "lddecodemetadata.h"
#include "datatypes/dropouts.h"

class ClipDetector
{
public:
    ClipDetector();

    Dropouts process(QVector<quint16> frameData, LdDecodeMetaData::VideoParameters videoParameters);
};

#endif // CLIPDETECTOR_H
