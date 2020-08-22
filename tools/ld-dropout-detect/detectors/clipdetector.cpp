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
// Note: This only works for the active area of the frame (since the other areas
// are not 0-100 IRE) - so clip detection should not replace dropouts except in the
// active area.
Dropouts ClipDetector::process(QVector<quint16> frameData, LdDecodeMetaData::VideoParameters videoParameters)
{
    QVector<qint32> startx;
    QVector<qint32> endx;
    QVector<qint32> frameLine;

    // Process the fields one line at a time
    for (qint32 y = videoParameters.firstActiveFrameLine; y < videoParameters.lastActiveFrameLine ; y++) {
        qint32 startOfLinePointer = y * videoParameters.fieldWidth;

        for (qint32 x = videoParameters.colourBurstStart; x < videoParameters.activeVideoEnd; x++) {
            // Get the IRE value for the source field, cast to 32 bit signed
            qint32 sourceIre = static_cast<qint32>(frameData[x + startOfLinePointer]);

            // Check for a luma clip event
            if ((sourceIre == 0) || (sourceIre == 65535)) {
                // Signal has clipped, scan back and forth looking for the start
                // and end points of the event (i.e. the point where the event
                // goes back into the expected range)
                qint32 range = 10; // maximum + and - scan range
                qint32 minX = x - range;
                if (minX < videoParameters.activeVideoStart) minX = videoParameters.activeVideoStart;
                qint32 maxX = x + range;
                if (maxX > videoParameters.activeVideoEnd) maxX = videoParameters.activeVideoEnd;

                qint32 doStartx = x;
                qint32 doEndx = x;

                for (qint32 i = x; i > minX; i--) {
                    qint32 ire = static_cast<qint32>(frameData[x + startOfLinePointer]);
                    if (ire > 200 && ire < 65335) {
                        doStartx = i;
                    }
                }

                for (qint32 i = x + 1; i < maxX; i++) {
                    qint32 ire = static_cast<qint32>(frameData[x + startOfLinePointer]);
                    if (ire > 200 && ire < 65335) {
                        doEndx = i;
                    }
                }

                // Record the dropout
                startx.append(doStartx);
                endx.append(doEndx);
                frameLine.append(y);

                // Move to the end of the clipping event
                x = x + range;
            }
        }
    }

    return Dropouts(startx, endx, frameLine);
}
