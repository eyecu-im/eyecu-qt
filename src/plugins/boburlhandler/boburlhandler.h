#ifndef BOBURLHANDLER_H
#define BOBURLHANDLER_H

#include <QObject>

#include <interfaces/ipluginmanager.h>
#include <interfaces/iurlprocessor.h>
#include <interfaces/ibitsofbinary.h>

#define BOBURLHANDLER_UUID "{8e3f0147-54b6-038e-78c5-23bc2e846ec1}"

class BobUrlHandler : public QObject, public IPlugin, public IUrlHandler
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IUrlHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IBobUrlHandler")
#endif
public:
    BobUrlHandler(QObject *parent = 0);
    
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return BOBURLHANDLER_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings(){return true;}
    virtual bool startPlugin(){return true;}

    //IUrlHandler
    virtual QNetworkReply *request(QNetworkAccessManager::Operation op, const QNetworkRequest &ARequest, QIODevice *AOutgoingData);

private:
    IBitsOfBinary *FBitsOfBinary;
    IUrlProcessor *FUrlProcessor;
};

#endif // BOBURLHANDLER_H
