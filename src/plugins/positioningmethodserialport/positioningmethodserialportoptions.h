#ifndef POSITIONINGMETHODSERIALPORTOPTIONS_H
#define POSITIONINGMETHODSERIALPORTOPTIONS_H

#include <QWidget>
#include <QTreeWidget>
#include <QSerialPort>

#include <interfaces/ioptionsmanager.h>
#include <definitions/optionvalues.h>
#include <utils/options.h>
#include <NmeaParser>

#include "ui_positioningmethodserialportoptions.h"
#include "portinfo.h"
#include "portsetings.h"

namespace Ui {
	class PositioningMethodSerialPortOptions;
}

class PositioningMethodSerialPort;

class PositioningMethodSerialPortOptions :
        public QWidget,
		public IOptionsDialogWidget
{
    Q_OBJECT
	Q_INTERFACES(IOptionsDialogWidget)

public:
    PositioningMethodSerialPortOptions(PositioningMethodSerialPort *ASerialPort, QWidget *AParent = 0);
    ~PositioningMethodSerialPortOptions();
    // IOptionsWidget
    virtual QWidget* instance() { return this; }

public slots:
	virtual void apply();
	virtual void reset();

protected:
    void changeEvent(QEvent *AEvent);
    void showEvent(QShowEvent *AEvent);
    void hideEvent(QHideEvent *AEvent);
    void setupControls();
    void disableControls(bool ADisable);
    static PortSettings getPortSettings(const QString &APortName);

protected slots:
    void scanPorts();
    void onPortSelected(const QString &APortName);
    void onTest(bool ATest);
    void onPortsFound(const QMap<QString,QString> &APortInfo);
    void onPortInfoThreadFinished();
    void updateUnits(int AValue);    
    void onPortReadFinished(int AErrorCode);
    void onPortRead(const QByteArray &AData);

signals:
    void modified();
    void childApply();
    void childReset();

private:
	Ui::PositioningMethodSerialPortOptions *ui;
    PositioningMethodSerialPort *FSerialPort;
    PortInfo *      FPortInfo;
    QMap<QString,QString> FPortInfoMap;
    QString         FPortName;
};

#endif // POSITIONINGMETHODSERIALPORTOPTIONS_H
