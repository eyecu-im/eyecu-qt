#include <definitions/optionvalues.h>
#include "avatarsizeoptionswidget.h"
#include "ui_avatarsizeoptionswidget.h"

AvatarSizeOptionsWidget::AvatarSizeOptionsWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::AvatarSizeOptionsWidget)
{
	ui->setupUi(this);
	reset();
}

AvatarSizeOptionsWidget::~AvatarSizeOptionsWidget()
{
	delete ui;
}

void AvatarSizeOptionsWidget::apply()
{
	Options::node(OPV_AVATARS_SMALLSIZE).setValue(ui->spbSmall->value());
	Options::node(OPV_AVATARS_NORMALSIZE).setValue(ui->spbNormal->value());
	Options::node(OPV_AVATARS_LARGESIZE).setValue(ui->spbLarge->value());
}

void AvatarSizeOptionsWidget::reset()
{
	ui->spbSmall->setValue(Options::node(OPV_AVATARS_SMALLSIZE).value().toInt());
	ui->spbNormal->setValue(Options::node(OPV_AVATARS_NORMALSIZE).value().toInt());
	ui->spbLarge->setValue(Options::node(OPV_AVATARS_LARGESIZE).value().toInt());
}

void AvatarSizeOptionsWidget::onValueChanged(int AValue)
{
	if (sender() == ui->spbSmall)
		ui->spbNormal->setMinimum(AValue);
	else if (sender() == ui->spbNormal)
	{
		ui->spbSmall->setMaximum(AValue);
		ui->spbLarge->setMinimum(AValue);
	}
	else if (sender() == ui->spbLarge)
		ui->spbNormal->setMaximum(AValue);

	emit modified();
}
