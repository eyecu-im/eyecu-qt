#include "positioningmethodserialport.h"

PositioningMethodSerialPortOptions::PositioningMethodSerialPortOptions(PositioningMethodSerialPort *ASerialPort, QWidget *AParent) :
    QWidget(AParent),
	ui(new Ui::PositioningMethodSerialPortOptions),
    FSerialPort(ASerialPort)
{
    ui->setupUi(this);
    setupControls();

    connect(ui->cmbPortName,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));

    connect(ui->cmbBaudRate,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    connect(ui->cmbDataBits,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    connect(ui->cmbStopBits,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    connect(ui->cmbParity,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    connect(ui->cmbFlowControl,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));

    connect(ui->spbBufferSize,SIGNAL(valueChanged(int)),SIGNAL(modified()));
    connect(ui->spbTimeout,SIGNAL(valueChanged(int)),SIGNAL(modified()));
    connect(ui->spbTimeTreshold,SIGNAL(valueChanged(int)),SIGNAL(modified()));
    connect(ui->spbDistanceTreshold,SIGNAL(valueChanged(int)),SIGNAL(modified()));

    connect(FSerialPort, SIGNAL(newSatelliteDataAvailable(SatelliteDataMap)), ui->ssSignalQuality, SLOT(setSatelliteData(SatelliteDataMap)));
    connect(FSerialPort, SIGNAL(newSatelliteDataAvailable(SatelliteDataMap)), ui->ssPosition, SLOT(setSatelliteData(SatelliteDataMap)));

    ui->pbTest->setDisabled(FSerialPort->isActive());   

    reset();
}

PositioningMethodSerialPortOptions::~PositioningMethodSerialPortOptions()
{
    delete ui;
}

void PositioningMethodSerialPortOptions::apply()
{
    QString name = ui->cmbPortName->currentText();
    Options::node(OPV_POSITIONING_METHOD_SERIALPORT_NAME).setValue(name);    
    Options::node(OPV_POSITIONING_METHOD_SERIALPORT_BUFFERSIZE).setValue(ui->spbBufferSize->value());
    Options::node(OPV_POSITIONING_METHOD_SERIALPORT_TIMEOUT).setValue(ui->spbTimeout->value());
    Options::node(OPV_POSITIONING_METHOD_SERIALPORT_TIMETRESHOLD).setValue(ui->spbTimeTreshold->value());
    Options::node(OPV_POSITIONING_METHOD_SERIALPORT_DISTANCETRESHOLD).setValue(ui->spbDistanceTreshold->value());

    OptionsNode node = Options::node(OPV_POSITIONING_METHOD_SERIALPORT, name);
    node.setValue(ui->cmbBaudRate->itemData(ui->cmbBaudRate->currentIndex()).toLongLong(), "baud-rate");
    node.setValue(ui->cmbDataBits->itemData(ui->cmbDataBits->currentIndex()).toInt(), "data-bits");
    node.setValue(ui->cmbStopBits->itemData(ui->cmbStopBits->currentIndex()).toInt(), "stop-bits");
    node.setValue(ui->cmbParity->itemData(ui->cmbParity->currentIndex()).toInt(), "parity");
    node.setValue(ui->cmbFlowControl->itemData(ui->cmbFlowControl->currentIndex()).toInt(), "flow-control");
    emit childApply();
}

void PositioningMethodSerialPortOptions::reset()
{
    FPortName = Options::node(OPV_POSITIONING_METHOD_SERIALPORT_NAME).value().toString(); // Port, currently selected
    if (ui->cmbPortName->currentText().isEmpty())
    {
        ui->spbBufferSize->setValue(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_BUFFERSIZE).value().toInt());
        ui->spbTimeout->setValue(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_TIMEOUT).value().toInt());
        ui->spbTimeTreshold->setValue(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_TIMETRESHOLD).value().toInt());
        ui->spbDistanceTreshold->setValue(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_DISTANCETRESHOLD).value().toInt());
    }
    ui->cmbBaudRate->setCurrentIndex(ui->cmbBaudRate->findData(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_BAUDRATE).value().toLongLong()));
    ui->cmbDataBits->setCurrentIndex(ui->cmbDataBits->findData(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_DATABITS).value().toInt()));
    ui->cmbStopBits->setCurrentIndex(ui->cmbStopBits->findData(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_STOPBITS).value().toString()));
    ui->cmbParity->setCurrentIndex(ui->cmbParity->findData(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_PARITY).value().toString()));
    ui->cmbFlowControl->setCurrentIndex(ui->cmbFlowControl->findData(Options::node(OPV_POSITIONING_METHOD_SERIALPORT_FLOWCONTROL).value().toString()));
    emit childReset();
}

void PositioningMethodSerialPortOptions::onPortRead(const QByteArray &AData)
{
    ui->console->insertPlainText(QString(AData));
    ui->console->ensureCursorVisible();
}

void PositioningMethodSerialPortOptions::disableControls(bool ADisable)
{
    if(ADisable)
    {
        ui->lblWait->setText(QString(tr("Reading port.")).append("<br>").append(tr("Please wait..."))); // <br> "
        ui->cmbPortName->setDisabled(true);
        ui->cmbBaudRate->setDisabled(true);
        ui->cmbDataBits->setDisabled(true);
        ui->cmbFlowControl->setDisabled(true);
        ui->cmbStopBits->setDisabled(true);
        ui->cmbParity->setDisabled(true);
        ui->spbTimeTreshold->setDisabled(true);
        ui->spbDistanceTreshold->setDisabled(true);
        ui->spbBufferSize->setDisabled(true);
        ui->spbTimeout->setDisabled(true);
    }
    else
    {
        ui->lblWait->clear();
        ui->cmbPortName->setEnabled(true);
        ui->cmbBaudRate->setEnabled(true);
        ui->cmbDataBits->setEnabled(true);
        ui->cmbFlowControl->setEnabled(true);
        ui->cmbStopBits->setEnabled(true);
        ui->cmbParity->setEnabled(true);
        ui->spbTimeTreshold->setEnabled(true);
        ui->spbDistanceTreshold->setEnabled(true);
        ui->spbBufferSize->setEnabled(true);
        ui->spbTimeout->setEnabled(true);
    }
}

PortSettings PositioningMethodSerialPortOptions::getPortSettings(const QString &APortName)
{
    PortSettings portSettings;
    OptionsNode node;
    if (Options::hasNode(OPV_POSITIONING_METHOD_SERIALPORT, APortName))
        node = Options::node(OPV_POSITIONING_METHOD_SERIALPORT, APortName);
    else
    {
        QSerialPort *serialPort = new QSerialPort(APortName);
        if (serialPort->open(QIODevice::ReadOnly))
        {
            portSettings.FBaudRate    = serialPort->baudRate();
            portSettings.FDataBits    = serialPort->dataBits();
            portSettings.FStopBits    = serialPort->stopBits();
            portSettings.FParity      = serialPort->parity();
            portSettings.FFlowControl = serialPort->flowControl();
            serialPort->close();
        }
        else // Failed to open the port
            node = Options::node(OPV_POSITIONING_METHOD_SERIALPORT, APortName);
        serialPort->deleteLater();
    }
    if (!node.isNull())
    {
        portSettings.FBaudRate = node.value("baud-rate").toLongLong();
        portSettings.FDataBits = (QSerialPort::DataBits)node.value("data-bits").toInt();
        portSettings.FStopBits = (QSerialPort::StopBits)node.value("stop-bits").toInt();
        portSettings.FParity = (QSerialPort::Parity)node.value("parity").toInt();
        portSettings.FFlowControl = (QSerialPort::FlowControl)node.value("flow-control").toInt();
    }
    return portSettings;
}

void PositioningMethodSerialPortOptions::onPortReadFinished(int AErrorCode)
{
    disableControls(false);
    ui->pbTest->setChecked(false);
    ui->pbTest->setDisabled(false);
    if(AErrorCode == QSerialPort::NoError)
        ui->lblWait->clear();
    else
        ui->lblWait->setText(tr("Error %1").arg(AErrorCode));
}

void PositioningMethodSerialPortOptions::scanPorts()
{
    FPortInfo = new PortInfo();
    connect(FPortInfo,SIGNAL(portsFound(QMap<QString,QString>)),SLOT(onPortsFound(QMap<QString,QString>)));
    connect(FPortInfo,SIGNAL(finished()),SLOT(onPortInfoThreadFinished()));
    FPortInfo->start(QThread::NormalPriority);
}

void PositioningMethodSerialPortOptions::onPortSelected(const QString &APortName)
{
    if (!APortName.isEmpty())
    {
        PortSettings portSettings = getPortSettings(APortName);
        ui->cmbBaudRate->setCurrentIndex(ui->cmbBaudRate->findData(portSettings.FBaudRate==QSerialPort::UnknownBaud?9600:portSettings.FBaudRate));
        ui->cmbDataBits->setCurrentIndex(ui->cmbDataBits->findData(portSettings.FDataBits==QSerialPort::UnknownDataBits?QSerialPort::Data8:portSettings.FDataBits));
        ui->cmbStopBits->setCurrentIndex(ui->cmbStopBits->findData(portSettings.FStopBits==QSerialPort::UnknownStopBits?QSerialPort::OneStop:portSettings.FStopBits));
        ui->cmbParity->setCurrentIndex(ui->cmbParity->findData(portSettings.FParity==QSerialPort::UnknownParity?QSerialPort::NoParity:portSettings.FParity));
        ui->cmbFlowControl->setCurrentIndex(ui->cmbFlowControl->findData(portSettings.FFlowControl==QSerialPort::UnknownFlowControl?QSerialPort::NoFlowControl:portSettings.FFlowControl));
        ui->lbPortData->setText(FPortInfoMap.value(APortName));
    }
}

void PositioningMethodSerialPortOptions::onTest(bool ATest)
{
    if (ATest)
    {
        ui->console->clear();
        PortSettings portSettings;
        portSettings.FBaudRate    = QSerialPort::BaudRate(ui->cmbBaudRate->itemData(ui->cmbBaudRate->currentIndex()).toInt());
        portSettings.FDataBits    = QSerialPort::DataBits(ui->cmbDataBits->itemData(ui->cmbDataBits->currentIndex()).toInt());
        portSettings.FStopBits    = QSerialPort::StopBits(ui->cmbStopBits->itemData(ui->cmbStopBits->currentIndex()).toInt());
        portSettings.FParity      = QSerialPort::Parity(ui->cmbParity->itemData(ui->cmbParity->currentIndex()).toInt());
        portSettings.FFlowControl = QSerialPort::FlowControl(ui->cmbFlowControl->itemData(ui->cmbFlowControl->currentIndex()).toInt());

        ReaderSettings readerSettings;
        readerSettings.FName = ui->cmbPortName->currentText();
        readerSettings.FSize = ui->spbBufferSize->value();
        readerSettings.FTimeout = ui->spbTimeout->value();
        readerSettings.FTimeTreshold = ui->spbTimeTreshold->value();
        readerSettings.FDistanceTreshold = ui->spbDistanceTreshold->value();


        FSerialPort->startReadPort(readerSettings, portSettings);
        if (FSerialPort->reader())
        {
            connect(FSerialPort->reader(),SIGNAL(portRead(QByteArray)),SLOT(onPortRead(QByteArray)));
            connect(FSerialPort->reader(),SIGNAL(finished(int)),SLOT(onPortReadFinished(int)));
            disableControls(true);
        }
        else
        {
            ui->pbTest->setChecked(false);
            int errorCode = FSerialPort->getLastError();
            if(errorCode == QSerialPort::NoError)
                ui->lblWait->clear();
            else
                ui->lblWait->setText(tr("Error %1").arg(errorCode));
        }
    }
    else
    {
        if (FSerialPort->reader())
        {
            FSerialPort->reader()->disconnect(SIGNAL(portRead(QByteArray)),this,SLOT(onPortRead(QByteArray)));
            if (!FSerialPort->isActive())
                FSerialPort->stopReadPort();
        }
    }
}

void PositioningMethodSerialPortOptions::onPortsFound(const QMap<QString, QString> &APortInfo)
{
    FPortInfoMap = APortInfo;
    for (QMap<QString, QString>::ConstIterator it=APortInfo.constBegin(); it!=APortInfo.constEnd(); it++)
        if (ui->cmbPortName->findText(it.key())==-1)
            ui->cmbPortName->addItem(it.key());

    for (int i=0; i<ui->cmbPortName->count(); )
        if (APortInfo.contains(ui->cmbPortName->itemText(i)))
            i++;
        else
            ui->cmbPortName->removeItem(i);

    if (ui->cmbPortName->count())
    {
        int index = ui->cmbPortName->findText(FPortName);
        ui->cmbPortName->setCurrentIndex(index<0?0:index);
    }
    if (ui->cmbPortName->currentText().isEmpty())   // No port name
    {                                               // Disable some controls
        ui->pbTest->setDisabled(true);
        ui->cmbBaudRate->setDisabled(true);
        ui->cmbDataBits->setDisabled(true);
        ui->cmbFlowControl->setDisabled(true);
        ui->cmbStopBits->setDisabled(true);
        ui->cmbParity->setDisabled(true);
    }
}

void PositioningMethodSerialPortOptions::onPortInfoThreadFinished()
{
    FPortInfo->deleteLater();
    FPortInfo = NULL;
    ui->lblWait->clear();
}

void PositioningMethodSerialPortOptions::changeEvent(QEvent *AEvent)
{
    QWidget::changeEvent(AEvent);
    switch (AEvent->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void PositioningMethodSerialPortOptions::showEvent(QShowEvent *AEvent)
{
	Q_UNUSED(AEvent)

    if (FSerialPort->reader())
    {
        disableControls(true);
        ui->pbTest->setDisabled(true);
        connect(FSerialPort->reader(),SIGNAL(portRead(QByteArray)),SLOT(onPortRead(QByteArray)));
    }
    else
    {
        disableControls(false);
        ui->pbTest->setDisabled(false);
    }
    scanPorts();
}

void PositioningMethodSerialPortOptions::hideEvent(QHideEvent *AEvent)
{
	Q_UNUSED(AEvent)

    FPortName = ui->cmbPortName->currentText(); // Remember currently selected port
    if (FSerialPort->reader())
    {
        FSerialPort->reader()->disconnect(SIGNAL(portRead(QByteArray)),this,SLOT(onPortRead(QByteArray)));
        if (!FSerialPort->isActive())
            FSerialPort->stopReadPort();
    }
    ui->console->clear();
}

void PositioningMethodSerialPortOptions::setupControls()
{
    // fill baud rate (is not the entire list of available values,
    ui->cmbBaudRate->addItem(QStringLiteral("1200"), QSerialPort::Baud1200);
    ui->cmbBaudRate->addItem(QStringLiteral("2400"), QSerialPort::Baud2400);
    ui->cmbBaudRate->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    ui->cmbBaudRate->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->cmbBaudRate->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->cmbBaudRate->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->cmbBaudRate->addItem(QStringLiteral("57600"), QSerialPort::Baud57600);
    ui->cmbBaudRate->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);

    // fill data bits
    ui->cmbDataBits->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->cmbDataBits->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->cmbDataBits->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->cmbDataBits->addItem(QStringLiteral("8"), QSerialPort::Data8);

    // fill parity
    ui->cmbParity->addItem(tr("None"), QSerialPort::NoParity);
    ui->cmbParity->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->cmbParity->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->cmbParity->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->cmbParity->addItem(tr("Space"), QSerialPort::SpaceParity);

    // fill stop bits
    ui->cmbStopBits->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#if defined(Q_OS_WIN) || defined(Q_OS_OS2)
    ui->cmbStopBits->addItem(QStringLiteral("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->cmbStopBits->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    // fill flow control
    ui->cmbFlowControl->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->cmbFlowControl->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->cmbFlowControl->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);

    ui->ssPosition->setDisplayType(SatelliteStatus::Position);
}

void PositioningMethodSerialPortOptions::updateUnits(int AValue)
{
    if (sender() == ui->spbBufferSize)
		ui->spbBufferSize->setSuffix(" "+tr("byte(s)", "Buffer size units", AValue));
    else if (sender() == ui->spbTimeTreshold)
		ui->spbTimeTreshold->setSuffix(" "+tr("second(s)", "Time treshold units", AValue));
    else if (sender() == ui->spbDistanceTreshold)
		ui->spbDistanceTreshold->setSuffix(" "+tr("meter(s)", "Distance treshold units", AValue));
    else if (sender() == ui->spbTimeout)
		ui->spbTimeout->setSuffix(" "+tr("millisecond(s)", "Port timeout units", AValue));
}
