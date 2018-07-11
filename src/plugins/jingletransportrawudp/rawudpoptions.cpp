#include <definitions/optionvalues.h>
#include "rawudpoptions.h"
#include "ui_rawudpoptions.h"



RawUdpOptions::RawUdpOptions(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::RawUdpOptions)
{
	ui->setupUi(this);

	ui->cmbNetworkInterface->addItem(tr("Default"), QVariant());
	QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
	for (QList<QNetworkInterface>::ConstIterator it=interfaces.constBegin();
		 it!=interfaces.constEnd(); ++it)
		if ((*it).flags().testFlag(QNetworkInterface::IsUp) &&
			!(*it).flags().testFlag(QNetworkInterface::IsLoopBack))
		{
			QList<QNetworkAddressEntry> entries = (*it).addressEntries();
			for (QList<QNetworkAddressEntry>::ConstIterator ita = entries.constBegin();
				 ita!=entries.constEnd(); ++ita)
				ui->cmbNetworkInterface->addItem(QString("%1 (%2)")
													.arg((*ita).ip().toString())
													.arg((*it).humanReadableName()),
												 QVariant((*ita).ip().toString()));
		}

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
	Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_IP).setValue(ui->cmbNetworkInterface->currentData());
	Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_FIRST).setValue(ui->spbPortFrom->value());
	Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_PORT_LAST).setValue(ui->spbPortTo->value());
}

void RawUdpOptions::reset()
{
	QVariant address = Options::node(OPV_JINGLE_TRANSPORT_RAWUDP_IP).value();
	ui->cmbNetworkInterface->setCurrentIndex(ui->cmbNetworkInterface->findData(address));
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
	return AHostAddress.isLoopback();
#endif
}
