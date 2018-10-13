#include <definitions/optionvalues.h>
#include "avatarsizeoptionswidget.h"
#include "ui_avatarsizeoptionswidget.h"

AvatarSizeOptionsWidget::AvatarSizeOptionsWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::AvatarSizeOptionsWidget)
{
	ui->setupUi(this);
	reset();
	connect(ui->cbDisplayEmpty, SIGNAL(stateChanged(int)), SIGNAL(modified()));
	connect(ui->cbAspectCrop, SIGNAL(stateChanged(int)), SIGNAL(modified()));
}

AvatarSizeOptionsWidget::~AvatarSizeOptionsWidget()
{
	delete ui;
}

void AvatarSizeOptionsWidget::apply()
{
	Options::node(OPV_AVATARS_DISPLAYEMPTY).setValue(ui->cbDisplayEmpty->isChecked());
	Options::node(OPV_AVATARS_ASPECTCROP).setValue(ui->cbAspectCrop->isChecked());
	Options::node(OPV_AVATARS_SMALLSIZE).setValue(ui->spbSmall->value());
	Options::node(OPV_AVATARS_NORMALSIZE).setValue(ui->spbNormal->value());
	Options::node(OPV_AVATARS_LARGESIZE).setValue(ui->spbLarge->value());
}

void AvatarSizeOptionsWidget::reset()
{
	ui->cbDisplayEmpty->setChecked(Options::node(OPV_AVATARS_DISPLAYEMPTY).value().toBool());
	ui->cbAspectCrop->setChecked(Options::node(OPV_AVATARS_ASPECTCROP).value().toBool());
	ui->spbSmall->setValue(Options::node(OPV_AVATARS_SMALLSIZE).value().toInt());
	ui->spbNormal->setValue(Options::node(OPV_AVATARS_NORMALSIZE).value().toInt());
	ui->spbLarge->setValue(Options::node(OPV_AVATARS_LARGESIZE).value().toInt());
}

void AvatarSizeOptionsWidget::onEditingFinished()
{
	if (sender() == ui->spbSmall)
		ui->spbNormal->setMinimum(ui->spbSmall->value());
	else if (sender() == ui->spbNormal)
	{
		ui->spbSmall->setMaximum(ui->spbNormal->value());
		ui->spbLarge->setMinimum(ui->spbNormal->value());
	}
	else if (sender() == ui->spbLarge)
		ui->spbNormal->setMaximum(ui->spbLarge->value());

	emit modified();
}

void AvatarSizeOptionsWidget::changeEvent(QEvent *e)
{
	QWidget::changeEvent(e);
	switch (e->type())
	{
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}
