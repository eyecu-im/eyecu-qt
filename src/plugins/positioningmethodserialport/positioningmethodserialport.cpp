#include <MercatorCoordinates>
#include "positioningmethodserialport.h"

PositioningMethodSerialPort::PositioningMethodSerialPort():
    FCurrentState(Stopped),
    FOptionsManager(NULL),
    FPositioning(NULL),
    FSerialRead(NULL),
    FDataSend(false),
    FErrorCode(QSerialPort::NoError)
{}

PositioningMethodSerialPort::~PositioningMethodSerialPort()
{}

//-----------------------------
void PositioningMethodSerialPort::pluginInfo(IPluginInfo *APluginInfo)
{
    APluginInfo->name = tr("Positioning Method Serial Port");
    APluginInfo->description = tr("Positioning method, which allows to use a positioning device, connected to a serial port");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(POSITIONING_UUID);
}

bool PositioningMethodSerialPort::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
    Q_UNUSED(AInitOrder);

    IPlugin *plugin= APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());

    AInitOrder = 150;   // This one should be initialized AFTER ...
    return true;

}

bool PositioningMethodSerialPort::initObjects()
{
    return true;
}

bool PositioningMethodSerialPort::initSettings()
{
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_NAME, QString());
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_BUFFERSIZE, 512);                         // Bytes
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_TIMETRESHOLD, 60);                  // Seconds
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_DISTANCETRESHOLD, 30);              // Meters
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_BAUDRATE, QSerialPort::Baud9600);
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_TIMEOUT, 1000);                     // Milliseconds
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_DATABITS, QSerialPort::Data8);
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_PARITY, QSerialPort::NoParity);
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_STOPBITS, QSerialPort::OneStop);
    Options::setDefaultValue(OPV_POSITIONING_METHOD_SERIALPORT_FLOWCONTROL, QSerialPort::NoFlowControl);

    if (FOptionsManager)
    {
		IOptionsDialogNode dnode = {ONO_SERIALPORT, OPN_GEOLOC"."+pluginUuid().toString(), MNI_POSITIONING_SERIALPORT, tr("Serial port")};
        FOptionsManager->insertOptionsDialogNode(dnode);
		FOptionsManager->insertOptionsDialogHolder(this);
    }
    return true;
}

QMultiMap<int, IOptionsDialogWidget *> PositioningMethodSerialPort::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (ANodeId == OPN_GEOLOC"."+pluginUuid().toString())
        widgets.insertMulti(OWO_SERIALPORT, new PositioningMethodSerialPortOptions(this, AParent));
    return widgets;
}

bool PositioningMethodSerialPort::select(bool ASelect)
{
    startPort(ASelect);
    startDataSending(ASelect);
    return true;
}

void PositioningMethodSerialPort::startPort(bool AStart)
{
    if(AStart)
    {
        ReaderSettings readerSettings = getReaderSettings();
        startReadPort(readerSettings, getPortSettings(readerSettings.FName));        
    }
    else
        if(FSerialRead)
            stopReadPort();
}

void PositioningMethodSerialPort::startDataSending(bool AStart)
{
    if (FDataSend != AStart)
    {
        FDataSend = AStart;
        if (FDataSend)
        {

            FTimeDataSent =
#if QT_VERSION >= 0x040700
                    QDateTime();    // Reset
#else
                    QTime();        // Reset
#endif
			FCurrentPosition = GeolocElement();  // Invalidate
        }
    }
}

void PositioningMethodSerialPort::changeCurrentState(IPositioningMethod::State AState)
{
    if (FCurrentState != AState)
        emit stateChanged(FCurrentState = AState);
}

void PositioningMethodSerialPort::startReadPort(const ReaderSettings &AReaderSettings, const PortSettings &APortSettings)
{
    changeCurrentState(Starting);
    if (!FSerialRead)
    {
        FSerialRead = new SerialRead(AReaderSettings, APortSettings);
        connect(FSerialRead,SIGNAL(finished(int)),SLOT(onSerialReadFinished(int)));
        connect(FSerialRead,SIGNAL(portRead(QByteArray)),SLOT(onPortRead(QByteArray)));
        FSerialRead->openSerialPort();        
    }
}

void PositioningMethodSerialPort::onPortRead(const QByteArray &AData)
{
    if (FParser.parse(AData))
    {
        if (FDataSend)
        {
            if (FCurrentState == Starting)
                changeCurrentState(Started);
            sendPosition();
        }
        if (FParser.isDataReady())
        {
            SatelliteDataMap satelliteData = FParser.satelliteData();
            SatelliteDataMap::ConstIterator it1 = satelliteData.constBegin();
            bool different = false;
            for (SatelliteDataMap::ConstIterator it=FSatelliteData.constBegin(); it!=FSatelliteData.constEnd(); it++, it1++)
                if ((it.key() != it1.key()) || (it.value() != it1.value()))
                {
                    different = true;
                    break;
                }
            if (it1 != satelliteData.constEnd())
                different = true;
            if (different)
            {
                FSatelliteData = satelliteData;
                emit newSatelliteDataAvailable(FSatelliteData);
            }
        }
    }
}

void PositioningMethodSerialPort::sendPosition()
{
	GeolocElement position = FParser.position();
    if (position.isValid())
    {
        if (FTimeDataSent.isValid())
        {
#if (QT_VERSION >= 0x040700)
            if ((FTimeDataSent.msecsTo(QDateTime::currentDateTime())) < Options::node(OPV_POSITIONING_METHOD_SERIALPORT_TIMETRESHOLD).value().toLongLong()*1000)
#else
            int dt=FTimeDataSent.msecsTo(QTime::currentTime());
            if (dt<0)
                dt+=24*60*60*1000;
            if (dt < Options::node(OPV_POSITIONING_METHOD_SERIALPORT_TIMETRESHOLD).value().toLongLong()*1000)
#endif
            return;
        }

		if (FCurrentPosition.isValid() &&
			FCurrentPosition.reliability() == position.reliability() &&
			position.coordinates().distance(FCurrentPosition.coordinates()) < Options::node(OPV_POSITIONING_METHOD_SERIALPORT_DISTANCETRESHOLD).value().toInt())
			return;

        FCurrentPosition = position;
        FTimeDataSent =
#if (QT_VERSION >= 0x040700)
                    QDateTime::currentDateTime();
#else
                    QTime::currentTime();
#endif
        emit newPositionAvailable(position);
    }
}

void PositioningMethodSerialPort::stopReadPort()
{
    changeCurrentState(Stopping);
    if(FSerialRead)
        FSerialRead->closeSerialPort();
}

void PositioningMethodSerialPort::onSerialReadFinished(int AErrorCode)
{
    FErrorCode = AErrorCode;
    changeCurrentState(Stopped);    
    if(FSerialRead)
    {
        disconnect(FSerialRead,SIGNAL(portRead(QByteArray)),this,SLOT(onPortRead(QByteArray)));
        FSerialRead->deleteLater();
        FSerialRead = NULL;                
    }
}

PortSettings PositioningMethodSerialPort::getPortSettings(const QString &APortName)
{
    OptionsNode node = Options::node(OPV_POSITIONING_METHOD_SERIALPORT, APortName);
    PortSettings portSettings;

    portSettings.FBaudRate     = QSerialPort::BaudRate(node.value("baud-rate").toInt());
    portSettings.FDataBits     = QSerialPort::DataBits(node.value("data-bits").toInt());
    portSettings.FStopBits     = QSerialPort::StopBits(node.value("stop-bits").toInt());
    portSettings.FParity       = QSerialPort::Parity(node.value("parity").toInt());
    portSettings.FFlowControl  = QSerialPort::FlowControl(node.value("flow-control").toInt());
    return portSettings;
}

ReaderSettings PositioningMethodSerialPort::getReaderSettings()
{
    OptionsNode node = Options::node(OPV_POSITIONING_METHOD_SERIALPORT);
    ReaderSettings readerSettings;
    readerSettings.FSize         = node.value("size").toInt();
    readerSettings.FTimeout      = node.value("timeout").toInt();
    readerSettings.FTimeTreshold = node.value("time-treshold").toInt();
    readerSettings.FDistanceTreshold = node.value("distance-treshold").toInt();
    readerSettings.FName         = node.value("name").toString();
    return readerSettings;
}
#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_positioningmethodserialport, PositioningMethodSerialPort)
#endif
