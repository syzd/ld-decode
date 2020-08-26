/************************************************************************

    diffdetector.cpp

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


#include "diffdetector.h"

DiffDetector::DiffDetector()
{

}

Dropouts DiffDetector::process(QVector<SourceVideo::Data> frameData, LdDecodeMetaData::VideoParameters videoParameters)
{
    Dropouts dropouts;

    // This method requires at least 3 sets of frame data to work
    if (frameData.size() < 3) {
        qDebug() << "3 or more frames are required for differential detection";
        return dropouts;
    }

    // Process the frame
    for (qint32 y = videoParameters.firstActiveFrameLine; y < videoParameters.lastActiveFrameLine ; y++) {
        for (qint32 x = videoParameters.activeVideoStart; x < videoParameters.activeVideoEnd; x++) {
            // Get the 16-bit value for each frame's pixels
            QVector<quint16> pixelValues;
            pixelValues.resize(frameData.size());
            for (qint32 source = 0; source < frameData.size(); source++) {
                pixelValues[source] = static_cast<quint16>(frameData[source][(y * videoParameters.fieldWidth) + x]);
            }

            // Find the median pixel value
            quint16 medianPixelValue = median(pixelValues);

            // If the source's pixel value is too far from the median, mark as a dropout

        }
    }

    // Return the dropouts for the first frame in the passed sources
    return dropouts;
}

// Method to find the median of a vector of qint32s
// This needs an odd number of samples in order to function correctly?
quint16 DiffDetector::median(QVector<quint16> v)
{
    size_t n = v.size() / 2;
    std::nth_element(v.begin(), v.begin()+n, v.end());
    return v[n];
}

// NOTE: JUST INCLUDING THIS FOR REFERENCE RIGHT NOW
// Method to convert a linear IRE to a logarithmic reflective brightness %
// Note: Follows the Rec. 709 OETF transfer function
float DiffDetector::convertLinearToBrightness(quint16 value, quint16 black16bIre, quint16 white16bIre, bool isSourcePal)
{
    float v = 0;
    float l = static_cast<float>(value);

    // Factors to scale Y according to the black to white interval
    // (i.e. make the black level 0 and the white level 65535)
    float yScale = (1.0 / (white16bIre - black16bIre)) * 65535;

    if (!isSourcePal) {
        // NTSC uses a 75% white point; so here we scale the result by
        // 25% (making 100 IRE 25% over the maximum allowed white point)
        yScale *= 125.0 / 100.0;
    }

    // Scale the L to 0-65535 where 0 = blackIreLevel and 65535 = whiteIreLevel
    l = (l - black16bIre) * yScale;
    if (l > 65535) l = 65535;
    if (l < 0) l = 0;

    // Scale L to 0.00-1.00
    l = (1.0 / 65535.0) * l;

    // Rec. 709 - https://en.wikipedia.org/wiki/Rec._709#Transfer_characteristics
    if (l < 0.018) {
        v = 4.500 * l;
    } else {
        v = pow(1.099 * l, 0.45) - 0.099;
    }

    return v;
}
