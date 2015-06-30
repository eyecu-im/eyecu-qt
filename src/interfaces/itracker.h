#ifndef ITRACKER_H
#define ITRACKER_H

#include <QObject>

#define TRACKER_UUID "{fdd3ab79-25da-4f96-a3d6-add3188047aa}"
 
class ITracker {
public:
    virtual QObject *instance() =0;
};

Q_DECLARE_INTERFACE(ITracker, "RWS.Plugin.ITracker/1.0")

#endif	//ITRACKER_H
