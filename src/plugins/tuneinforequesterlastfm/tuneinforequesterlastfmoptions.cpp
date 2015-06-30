#include <definitions/optionvalues.h>
#include "tuneinforequesterlastfmoptions.h"
#include "ui_tuneinforequesterlastfmoptions.h"

TuneInfoRequesterLastFmOptions::TuneInfoRequesterLastFmOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneInfoRequesterLastFmOptions)
{
    ui->setupUi(this);
    connect(ui->cmbImageSize, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
    connect(ui->chkAutocorrect, SIGNAL(toggled(bool)), SIGNAL(modified()));
    reset();
}

TuneInfoRequesterLastFmOptions::~TuneInfoRequesterLastFmOptions()
{
    delete ui;
}

void TuneInfoRequesterLastFmOptions::apply()
{
    Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_IMAGESIZE).setValue(ui->cmbImageSize->currentIndex());
    Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_IMAGESIZE).setValue(ui->chkAutocorrect->isChecked());
    emit childApply();
}

void TuneInfoRequesterLastFmOptions::reset()
{
    ui->cmbImageSize->setCurrentIndex(Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_IMAGESIZE).value().toInt());
    ui->chkAutocorrect->setChecked(Options::node(OPV_TUNE_INFOREQUESTER_LASTFM_IMAGESIZE).value().toBool());
    emit childReset();
}
