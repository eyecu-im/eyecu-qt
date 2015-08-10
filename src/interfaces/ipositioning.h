#ifndef IPOSITIONING_H
#define IPOSITIONING_H

#include <math.h>
#include <QtGeo/geolocelement.h>

#define POSITIONING_UUID "{639EDBAA-A684-42e4-A9AD-28FC9BCB8F7C}"

class IPositioningMethod
{
public:
    enum State
    {
        Stopped,
        Starting,
        Started,
        Stopping
    };

    virtual QObject *instance() =0;
    virtual QString name() const =0;
    virtual bool select(bool ASelect) =0;
    virtual State state() const =0;
	virtual QString	iconId() const = 0;
protected:
	virtual void newPositionAvailable(const GeolocElement &APosition) =0;
    virtual void stateChanged(int AState) =0;
};

class IPositioning
{
public:
	virtual QObject *instance() =0;
	virtual const GeolocElement &currentPosition() const =0;

protected:
	virtual void newPositionAvailable(const GeolocElement &APosition) =0;
};

Q_DECLARE_INTERFACE(IPositioningMethod,"RWS.Plugin.IPositioningMethod/1.0")
Q_DECLARE_INTERFACE(IPositioning,"RWS.Plugin.IPositioning/1.0")

#endif	//IPOSITIONING_H
