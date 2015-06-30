/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// Qt Usage Environment: for a simple, non-scripted, console application
// C++ header

#ifndef _QT_USAGE_ENVIRONMENT_HH
#define _QT_USAGE_ENVIRONMENT_HH

#ifndef _QT_USAGE_ENVIRONMENT0_HH
#include "QtUsageEnvironment0.hh"
#endif

class QtUsageEnvironment: public QtUsageEnvironment0 {
public:
  static QtUsageEnvironment* createNew(TaskScheduler& taskScheduler);

  // redefined virtual functions:
  virtual int getErrno() const;

  virtual UsageEnvironment& operator<<(char const* str);
  virtual UsageEnvironment& operator<<(int i);
  virtual UsageEnvironment& operator<<(unsigned u);
  virtual UsageEnvironment& operator<<(double d);
  virtual UsageEnvironment& operator<<(void* p);

protected:
  QtUsageEnvironment(TaskScheduler& taskScheduler);
      // called only by "createNew()" (or subclass constructors)
  virtual ~QtUsageEnvironment();
};


class BasicTaskScheduler: public BasicTaskScheduler0 {
public:
  static BasicTaskScheduler* createNew();
  virtual ~BasicTaskScheduler();

protected:
  BasicTaskScheduler();
      // called only by "createNew()"

protected:
  // Redefined virtual functions:
  virtual void SingleStep(unsigned maxDelayTime);

  virtual void setBackgroundHandling(int socketNum, int conditionSet, BackgroundHandlerProc* handlerProc, void* clientData);
  virtual void moveSocketHandling(int oldSocketNum, int newSocketNum);

protected:
  // To implement background operations:
  int fMaxNumSockets;
  fd_set fReadSet;
  fd_set fWriteSet;
  fd_set fExceptionSet;
};

#endif
