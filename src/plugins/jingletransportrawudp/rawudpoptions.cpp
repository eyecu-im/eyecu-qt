#include "rawudpoptions.h"
#include "ui_rawudpoptions.h"

RawUdpOptions::RawUdpOptions(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::RawUdpOptions)
{
	ui->setupUi(this);

	reset();
}

RawUdpOptions::~RawUdpOptions()
{
	delete ui;
}

void RawUdpOptions::apply()
{

}

void RawUdpOptions::reset()
{

}
