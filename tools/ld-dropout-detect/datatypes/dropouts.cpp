/************************************************************************

    dropouts.cpp

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

#include "dropouts.h"

Dropouts::Dropouts(const QVector<qint32> &startx, const QVector<qint32> &endx, const QVector<qint32> &frameLine)
    : m_startx(startx), m_endx(endx), m_frameLine(frameLine)
{
}

// Assignment '=' operator
Dropouts& Dropouts::operator=(const Dropouts &inDropouts)
{
    // Do not allow assignment if both objects are the same
    if (this != &inDropouts) {
        // Copy the object's data
        m_startx = inDropouts.m_startx;
        m_endx = inDropouts.m_endx;
        m_frameLine = inDropouts.m_frameLine;;
    }

    return *this;
}

// Append a dropout
void Dropouts::append(const qint32 startx, const qint32 endx, const qint32 frameLine)
{
    m_startx.append(startx);
    m_endx.append(endx);
    m_frameLine.append(frameLine);
}

// Return the size of the Dropouts record
qint32 Dropouts::size() const
{
    return m_startx.size();
}

// Get methods
QVector<qint32> Dropouts::startx() const
{
    return m_startx;
}

QVector<qint32> Dropouts::endx() const
{
    return m_endx;
}

QVector<qint32> Dropouts::frameLine() const
{
    return m_frameLine;
}

// Method to concatenate dropouts on the same line that are close together
// (to cut down on the amount of generated dropout data when processing noisy/bad sources)

// TODO: Sort the DOs by frameLine otherwise this won't work correctly

void Dropouts::concatenate()
{
    qint32 sizeAtStart = m_startx.size();

    // This variable controls the minimum allowed gap between dropouts
    // if the gap between the end of the last dropout and the start of
    // the next is less than minimumGap, the two dropouts will be
    // concatenated together
    qint32 minimumGap = 50;

    // Start from dropout 1 as 0 has no previous dropout
    qint32 i = 1;

    while (i < m_startx.size()) {
        // Is the current dropout on the same frame line as the last?
        if (m_frameLine[i - 1] == m_frameLine[i]) {
            if ((m_endx[i - 1] + minimumGap) > (m_startx[i])) {
                // Concatenate
                m_endx[i - 1] = m_endx[i];

                // Remove the current dropout
                m_startx.removeAt(i);
                m_endx.removeAt(i);
                m_frameLine.removeAt(i);
            }
        }

        // Next dropout
        i++;
    }

    qDebug() << "Concatenated dropouts: was" << sizeAtStart << "now" << m_startx.size() << "dropouts";
}

// Custom debug streaming operator
// THIS REALLY SHOULD LIST ALL DROPOUTS TO DEBUG
QDebug operator<<(QDebug dbg, const Dropouts &dropouts)
{
    dbg.nospace() << "Dropout object contains " << dropouts.size() << " entries:" << endl;

    for (qint32 i = 0; i < dropouts.size(); i++) {
        dbg.nospace() << "  [" << i << "] startx = " << dropouts.startx()[i] <<
                         " - endx = " << dropouts.endx()[i] << " - line = " << dropouts.frameLine()[i] << endl;
    }

    return dbg.maybeSpace();
}

