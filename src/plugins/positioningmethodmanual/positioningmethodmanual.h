#ifndef POSITIONINGMETHODMANUAL_H
#define POSITIONINGMETHODMANUAL_H

#include <QTreeWidget>

#include <interfaces/imap.h>
#include <interfaces/ipoi.h>
#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioning.h>
#include <interfaces/imainwindow.h>

#include <definitions/resources.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>

#include <utils/action.h>
#include "positioningmethodmanualoptions.h"

#define POSITIONINGMETHODMANUAL_UUID "{32dbe444-c291-4947-ab76-899ce7bcd023}"

class PositioningMethodManual:
        public QObject,
        public IPlugin,
        public IPositioningMethod,
		public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IPositioningMethod IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.PositioningMethodManual")
#endif
public:
    PositioningMethodManual();
    ~PositioningMethodManual();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return POSITIONINGMETHODMANUAL_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}

    //IPositioningProvider
    virtual QString name() const {return tr("Manual");}
    virtual bool select(bool ASelect);
    virtual State state() const {return FCurrentState;}

    //IOptionsHolder
    virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);    

public slots:
    void hereIam();
	void setPosition();
    void retractGeoloc();

protected:
	void addMenu();
	void deleteMenu();
    void newDataSend(const QPointF &ACoordinates, const QString &ATimeStamp);
    void hereIAmCoords(double ALongitude, double ALatitude);
    void hereIamPoi(QString APoiId);
    void changeCurrentState(State AState);

protected slots:
    void setTimeInterval(long timeout);
    void timerDataSend();

    void onOptionsOpened();
    void onOptionsClosed();
    void onOptionsChanged(const OptionsNode &ANode);
    void onPoiActionTriggered();
    void onShortcutActivated(const QString &AId, QWidget *AWidget);

signals:
    void stateChanged(int AState);
	void newPositionAvailable(const GeolocElement &APosition);

private:
    State               FCurrentState;
    IOptionsManager     *FOptionsManager;
	IMainWindowPlugin	*FMainWindowPlugin;
    IMap                *FMap;
    IPoi                *FPoi;
    PositioningMethodManualOptions      *FOptions;
    Action              *FActionHereIAm;
	Action              *FActionSetPosition;
    Action              *FActionStopPublish;
    Action              *FPoiActionHereIAm;
    QTimer              *FIntervalTimer;
};
#endif // POSITIONINGMETHODMANUAL_H
