#ifndef ICHATMARKERS_H
#define ICHATMARKERS_H

#include <QObject>
#include <QIcon>

#define CHATMARKERS_UUID "{}"
                     
class IChatMarkers {

public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IChatMarkers, "RWS.Plugin.IChatMarkers/1.0")

#endif	//ICHATMARKERS_H
