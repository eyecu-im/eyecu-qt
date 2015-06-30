/****************************************************************************
** 
** Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
** 
** This file is part of a Qt Solutions component.
**
** Commercial Usage  
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Solutions Commercial License Agreement provided
** with the Software or, alternatively, in accordance with the terms
** contained in a written agreement between you and Nokia.
** 
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
** 
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.1, included in the file LGPL_EXCEPTION.txt in this
** package.
** 
** GNU General Public License Usage 
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
** 
** Please note Third Party Software included with Qt Solutions may impose
** additional restrictions and it is the user's responsibility to ensure
** that they have met the licensing requirements of the GPL, LGPL, or Qt
** Solutions Commercial license and the relevant license of the Third
** Party Software they are using.
** 
** If you are unsure which license is appropriate for your use, please
** contact Nokia at qt-info@nokia.com.
** 
****************************************************************************/

#include "qtlockedfile.h"
#include <QtCore/QFileInfo>
#include <QRegExp>

#define MUTEX_PREFIX "\\SEM32\\QtLockedFile_"
// Maximum number of concurrent read locks. Must not be greater than 65536: maybe OS/2 Toolkit contains a macro definition for that value?
#define MAX_READERS 64

Qt::HANDLE QtLockedFile::getMutexHandle(int idx, bool doCreate)
{    
    if (mutexname.isEmpty()) {
        QFileInfo fi(*this);	
        QString fistr(fi.absoluteFilePath().toLower());
        fistr.replace(QRegExp("[:/\\\\]"), "_");
        mutexname = QString::fromLatin1(MUTEX_PREFIX)
                    + fistr;
    }
    QString mname(mutexname);

    if (idx >= 0)
        mname += QString::number(idx);

    Qt::HANDLE mutex;
    if (doCreate) {
	APIRET rc=DosCreateMutexSem(mname.toLocal8Bit().constData(), &mutex, 0L, FALSE);
        if (rc) {
            qWarning("QtLockedFile::getMutexHandle(): DosCreateMutexSem() failed! rc=%x\n", rc);
            return 0;
        }
    }
    else {
	APIRET rc=DosOpenMutexSem(mname.toLocal8Bit().constData(), &mutex);
        if (rc) {
            if (rc!=ERROR_SEM_NOT_FOUND)
                qWarning("QtLockedFile::getMutexHandle(): DosOpenMutexSem() failed! rc=%x\n", rc);
            return 0;
        }
    }
    return mutex;
}

bool QtLockedFile::waitMutex(Qt::HANDLE mutex, bool doBlock)
{
    Q_ASSERT(mutex);
    APIRET rc=DosRequestMutexSem(mutex, doBlock ? SEM_INDEFINITE_WAIT:SEM_IMMEDIATE_RETURN);
    switch (rc) {
        case NO_ERROR:
        case ERROR_SEM_OWNER_DIED:
            return true;
            break;
        case ERROR_TIMEOUT:
            break;
        default:
            qDebug("QtLockedFile::waitMutex: DosRequestMutexSem() failed! rc=%d\n", rc);
    }
    return false;
}

bool QtLockedFile::lock(LockMode mode, bool block)
{
    if (!isOpen()) {
        qWarning("QtLockedFile::lock(): file is not opened");
        return false;
    }

    if (mode == NoLock)
        return unlock();

    if (mode == m_lock_mode)
        return true;

    if (m_lock_mode != NoLock)
        unlock();

    if (!wmutex && !(wmutex = getMutexHandle(-1, true)))
        return false;

    if (!waitMutex(wmutex, block))
        return false;

    if (mode == ReadLock) {
        int idx = 0;
        for (; idx < MAX_READERS; idx++) {
            rmutex = getMutexHandle(idx, false);
            if (!rmutex || waitMutex(rmutex, false))
                break;
            DosCloseMutexSem(rmutex);
        }
        bool ok = true;
        if (idx >= MAX_READERS) {
            qWarning("QtLockedFile::lock(): too many readers");
            rmutex = 0;
            ok = false;
        }
        else if (!rmutex) {
            rmutex = getMutexHandle(idx, true);
            if (!rmutex || !waitMutex(rmutex, false))
                ok = false;
        }
        if (!ok && rmutex) {
            DosCloseMutexSem(rmutex);
            rmutex = 0;
        }
        DosReleaseMutexSem(wmutex);
        if (!ok)
            return false;
    } else {
        Q_ASSERT(rmutexes==null);
        rmutexes=new SEMRECORD[MAX_READERS];
        int i;
        for (i=numMutexes=0; i < MAX_READERS; i++) {
            HSEM mutex = (HSEM)getMutexHandle(i, false);
            if (mutex) {
                rmutexes[numMutexes].hsemCur=mutex;
		rmutexes[numMutexes].ulUser=0L;
		numMutexes++;
            }
        }

        if (numMutexes) {
            bool rc=false;
            HMUX      hmuxHandAny = NULLHANDLE;
            APIRET res=DosCreateMuxWaitSem((PSZ) NULL,
                           &hmuxHandAny,
                           numMutexes,              /* Number of semaphores in list */
                           rmutexes,            /* Semaphore list               */
                           DCMW_WAIT_ALL);  /* Wait for any semaphore       */

            if (res!=NO_ERROR)
               qWarning("QtLockedFile::lock(): DosCreateMuxWaitSem() failed! res=%d\n", res);
            else {
               ULONG ulDummy;
               res = DosWaitMuxWaitSem(hmuxHandAny, block?SEM_INDEFINITE_WAIT:SEM_IMMEDIATE_RETURN, &ulDummy);
               if (res != NO_ERROR && res != ERROR_SEM_OWNER_DIED) {
                  if (res != ERROR_TIMEOUT)
                     qWarning("QtLockedFile::lock(): DosWaitMuxWaitSem() failed! res=%d\n", res);
                   m_lock_mode = WriteLock;  // trick unlock() to clean up - semiyucky
                   unlock();
               } else
                  rc=true;
            }
            res = DosCloseMuxWaitSem(hmuxHandAny);
            if (res != NO_ERROR)
               qWarning("QtLockedFile::lock(): DosCloseMuxWaitSem() failed! res=%d\n", res);
            return rc;
         }        
    }

    m_lock_mode = mode;
    return true;
}

bool QtLockedFile::unlock()
{
    if (!isOpen()) {
        qWarning("QtLockedFile::unlock(): file is not opened");
        return false;
    }

    if (!isLocked())
        return true;

    if (m_lock_mode == ReadLock) {
        DosReleaseMutexSem(rmutex);
        DosCloseMutexSem(rmutex);
        rmutex = 0;
    } else {
        for (int i=0; i<numMutexes; i++) {
            DosReleaseMutexSem((HMTX)rmutexes[i].hsemCur);
            DosCloseMutexSem((HMTX)rmutexes[i].hsemCur);
        }
        delete rmutexes;
        rmutexes=NULL;
        numMutexes=0;
        DosReleaseMutexSem(wmutex);
    }

    m_lock_mode = QtLockedFile::NoLock;
    return true;
}

QtLockedFile::~QtLockedFile()
{
    if (isOpen())
        unlock();
    if (wmutex)
        DosCloseMutexSem(wmutex);
}
