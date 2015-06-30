#ifndef IXHTMLIM_H
#define IXHTMLIM_H

#include <QObject>

#define XHTMLIM_UUID "{c6ea240d-10f5-4daa-beb5-2eab1c18a11d}"
 
class IXhtmlIm
{
public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IXhtmlIm, "RWS.Plugin.IXhtmlIm/1.0")

#endif	//IXHTMLIM_H
