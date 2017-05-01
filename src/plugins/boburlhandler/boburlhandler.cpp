#include "boburlhandler.h"
#include "networkreplyprivate.h"

BobUrlHandler::BobUrlHandler(QObject *parent) :
    QObject(parent), FBitsOfBinary(NULL), FUrlProcessor(NULL)
{}

void BobUrlHandler::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("BOB URL handler");
    APluginInfo->description = tr("Allows URL processor plugin to handle Bits of Binary links (cid: scheme)");
    APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
    APluginInfo->dependences.append(URLPROCESSOR_UUID);
    APluginInfo->dependences.append(BITSOFBINARY_UUID);
}

bool BobUrlHandler::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder)

    IPlugin *plugin = APluginManager->pluginInterface("IUrlProcessor").value(0,NULL);
    if (plugin)
        FUrlProcessor = qobject_cast<IUrlProcessor *>(plugin->instance());
    else
        return false;

    plugin = APluginManager->pluginInterface("IBitsOfBinary").value(0,NULL);
    if (plugin)
        FBitsOfBinary = qobject_cast<IBitsOfBinary *>(plugin->instance());
    else
        return false;

    return true;
}

bool BobUrlHandler::initObjects()
{
    if (FUrlProcessor)
        return FUrlProcessor->registerUrlHandler("cid", this);

    return false;
}

QNetworkReply *BobUrlHandler::request(QNetworkAccessManager::Operation op, const QNetworkRequest &ARequest, QIODevice *AOutgoingData)
{
    if (op==QNetworkAccessManager::GetOperation||       // The only operations
        op==QNetworkAccessManager::HeadOperation)       // supported for now
    {
        QString cid=ARequest.url().path();
        if (cid.startsWith("sha1+") && cid.endsWith("@bob.xmpp.org"))
            return new NetworkReplyPrivate(op, ARequest, AOutgoingData, this, FBitsOfBinary, FUrlProcessor->instance());
    }
    return NULL;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_boburlhandler, BobUrlHandler)
#endif
