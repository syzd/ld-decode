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
#include <QImage>

#include "sourcevideo.h"
#include "lddecodemetadata.h"
#include "datatypes/dropouts.h"

class TbcSource
{
public:
    TbcSource();
    ~TbcSource();

    bool open(QString _inputFilename);
    void close();

    bool isValid();
    bool isCav();
    bool isPal();

    void setReverseFieldOrder();

    qint32 getStartVbiFrame();
    qint32 getEndVbiFrame();
    qint32 getNumberOfFrames();
    bool isFrameAvailable(qint32 vbiFrameNumber);

    QImage getFrameImage(qint32 vbiFrameNumber);
    SourceVideo::Data getFrameData(qint32 vbiFrameNumber);
    Dropouts getFrameDropouts(qint32 vbiFrameNumber);
    LdDecodeMetaData::VideoParameters getVideoParameters();

private:
    bool isSourceValid;
    bool isSourceCav;
    qint32 startVbiFrame;
    qint32 endVbiFrame;
    QString inputFilename;

    SourceVideo sourceVideo;
    LdDecodeMetaData ldDecodeMetaData;

    void defaults();
    bool determineDiscTypeAndFrames();
    qint32 convertVbiFrameNumberToSequential(qint32 vbiFrameNumber);
};

#endif // TBCSOURCE_H
