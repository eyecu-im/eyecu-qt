#include "definitions/optionvalues.h"

#include "avataroptionswidget.h"
#include "ui_avataroptionswidget.h"

AvatarOptionsWidget::AvatarOptionsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AvatarOptionsWidget)
{
    ui->setupUi(this);
    reset();
	connect(ui->cbShow, SIGNAL(stateChanged(int)), SIGNAL(modified()));
	connect(ui->cbGrayscaledOffline, SIGNAL(stateChanged(int)), SIGNAL(modified()));
	connect(ui->cbShowEmpty, SIGNAL(stateChanged(int)), SIGNAL(modified()));
	connect(ui->cmbAvatarPosition, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
}

AvatarOptionsWidget::~AvatarOptionsWidget()
{
    delete ui;
}

void AvatarOptionsWidget::apply()
{	
	Options::node(OPV_ROSTER_AVATARS_DISPLAY).setValue(ui->cbShow->isChecked());
	Options::node(OPV_ROSTER_AVATARS_DISPLAYEMPTY).setValue(ui->cbShowEmpty->isChecked());
	Options::node(OPV_ROSTER_AVATARS_DISPLAYGRAY).setValue(ui->cbGrayscaledOffline->isChecked());
	Options::node(OPV_ROSTER_AVATARS_POSITION).setValue(ui->cmbAvatarPosition->currentIndex());
    emit childApply();
}

void AvatarOptionsWidget::reset()
{
	ui->cbShow->setChecked(Options::node(OPV_ROSTER_AVATARS_DISPLAY).value().toBool());
	ui->cbShowEmpty->setChecked(Options::node(OPV_ROSTER_AVATARS_DISPLAYEMPTY).value().toBool());
	ui->cbGrayscaledOffline->setChecked(Options::node(OPV_ROSTER_AVATARS_DISPLAYGRAY).value().toBool());
	ui->cmbAvatarPosition->setCurrentIndex(Options::node(OPV_ROSTER_AVATARS_POSITION).value().toInt());

    emit childReset();
}

void AvatarOptionsWidget::changeEvent(QEvent *e)
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
