/************************************************************************

    dropouts.h

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

#ifndef DROPOUTS_H
#define DROPOUTS_H

#include <QApplication>
#include <QDebug>
#include <QtGlobal>
#include <QMetaType>

class Dropouts
{
public:
    Dropouts() = default;
    ~Dropouts() = default;
    Dropouts(const Dropouts &) = default;

    Dropouts(const QVector<qint32> &startx, const QVector<qint32> &endx, const QVector<qint32> &frameLine);
    Dropouts &operator=(const Dropouts &);

    qint32 size();
    void concatenate();

    QVector<qint32> startx() const;
    QVector<qint32> endx() const;
    QVector<qint32> frameLine() const;

private:
    QVector<qint32> m_startx;
    QVector<qint32> m_endx;
    QVector<qint32> m_frameLine;
};

Q_DECLARE_METATYPE(Dropouts);
QDebug operator<<(QDebug dbg, const Dropouts &dropouts);

#endif // DROPOUTS_H
