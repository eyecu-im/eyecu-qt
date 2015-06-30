#include <definitions/optionvalues.h>
#include "multimediaplayeroptions.h"
#include "ui_multimediaplayeroptions.h"

MultimediaPlayerOptions::MultimediaPlayerOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MultimediaPlayerOptions)
{
    ui->setupUi(this);
    reset();

    connect(ui->chkSmoothResize,SIGNAL(toggled(bool)),SIGNAL(modified()));
    connect(ui->cmbAspectRatioMode, SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
}

MultimediaPlayerOptions::~MultimediaPlayerOptions()
{
    delete ui;
}

void MultimediaPlayerOptions::apply()
{
    Options::node(OPV_MMPLAYER_SMOOTHRESIZE).setValue(ui->chkSmoothResize->isChecked());
    Options::node(OPV_MMPLAYER_ASPECTRATIOMODE).setValue(ui->cmbAspectRatioMode->currentIndex());
    emit childApply();
}

void MultimediaPlayerOptions::reset()
{
    ui->chkSmoothResize->setChecked(Options::node(OPV_MMPLAYER_SMOOTHRESIZE).value().toBool());
    ui->cmbAspectRatioMode->setCurrentIndex(Options::node(OPV_MMPLAYER_ASPECTRATIOMODE).value().toInt());
    emit childReset();
}
