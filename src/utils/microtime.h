//
//   Copyright (C) 2009 by Dmitry Adjiev aka GNU Dimarik
//   gnudimarik@gmail.com
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef MICROTIME_H
#define MICROTIME_H

#include <QTime>
#include "utilsexport.h"

/**
 * @file  microtime.h
 * @class MicroTime
 *
 * Translates time from microseconds to standard view
 */

class UTILS_EXPORT MicroTime : public QTime
{
public:

   /**
    * @param time time in microseconds
    */

    MicroTime(qint64 time = 0);

private:

   /**
    * Calculate seconds
    *
    * @param time - time in microseconds
    * @return seconds
    */

    inline int get_seconds(qint64 time)
    {
        return (time / 1000000 );
    }

   /**
    * Calculate minutes
    *
    * @param time - time in microseconds
    * @return minutes
    */

    inline int get_minutes(qint64 time)
    {
        return (get_seconds(time) / 60);
    }

   /**
    * Calculates hours
    *
    * @param time - time in microseconds
    * @return hours
    */

    inline int get_hours(qint64 time)
    {
        return (get_minutes(time) / 60);
    }

};

#endif // MICROTIME_H
