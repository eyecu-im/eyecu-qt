#ifndef IGCOMPASS_H
#define IGCOMPASS_H

#include <QObject>
//#include <utils/geolocdata.h>

#define COMPASS_UUID "{B387614C-A190-3B9B-8DA9-ED179B3BE20E}"
 
class IGcompass {
public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(IGcompass, "RWS.Plugin.IGcompass/1.0")

#endif	//IGCOMPASS_H
