#ifndef IMAPMAGNIFIER
#define IMAPMAGNIFIER_H

#include <QObject>

#define MAPMAGNIFIER_UUID "{f3b24ca3-2ab4-8231-a25f-ce890d175bda}"
 
class IMapMagnifier
{
public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IMapMagnifier, "RWS.Plugin.IMapMagnifier/1.0")

#endif	//IMAPMAGNIFIER_H
