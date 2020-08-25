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

#include "clipdetector.h"

ClipDetector::ClipDetector()
{
}

// The clip detector looks for values that are outside the expected 0 to 100 IRE.
// If a clip is found, the detector hunts back and forth looking for the
// likely start and end point of the event.
//
// Note: This only works in the active video area
Dropouts ClipDetector::process(SourceVideo::Data frameData, LdDecodeMetaData::VideoParameters videoParameters)
{
    QVector<qint32> startx;
    QVector<qint32> endx;
    QVector<qint32> frameLine;

    // Get the actual 0 and 100 IRE points
    quint16 whiteIre = static_cast<quint16>(videoParameters.white16bIre);
    quint16 blackIre = static_cast<quint16>(videoParameters.black16bIre);

    // Calculate the 5% over/undershoot IRE points
    float ireRangePercent = (static_cast<float>(whiteIre - blackIre) / 100.0) * 8; // 8% range
    quint16 whiteOvershoot = whiteIre + static_cast<quint16>(ireRangePercent);
    quint16 blackUndershoot = blackIre - static_cast<quint16>(ireRangePercent);
    if (whiteOvershoot > 65535) whiteOvershoot = 65535;
    if (blackUndershoot < 0) blackUndershoot = 0;

    qDebug() << "blackIRE =" << blackIre << "- whiteIRE =" << whiteIre;
    qDebug() << "black undershoot =" << blackUndershoot << "- white overshoot =" << whiteOvershoot;

    // Process the fields one line at a time
    for (qint32 y = videoParameters.firstActiveFrameLine; y < videoParameters.lastActiveFrameLine ; y++) {
        for (qint32 x = videoParameters.activeVideoStart; x < videoParameters.activeVideoEnd; x++) {
            // Get the 16-bit value for the source field, cast to 32 bit signed
            quint16 sourceIre = static_cast<quint16>(frameData[(y * videoParameters.fieldWidth) + x]);

            // Check for a clip event
            if ((sourceIre < blackUndershoot) || (sourceIre > whiteOvershoot)) {
                // Signal has clipped, scan back and forth looking for the start
                // and end points of the event (i.e. the point where the event
                // goes back into the expected range)
                qint32 range = 20; // maximum + and - scan range (in pixels)
                qint32 minX = x - range;
                if (minX < videoParameters.activeVideoStart) minX = videoParameters.activeVideoStart;
                qint32 maxX = x + range;
                if (maxX > videoParameters.activeVideoEnd) maxX = videoParameters.activeVideoEnd;

                qint32 doStartx = x;
                qint32 doEndx = x;

                // Scan backwards
                for (qint32 i = x; i > minX; i--) {
                    quint16 ire = static_cast<quint16>(frameData[(y * videoParameters.fieldWidth) + i]);
                    if (ire < blackIre || ire > whiteIre) {
                        doStartx = i;
                    }
                }

                // Scan forwards
                for (qint32 i = x + 1; i < maxX; i++) {
                    quint16 ire = static_cast<quint16>(frameData[(y * videoParameters.fieldWidth) + i]);
                    if (ire < blackIre || ire > whiteIre) {
                        doEndx = i;
                    }
                }

                // Record the dropout (only if longer than 1 pixel)
                if ((doEndx - doStartx) > 0) {
                    startx.append(doStartx);
                    endx.append(doEndx);
                    frameLine.append(y + 1);
                }

                // Move to the end of the check range
                x = x + range;
            }
        }
    }

    return Dropouts(startx, endx, frameLine);
}

