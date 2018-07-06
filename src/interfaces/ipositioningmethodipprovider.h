#ifndef IPOSITIONINGMETHODIPPROVIDER_H
#define IPOSITIONINGMETHODIPPROVIDER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <HttpRequest>
#include <GeolocElement>

class IPositioningMethodIpProvider
{
public:
    virtual QObject *instance() = 0;
	virtual void	setHttpRequester(HttpRequester *AHttpRequester) = 0;
	virtual bool	request() = 0;
	virtual QString	name() const = 0;
	virtual QIcon	icon() const = 0;

protected:
	virtual void requestError() = 0;
	virtual void newPositionAvailable(const GeolocElement &APosition) = 0;
};

Q_DECLARE_INTERFACE(IPositioningMethodIpProvider, "eyeCU.Plugin.IPositioningMethodIpProvider/1.0")

#endif	// IPOSITIONINGMETHODIPPROVIDER_H
