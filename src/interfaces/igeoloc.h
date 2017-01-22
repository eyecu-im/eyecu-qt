#ifndef IGEOLOC_H
#define IGEOLOC_H

#include <QPointF>

#include <utils/jid.h>
#include <QtGeo/mercatorcoordinates.h>
#include <QtGeo/geolocelement.h>

#define GEOLOC_UUID "{B387614D-A190-3B9B-8DA9-ED179B3BE20E}"

class IGeoloc {
public:
	virtual QObject *instance() =0;
	virtual QIcon   getIcon() const =0;
	virtual QString getIconFileName() const =0;
	virtual GeolocElement getGeoloc(const Jid &AJid) const = 0;
	virtual bool    hasGeoloc(const Jid &AJid) const = 0;
	virtual QString getLabel(const Jid &AContactJid) const =0;

protected:
	virtual void locationReceived(const Jid &AStreamJid, const Jid &AContactJid, const MercatorCoordinates &ACoordinates, bool) =0;
	virtual void locationRemoved(const Jid &AStreamJid, const Jid &AContactJid) =0;
};

Q_DECLARE_INTERFACE(IGeoloc, "RWS.Plugin.IGeoloc/2.0")

#endif	//IGEOLOC_H
