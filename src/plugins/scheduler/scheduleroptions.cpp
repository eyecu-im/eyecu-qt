#include "scheduleroptions.h"
#include "ui_scheduleroptions.h"

SchedulerOptions::SchedulerOptions(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SchedulerOptions)
{
	ui->setupUi(this);
}

SchedulerOptions::~SchedulerOptions()
{
	delete ui;
}

void SchedulerOptions::apply()
{
	emit childApply();
}

void SchedulerOptions::reset()
{
	emit childReset();
}
