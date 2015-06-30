#ifndef POSITIONINGMETHODSERIALPORT_H
#define POSITIONINGMETHODSERIALPORT_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/ipositioning.h>

#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>

#include "positioningmethodserialportoptions.h"
#include "portsetings.h"
#include "serialread.h"

#define POSITIONINGMETHODSERIALPORT_UUID "{B387614B-A190-3B9B-8DA9-ED179B3BE20E}"

class PositioningMethodSerialPort: public QObject,
                                   public IPlugin,
                                   public IPositioningMethod,
								   public IOptionsDialogHolder
{
    Q_OBJECT
	Q_INTERFACES(IPlugin IPositioningMethod IPositioningMethod IOptionsDialogHolder)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.PositioningMethodSerialPort")
#endif
public:
    PositioningMethodSerialPort();
    ~PositioningMethodSerialPort();
    //IPlugin
    virtual QObject *instance() { return this; }
    virtual QUuid pluginUuid() const { return POSITIONINGMETHODSERIALPORT_UUID; }
    virtual void pluginInfo(IPluginInfo *APluginInfo);
    virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
    virtual bool initObjects();
    virtual bool initSettings();
    virtual bool startPlugin(){return true;}
    //IOptionsHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent);

    //IPositioningProvider
    virtual QString name() const {return tr("Serial port");}
    virtual bool select(bool ASelect);
    virtual State state() const {return FCurrentState;}

    bool isActive() const {return FSerialRead != NULL && FDataSend;}
    SerialRead *reader() const {return FSerialRead;}
    void startReadPort(const ReaderSettings &AReaderSettings, const PortSettings &APortSettings);
    void stopReadPort();
    int  getLastError() const {return FErrorCode;}

protected:
    PortSettings getPortSettings(const QString &APortName);
    ReaderSettings getReaderSettings();
    void startDataSending(bool AStart);
    void changeCurrentState(State AState);

protected slots:
    void onSerialReadFinished(int AErrorCode);
    void onPortRead(const QByteArray &AData);

    void sendPosition();
    void startPort(bool AStart);

signals:
    //IPositioningProvider
    void stateChanged(int AState);
	void newPositionAvailable(const GeolocElement &APosition);
    void newSatelliteDataAvailable(const SatelliteDataMap &ASateliteData);

private:
    State           FCurrentState;
    IOptionsManager *FOptionsManager;
    IPositioning    *FPositioning;

    NmeaParser      FParser;
    SatelliteDataMap FSatelliteData;
    SerialRead      *FSerialRead;
#if QT_VERSION >= 0x040700
    QDateTime       FTimeDataSent;
#else
    QTime           FTimeDataSent;
#endif
    bool            FDataSend;
	GeolocElement   FCurrentPosition;
    int             FErrorCode;
};

#endif // POSITIONINGMETHODSERIALPORT_H
