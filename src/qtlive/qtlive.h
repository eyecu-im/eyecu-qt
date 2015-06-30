#ifndef QTLIVE_H
#define QTLIVE_H

#include <QObject>
#include "liveExport.h"

class LIVE_EXPORT QtLive : public QObject
{
    Q_OBJECT
public:
    explicit QtLive(QObject *parent = 0);
    
signals:
    
public slots:
    
};

#endif // QTLIVE_H
