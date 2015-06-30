#ifndef MAPSEARCH_H
#define MAPSEARCH_H

#include <QSharedPointer>

#include <interfaces/ipluginmanager.h>
#include <interfaces/imapsearch.h>
#include <interfaces/imainwindow.h>
#include <interfaces/iconnectionmanager.h>
#include <interfaces/imap.h>
#include <interfaces/ipoi.h>
#include <interfaces/imessagewidgets.h>

#include <utils/options.h>

#include "mapsearchdialog.h"

class MapSearch : public QObject,
                  public IPlugin,
				  public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES (IPlugin IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IMapSearch")
#endif
public:
    MapSearch(QObject *parent = 0);
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return MAPSEARCH_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IOptionsHolder
	QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

private:
	IMap				*FMap;
	IPoi				*FPoi;
	IMainWindowPlugin	*FMainWindowPlugin;
	IConnectionManager	*FConnectionManager;
	IOptionsManager		*FOptionsManager;
	IMessageWidgets		*FMessageWidgets;
    QHash <QUuid, IMapSearchProvider *> FSearchProviders;
	QSharedPointer<MapSearchDialog> FMapSearchDialog;

protected:
	QSharedPointer<MapSearchDialog> &mapSearchDialog();

protected slots:
    void onMapSearchTriggered();
    void onChatWindowCreated(IMessageChatWindow *AWindow);
    void onNormalWindowCreated(IMessageNormalWindow *AWindow);
    void onInsertSearchResult(bool AChecked);
};

#endif // MAPSEARCH_H
