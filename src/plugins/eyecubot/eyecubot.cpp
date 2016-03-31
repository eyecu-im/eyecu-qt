#include "eyecubot.h"
#include <QDomNode>
#include <QDomNodeList>
#include <QDomElement>
#include <definitions/namespaces.h>
#include <definitions/stanzahandlerorders.h>
#include "utils/qt4qt5compat.h"

#define EYECU_BOT        "eyecu@rwsoftware.ru"
//"vn7@isgeek.info"

#define SHC_MESSAGE         "/iq[@type='set']"
#define SHC_SUBS_IN         "/iq[@type='set']/query[@xmlns='" NS_JABBER_ROSTER "']"
#define SHC_SUBS_RESULT     "/iq[@type='result']"


Eyecubot::Eyecubot():
        FOptionsManager(NULL),
        FRoster(NULL),
        FRosterManager(NULL),
        FRosterChanger(NULL),
        FStanzaProcessor(NULL),
        FAttentionHandlerIn(0),
        FAttentionHandlerOut(0)
{

}

Eyecubot::~Eyecubot()
{
}

//-----------------------------
void Eyecubot::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("eyeCU bot");
    APluginInfo->description = tr("eyeCU bot");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	//APluginInfo->dependences.append(YYY_UUID); //----???
}

bool Eyecubot::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

	IPlugin *plugin = APluginManager->pluginInterface("IRosterManager").value(0,NULL);
    if (plugin)
        {
		FRosterManager = qobject_cast<IRosterManager *>(plugin->instance());
        if (FRosterManager)
            connect(FRosterManager->instance(),SIGNAL(rosterOpened(IRoster *)),this,SLOT(fromRoster(IRoster *)));
        }

	plugin = APluginManager->pluginInterface("IRosterChanger").value(0,NULL);
	if (plugin)
		FRosterChanger = qobject_cast<IRosterChanger *>(plugin->instance());


    plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0);
    if (plugin)
        {
        FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
        }
    else
        return false;

    plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    connect(Options::instance(),SIGNAL(optionsOpened()),SLOT(onOptionsOpened()));
    connect(Options::instance(),SIGNAL(optionsClosed()),SLOT(onOptionsClosed()));
    connect(Options::instance(),SIGNAL(optionsChanged(const OptionsNode &)),SLOT(onOptionsChanged(const OptionsNode &)));


    //AInitOrder = 200;   //
    return true;

}

bool Eyecubot::initObjects()
{
    if (FStanzaProcessor)
    {
        IStanzaHandle requestHandle;
        requestHandle.handler = this;
        requestHandle.order = SHO_DEFAULT;
        requestHandle.direction = IStanzaHandle::DirectionIn;
        requestHandle.conditions.append(SHC_SUBS_IN);
        requestHandle.conditions.append(SHC_SUBS_RESULT);
        FAttentionHandlerIn = FStanzaProcessor->insertStanzaHandle(requestHandle);

        requestHandle.conditions.clear();
        requestHandle.order = SHO_DEFAULT;
        requestHandle.direction = IStanzaHandle::DirectionOut;
        requestHandle.conditions.append(SHC_MESSAGE);
        FAttentionHandlerOut = FStanzaProcessor->insertStanzaHandle(requestHandle);
    }


    return true;
}

bool Eyecubot::initSettings()
{
    //Options::setDefaultValue(OPV_TUNE_SHOW, true);
    //if (FOptionsManager)
    //    FOptionsManager->insertOptionsHolder(this);
    return true;
}


void Eyecubot::fromRoster(IRoster *ARoster)
{
	Jid streamJid = ARoster->streamJid();
	Jid contactJid(EYECU_BOT);
	QList<IRosterItem> ritems = ARoster->items();
	bool bot = false;
	foreach (IRosterItem ritem, ritems)
		if((ritem.itemJid.bare()==EYECU_BOT) &&	(ritem.subscription==SUBSCRIPTION_BOTH))
				bot = true;
	if(!bot)
		FRosterChanger->subscribeContact(streamJid,contactJid);
}


/*-------------------------------------------
<iq from="vn7@jabber.ru/eyeCU-PC" type="set" to="vn7@jabber.ru/eyeCU-PC" id="push2801645042">
  <query xmlns="jabber:iq:roster">
    <item subscription="from" ask="subscribe" jid="vn7@isgeek.info"/>
  </query>
</iq>
---------------------------------------------*/
bool Eyecubot::stanzaReadWrite(int AHandlerId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept)
{
    //Q_UNUSED(AStreamJid);
    Q_UNUSED(AAccept);
//QString xml = AStanza.toString();
//qDebug("Eyecubot::stanza: \n%s",xml.TOASCII().data());

    Jid contactJid(EYECU_BOT);
    QString type = AStanza.type();
    QString id = AStanza.attribute("id");

    if (AHandlerId == FAttentionHandlerIn)
    {
        AAccept = true;
        QDomElement itemElem = AStanza.firstElement("query", NS_JABBER_ROSTER);
        QDomElement a = itemElem.firstChild().toElement();
        if(!a.isNull() && (a.attributeNode("jid").value() == EYECU_BOT))
            {
                QString subscr = a.attributeNode("subscription").value();
                QString ask = a.attributeNode("ask").value();

                if(subscr=="none" && ask.isNull()) // Eyecubot delete subscription
				{
					IRoster *roster = FRosterManager!=NULL ? FRosterManager->findRoster(AStreamJid) : NULL;
					if (roster && roster->isOpen() && !roster->findItem(contactJid).isNull())
						roster->removeItem(contactJid);
					qDebug("Eyecubot delete you subscription");
				}

                if(subscr=="botn")
				{

				}
            }
        //if(){}

        //if(){}

    }

    if (AHandlerId == FAttentionHandlerOut)
    {
        QDomElement itemElem = AStanza.firstElement("query", NS_JABBER_ROSTER);
        QDomElement a = itemElem.firstChild().toElement();
        if(!a.isNull() && (a.attributeNode("jid").value() == EYECU_BOT))
            {
                QString subscr = a.attributeNode("subscription").value();
qDebug("Eyecubot::stanzaReadWrite->FAttentionHandlerOut ");
            }

    }
    return false;
}


void Eyecubot::onOptionsOpened()
{
    //onOptionsChanged(Options::node(OPV_TUNE_SHOW));
}

void Eyecubot::onOptionsClosed()
{
}

void Eyecubot::onOptionsChanged(const OptionsNode &ANode)
{
	Q_UNUSED(ANode)
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_eyecubot, Eyecubot)
#endif
