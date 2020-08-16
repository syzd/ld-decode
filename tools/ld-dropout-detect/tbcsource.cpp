/************************************************************************

    tbcsource.cpp

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

#include "tbcsource.h"

TbcSource::TbcSource()
{
    defaults();
}

TbcSource::~TbcSource()
{
    close();
}

void TbcSource::defaults()
{
    isSourceValid = false;
    isSourceCav = false;
    startVbiFrame = 0;
    endVbiFrame = 0;
}

bool TbcSource::open(QString inputFilename)
{
    // If source is already valid, close it before replacing it
    if (isSourceValid) {
        close();
        defaults();
    }

    // Open the TBC metadata file
    qDebug() << "Processing JSON metadata for" << inputFilename;
    if (!ldDecodeMetaData.read(inputFilename + ".json")) {
        // Failed to open metadata
        qWarning() << "Open TBC JSON metadata failed for filename" << inputFilename;
        return false;
    }

    // Get the video parameters from the metadata
    LdDecodeMetaData::VideoParameters videoParameters = ldDecodeMetaData.getVideoParameters();

    // Open the source TBC
    qDebug() << "Opening the TBC source file" << inputFilename;
    if (!sourceVideo.open(inputFilename, videoParameters.fieldWidth * videoParameters.fieldHeight)) {
        // Failed to open TBC source file
        qWarning() << "Open TBC file failed for filename" << inputFilename;
        return false;
    }

    // Ensure the disc has been mapped
    if (!ldDecodeMetaData.getVideoParameters().isMapped) {
        qWarning() << "TBC source file has not been mapped";
        close();
        return false;
    }

    // Determine the maximum and minimum VBI frame numbers
    // and the disc type (CAV or CLV)
    if (!determineDiscTypeAndFrames()) {
        close();
        qWarning() << "Unable to determine disc type or number of frames";
        return false;
    }

    // Both Metadata and TBC opened successfully
    isSourceValid = true;
    return true;
}

// Tests to see if the source TBC is CAV or CLV and gets the range of VBI frame
// numbers available
bool TbcSource::determineDiscTypeAndFrames()
{
    // Determine the disc type
    VbiDecoder vbiDecoder;
    qint32 cavCount = 0;
    qint32 clvCount = 0;

    qint32 typeCountMax = 100;
    if (ldDecodeMetaData.getNumberOfFrames() < typeCountMax)
        typeCountMax = ldDecodeMetaData.getNumberOfFrames();

    // Using sequential frame numbering starting from 1
    for (qint32 seqFrame = 1; seqFrame <= typeCountMax; seqFrame++) {
        // Get the VBI data and then decode
        QVector<qint32> vbi1 = ldDecodeMetaData.getFieldVbi(ldDecodeMetaData.getFirstFieldNumber(seqFrame)).vbiData;
        QVector<qint32> vbi2 = ldDecodeMetaData.getFieldVbi(ldDecodeMetaData.getSecondFieldNumber(seqFrame)).vbiData;
        VbiDecoder::Vbi vbi = vbiDecoder.decodeFrame(vbi1[0], vbi1[1], vbi1[2], vbi2[0], vbi2[1], vbi2[2]);

        // Look for a complete, valid CAV picture number or CLV time-code
        if (vbi.picNo > 0) cavCount++;

        if (vbi.clvHr != -1 && vbi.clvMin != -1 &&
                vbi.clvSec != -1 && vbi.clvPicNo != -1) clvCount++;
    }
    qDebug() << "Got" << cavCount << "CAV picture codes and" << clvCount << "CLV timecodes";

    // If the metadata has no picture numbers or time-codes, we cannot use the source
    if (cavCount == 0 && clvCount == 0) {
        qDebug() << "Source does not seem to contain valid CAV picture numbers or CLV time-codes - cannot process";
        return false;
    }

    // Determine disc type
    if (cavCount > clvCount) {
        isSourceCav = true;
        qDebug() << "Got" << cavCount << "valid CAV picture numbers - source disc type is CAV";
        qInfo() << "Disc type is CAV";
    } else {
        isSourceCav = false;
        qDebug() << "Got" << clvCount << "valid CLV picture numbers - source disc type is CLV";
        qInfo() << "Disc type is CLV";

    }

    // Disc has been mapped, so we can use the first and last frame numbers as the
    // min and max range of VBI frame numbers in the input source
    QVector<qint32> vbi1 = ldDecodeMetaData.getFieldVbi(ldDecodeMetaData.getFirstFieldNumber(1)).vbiData;
    QVector<qint32> vbi2 = ldDecodeMetaData.getFieldVbi(ldDecodeMetaData.getSecondFieldNumber(1)).vbiData;
    VbiDecoder::Vbi vbi = vbiDecoder.decodeFrame(vbi1[0], vbi1[1], vbi1[2], vbi2[0], vbi2[1], vbi2[2]);

    if (isSourceCav) {
        startVbiFrame = vbi.picNo;
    } else {
        LdDecodeMetaData::ClvTimecode timecode;
        timecode.hours = vbi.clvHr;
        timecode.minutes = vbi.clvMin;
        timecode.seconds = vbi.clvSec;
        timecode.pictureNumber = vbi.clvPicNo;
        startVbiFrame = ldDecodeMetaData.convertClvTimecodeToFrameNumber(timecode);
    }

    vbi1 = ldDecodeMetaData.getFieldVbi(
                       ldDecodeMetaData.getFirstFieldNumber(ldDecodeMetaData.getNumberOfFrames())).vbiData;
    vbi2 = ldDecodeMetaData.getFieldVbi(
                       ldDecodeMetaData.getSecondFieldNumber(ldDecodeMetaData.getNumberOfFrames())).vbiData;
    vbi = vbiDecoder.decodeFrame(vbi1[0], vbi1[1], vbi1[2], vbi2[0], vbi2[1], vbi2[2]);

    if (isSourceCav) {
        endVbiFrame = vbi.picNo;
    } else {
        LdDecodeMetaData::ClvTimecode timecode;
        timecode.hours = vbi.clvHr;
        timecode.minutes = vbi.clvMin;
        timecode.seconds = vbi.clvSec;
        timecode.pictureNumber = vbi.clvPicNo;
        endVbiFrame = ldDecodeMetaData.convertClvTimecodeToFrameNumber(timecode);
    }

    if (isSourceCav) {
        // If the source is CAV frame numbering should be a minimum of 1 (it
        // can be 0 for CLV sources)
        if (startVbiFrame < 1) {
            qCritical() << "CAV start frame of" << startVbiFrame << "is out of bounds (should be 1 or above)";
            return false;
        }
    }

    qInfo() << "VBI frame number range is" << startVbiFrame << "to" <<
        endVbiFrame;

    return true;
}

void TbcSource::close()
{
    if (isSourceValid) {
        sourceVideo.close();
        defaults();
    }
}

bool TbcSource::isValid()
{
    return isSourceValid;
}

// Returns true if source is CAV and false if source is CLV
bool TbcSource::isCav()
{
    return isSourceCav;
}

// Returns true if source is PAL and false if source is NTSC
bool TbcSource::isPal()
{
    return ldDecodeMetaData.getVideoParameters().isSourcePal;
}

void TbcSource::setReverseFieldOrder()
{
    if (isSourceValid) {
        ldDecodeMetaData.setIsFirstFieldFirst(false);
    }
}

qint32 TbcSource::getStartVbiFrame()
{
    if (!isSourceValid) return -1;
    return startVbiFrame;
}

qint32 TbcSource::getEndVbiFrame()
{
    if (!isSourceValid) return -1;
    return endVbiFrame;
}

qint32 TbcSource::getNumberOfFrames()
{
    if (!isSourceValid) return -1;
    return endVbiFrame - startVbiFrame + 1;
}
