#ifndef EYECUBOT_H
#define EYECUBOT_H

#include <interfaces/ieyecubot.h>
#include <interfaces/irostermanager.h>
#include <interfaces/irosterchanger.h>
#include <interfaces/istanzaprocessor.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>

#include <utils/jid.h>

class Eyecubot: public QObject,
                public IPlugin,
                public IEyecubot,
                public IStanzaHandler
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IEyecubot IStanzaHandler)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IEyecubot")
#endif
public:
    Eyecubot();
    ~Eyecubot();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return EYECUBOT_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}
    //IStanzaHandler
    virtual bool stanzaReadWrite(int AHandleId, const Jid &AStreamJid, Stanza &AStanza, bool &AAccept);
    //- plugin functions -----

private:
	IOptionsManager		*FOptionsManager;
	IRoster				*FRoster;
	IRosterManager		*FRosterManager;
	IRosterChanger		*FRosterChanger;
	IStanzaProcessor	*FStanzaProcessor;

	int					FAttentionHandlerIn;
	int					FAttentionHandlerOut;

protected slots:
    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);
    void fromRoster(IRoster *ARoster);
};

#endif // EYECUBOT_H
