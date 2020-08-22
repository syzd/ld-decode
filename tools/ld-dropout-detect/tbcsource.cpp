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

// Set defaults
void TbcSource::defaults()
{
    isSourceValid = false;
    isSourceCav = false;
    startVbiFrame = 0;
    endVbiFrame = 0;
}

// Open a TBC source file
bool TbcSource::open(QString _inputFilename)
{
    // If source is already valid, close it before replacing it
    if (isSourceValid) {
        close();
        defaults();
    }

    // Store the filename
    inputFilename = _inputFilename;

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

// Close a TBC source file
void TbcSource::close()
{
    if (isSourceValid) {
        qInfo() << "Closing source with filename" << inputFilename;
        sourceVideo.close();
        defaults();
    }
}

// Returns true if the TBC source file is valid
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

// Set second field first
void TbcSource::setReverseFieldOrder()
{
    if (isSourceValid) {
        ldDecodeMetaData.setIsFirstFieldFirst(false);
    }
}

// Get the lowest available VBI frame number
qint32 TbcSource::getStartVbiFrame()
{
    if (!isSourceValid) return -1;
    return startVbiFrame;
}

// Get the highest available VBI frame number
qint32 TbcSource::getEndVbiFrame()
{
    if (!isSourceValid) return -1;
    return endVbiFrame;
}

// Get the number of available frames
qint32 TbcSource::getNumberOfFrames()
{
    if (!isSourceValid) return -1;
    return endVbiFrame - startVbiFrame + 1;
}

// Returns true if the VBI frame number is available from the TBC source
// Note: Frames can be either out of range (not included in the TBC) or
// padded (missing from the TBC)
bool TbcSource::isFrameAvailable(qint32 vbiFrameNumber)
{
    // Range check
    if (vbiFrameNumber > getEndVbiFrame() || vbiFrameNumber < getStartVbiFrame()) return false; // Frame is out of range

    // Padded check
    bool firstFieldIsPadded = ldDecodeMetaData.getField(ldDecodeMetaData.getFirstFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber))).pad;
    bool secondFieldIsPadded = ldDecodeMetaData.getField(ldDecodeMetaData.getSecondFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber))).pad;

    if (firstFieldIsPadded || secondFieldIsPadded) return false; // Frame is padded

    // Frame is available
    return true;
}

// Method to convert a VBI frame number to a sequential frame number
qint32 TbcSource::convertVbiFrameNumberToSequential(qint32 vbiFrameNumber)
{
    // Offset the VBI frame number to get the sequential source frame number
    return vbiFrameNumber - getStartVbiFrame() + 1;
}

// Get the drop out data for the requested VBI frame
// Gets data for both fields and combines it into a single frame
Dropouts TbcSource::getFrameDropouts(qint32 vbiFrameNumber)
{
    QVector<qint32> startx;
    QVector<qint32> endx;
    QVector<qint32> frameLine;

    if (isFrameAvailable(vbiFrameNumber)) {
        // Get the first and second field dropout data
        qDebug() << "Requesting field dropout data for frame number" << vbiFrameNumber << "- sequential frame" << convertVbiFrameNumberToSequential(vbiFrameNumber);
        LdDecodeMetaData::DropOuts firstFieldDropOuts = ldDecodeMetaData.getFieldDropOuts(ldDecodeMetaData.getFirstFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber)));
        LdDecodeMetaData::DropOuts secondFieldDropOuts = ldDecodeMetaData.getFieldDropOuts(ldDecodeMetaData.getSecondFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber)));

        // Odd lines
        for (qint32 i = 0; i < firstFieldDropOuts.startx.size(); i++) {
            startx.append(firstFieldDropOuts.startx[i]);
            endx.append(firstFieldDropOuts.endx[i]);
            frameLine.append((firstFieldDropOuts.fieldLine[i] * 2) - 1);
        }

        // Even lines
        for (qint32 i = 0; i < secondFieldDropOuts.startx.size(); i++) {
            startx.append(secondFieldDropOuts.startx[i]);
            endx.append(secondFieldDropOuts.endx[i]);
            frameLine.append((secondFieldDropOuts.fieldLine[i] * 2));
        }

        qDebug() << "VBI frame number" << vbiFrameNumber << "has" << firstFieldDropOuts.startx.size() + secondFieldDropOuts.startx.size() << "dropouts";
    } else {
        qDebug() << "Requested frame is not available from source, no dropout data available";
    }    

    return Dropouts(startx, endx, frameLine);
}

// Generate a QImage of the requested VBI frame
QImage TbcSource::getFrameImage(qint32 vbiFrameNumber)
{
    // Get the metadata for the video parameters
    LdDecodeMetaData::VideoParameters videoParameters = ldDecodeMetaData.getVideoParameters();

    // Calculate the frame height
    qint32 frameHeight = (videoParameters.fieldHeight * 2) - 1;

    // Show debug information
    qDebug().nospace() << "Generating a QImage from frame " << vbiFrameNumber <<
                " (" << videoParameters.fieldWidth << "x" << frameHeight << ")";

    // Create a QImage
    QImage frameImage = QImage(videoParameters.fieldWidth, frameHeight, QImage::Format_RGB888);

    // Ensure the requested frame is available
    if (!isFrameAvailable(vbiFrameNumber)) {
        qDebug() << "Requested frame is not available from source";
        frameImage.fill(Qt::darkGray);
        return frameImage;
    }

    // Get the field data
    SourceVideo::Data firstFieldData = sourceVideo.getVideoField(ldDecodeMetaData.getFirstFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber)));
    SourceVideo::Data secondFieldData = sourceVideo.getVideoField(ldDecodeMetaData.getSecondFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber)));

    // Copy the raw 16-bit grayscale data into the RGB888 QImage
    for (qint32 y = 0; y < frameHeight; y++) {
        for (qint32 x = 0; x < videoParameters.fieldWidth; x++) {
            // Get the 16-bit value and convert to 8-bit
            quint16 pixelValue;
            if (y % 2) pixelValue = secondFieldData[(videoParameters.fieldWidth * (y / 2)) + x] / 256;
            else pixelValue = firstFieldData[(videoParameters.fieldWidth * (y / 2)) + x] / 256;

            // Place the 8-bit value into the output QImage
            qint32 xpp = x * 3;
            *(frameImage.scanLine(y) + xpp + 0) = static_cast<uchar>(pixelValue); // R
            *(frameImage.scanLine(y) + xpp + 1) = static_cast<uchar>(pixelValue); // G
            *(frameImage.scanLine(y) + xpp + 2) = static_cast<uchar>(pixelValue); // B
        }
    }

    return frameImage;
}

// Get the raw data from the requested VBI frame
QVector<quint16> TbcSource::getFrameData(qint32 vbiFrameNumber)
{

    // Get the metadata for the video parameters
    LdDecodeMetaData::VideoParameters videoParameters = ldDecodeMetaData.getVideoParameters();

    // Calculate the frame height
    qint32 frameHeight = (videoParameters.fieldHeight * 2) - 1;

    // Show debug information
    qDebug().nospace() << "Generating frame data for frame " << vbiFrameNumber <<
                " (" << videoParameters.fieldWidth << "x" << frameHeight << ")";

    // Create a vector for the data
    QVector<quint16> frameData;
    frameData.fill(0, frameHeight * videoParameters.fieldWidth);

    // Ensure the requested frame is available
    if (!isFrameAvailable(vbiFrameNumber)) {
        qDebug() << "Requested frame is not available from source";
        return frameData;
    }

    // Get the field data
    SourceVideo::Data firstFieldData = sourceVideo.getVideoField(ldDecodeMetaData.getFirstFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber)));
    SourceVideo::Data secondFieldData = sourceVideo.getVideoField(ldDecodeMetaData.getSecondFieldNumber(convertVbiFrameNumberToSequential(vbiFrameNumber)));

    // Copy the data from both fields into the frame data
    for (qint32 y = 0; y < frameHeight; y++) {
        for (qint32 x = 0; x < videoParameters.fieldWidth; x++) {
            // Get the 16-bit value
            if (y % 2) frameData[(videoParameters.fieldWidth * y) + x] = secondFieldData[(videoParameters.fieldWidth * (y / 2)) + x] / 256;
            else frameData[(videoParameters.fieldWidth * y) + x] = firstFieldData[(videoParameters.fieldWidth * (y / 2)) + x] / 256;
        }
    }

    return frameData;
}

// Get the video parameter metadata for the TBC source
LdDecodeMetaData::VideoParameters TbcSource::getVideoParameters()
{
    return ldDecodeMetaData.getVideoParameters();
}