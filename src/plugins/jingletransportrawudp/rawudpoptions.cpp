#include <definitions/optionvalues.h>
#include "rawudpoptions.h"
#include "ui_rawudpoptions.h"



RawUdpOptions::RawUdpOptions(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::RawUdpOptions)
{
	ui->setupUi(this);

	QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
	for (QList<QHostAddress>::ConstIterator it = addresses.constBegin(); it!=addresses.constEnd(); ++it)
		if (!isLoopback(*it))
			ui->cmbNetworkInterface->addItem((*it).toString());

	connect(ui->cmbNetworkInterface, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->spbPortFrom, SIGNAL(valueChanged(int)), SIGNAL(modified()));
	connect(ui->spbPortTo, SIGNAL(valueChanged(int)), SIGNAL(modified()));
	reset();
}

RawUdpOptions::~RawUdpOptions()
{
	delete ui;
}

void RawUdpOptions::apply()
{
	Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_IP).setValue(ui->cmbNetworkInterface->currentText());
	Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST).setValue(ui->spbPortFrom->value());
	Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST).setValue(ui->spbPortTo->value());
}

void RawUdpOptions::reset()
{
	ui->cmbNetworkInterface->setCurrentIndex(ui->cmbNetworkInterface->findText(Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_IP).value().toString()));
	ui->spbPortFrom->setValue(Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST).value().toInt());
	ui->spbPortTo->setValue(Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST).value().toInt());
}

bool RawUdpOptions::isLoopback(const QHostAddress &AHostAddress)
{
#if QT_VERSION < 0x050000
	if (AHostAddress.protocol()==QAbstractSocket::IPv4Protocol)
	{
		quint32 address = AHostAddress.toIPv4Address();
		return address>=0x7f000000 && address<=0x7fffffff;
	}
	else
		return AHostAddress == QHostAddress("0:0:0:0:0:0:0:1");
#else
	return AHostAddress.isLooback();
#endif
}
