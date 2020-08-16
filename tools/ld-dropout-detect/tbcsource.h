/************************************************************************

    tbcsource.h

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

#ifndef TBCSOURCE_H
#define TBCSOURCE_H

#include <QApplication>
#include <QDebug>
#include <QtGlobal>

#include "sourcevideo.h"
#include "lddecodemetadata.h"

class TbcSource
{
public:
    TbcSource();
    ~TbcSource();

    bool open(QString inputFilename);
    void close();

    bool isValid();
    bool isCav();
    bool isPal();

    void setReverseFieldOrder();

    qint32 getStartVbiFrame();
    qint32 getEndVbiFrame();
    qint32 getNumberOfFrames();

private:
    bool isSourceValid;
    bool isSourceCav;
    qint32 startVbiFrame;
    qint32 endVbiFrame;

    SourceVideo sourceVideo;
    LdDecodeMetaData ldDecodeMetaData;

    void defaults();
    bool determineDiscTypeAndFrames();
};

#endif // TBCSOURCE_H
