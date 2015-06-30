#ifndef IOOB
#define IOOB_H

#include <QObject>

#define OOB_UUID "{3d5702bc-29b9-40f2-88fe-85887cd6d8de}"
 
class IOob
{
public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IOob, "RWS.Plugin.IOob/1.0")

#endif	//IOOB_H
