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

#include "detectionsources.h"

DetectionSources::DetectionSources()
{
}

DetectionSources::~DetectionSources()
{
    close();
}

bool DetectionSources::open(QVector<QString> inputFilenames)
{
    bool first = true;
    bool sourcesAreCav = false;
    bool sourcesArePal = false;

    detectionSource.resize(inputFilenames.size());

    // Load all sources and fail if anything goes wrong
    for (qint32 i = 0; i < inputFilenames.size(); i++) {
        detectionSource[i] = new TbcSource;
        if (!detectionSource[i]->open(inputFilenames[i])) {
            // Opening source failed...
            qWarning() << "Failed to open source" << i << "with filename" << inputFilenames[i];
            close();
            return false;
        }

        if (first) {
            first = false;
            sourcesAreCav = detectionSource[i]->isCav();
            sourcesArePal = detectionSource[i]->isPal();
        } else {
            // Ensure all sources are CAV or all are CLV
            if (detectionSource[i]->isCav() != sourcesAreCav) {
                qWarning() << "Sources are a mixture of CAV and CLV - cannot process!";
                close();
                return false;
            }

            // Ensure all sources are PAL or all are NTSC
            if (detectionSource[i]->isPal() != sourcesArePal) {
                qWarning() << "Sources are a mixture of PAL and NTSC - cannot process!";
                close();
                return false;
            }
        }

        // Show some information about the source TBC
        qInfo().nospace() << "Open TBC source #" << i << " with filename of " << inputFilenames[i];
        qInfo().nospace() << "  Source has " << detectionSource[i]->getNumberOfFrames() << " frames." <<
                             " Start frame is " << detectionSource[i]->getStartVbiFrame() <<
                             " and end frame is " << detectionSource[i]->getEndVbiFrame();
    }

    // Verify that all sources are the same disc type

    return true;
}

void DetectionSources::close()
{
    // Delete and remove all of the detection sources
    for (qint32 i = 0; i < detectionSource.size(); i++) {
        delete detectionSource[i];
        detectionSource.remove(i);
    }
}

TbcSource* DetectionSources::getDetectionSource(qint32 sourceNumber)
{
    if (sourceNumber > detectionSource.size()) {
        qCritical() << "Caller requested out of range detectionSource!";
    }

    return detectionSource[sourceNumber];
}

qint32 DetectionSources::getNumberOfSources()
{
    return detectionSource.size();
}

// Set all sources to reversed field order
void DetectionSources::setReverseFieldOrder()
{
    for (qint32 i = 0; i < detectionSource.size(); i++) {
        detectionSource[i]->setReverseFieldOrder();
    }
}
