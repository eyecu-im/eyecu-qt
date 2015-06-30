#ifndef IMAPLOCATIONSELECTOR_H
#define IMAPLOCATIONSELECTOR_H

#include <QObject>
#include <QtGeo/mercatorcoordinates.h>
#include <QtGeo/mapcoordinates.h>
#include <definitions/mapobjecttyperole.h>

#define MAPLOCATIONSELECTOR_UUID "{25f3d8a2-942a-37c1-f5dc-230b6c8de23b}"

class ILocationSelector
{
public:
    virtual QObject *instance()=0;
    virtual void onLocationSelected() = 0;
};

class IMapLocationSelector
{
public:
    virtual QObject *instance()=0;
    virtual bool selectLocation(ILocationSelector *ALocationSelector) = 0;
    virtual bool finishSelectLocation(ILocationSelector *ALocationSelector, bool ACancel) = 0;
    virtual const MercatorCoordinates &selectedLocation() const = 0;

protected:
    virtual void locationSelected() = 0;
    virtual void locationSelectionCancelled() = 0;
};

Q_DECLARE_INTERFACE(ILocationSelector,"RWS.Plugin.ILocationSelector/1.0")
Q_DECLARE_INTERFACE(IMapLocationSelector,"RWS.Plugin.IMapLocationSelector/1.0")
#endif	//IMAPLOCATIONSELECTOR_H
