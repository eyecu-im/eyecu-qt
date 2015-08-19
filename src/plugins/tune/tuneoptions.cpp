#include <QDir>
#include <definitions/optionvalues.h>

#include "tuneoptions.h"
#include "ui_tuneoptions.h"

TuneOptions::TuneOptions(QHash<QUuid, ITuneInfoRequester *> ARequesters, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneOptions)
{
    ui->setupUi(this);
    for (QHash<QUuid, ITuneInfoRequester *>::const_iterator it=ARequesters.constBegin(); it!=ARequesters.constEnd(); it++)
        ui->cmbTuneDataRequester->addItem(it.value()->serviceIcon(), it.value()->serviceName(), it.key().toString());
    connect(ui->cmbTuneDataRequester, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
    connect(ui->cbDisplayImage, SIGNAL(stateChanged(int)), SIGNAL(modified()));
    connect(ui->cbQueryUrl, SIGNAL(stateChanged(int)), SIGNAL(modified()));
    connect(ui->pbClearCache, SIGNAL(clicked()), SIGNAL(clearCache()));
    reset();
}

TuneOptions::~TuneOptions()
{
    delete ui;
}

void TuneOptions::apply()
{
    Options::node(OPV_TUNE_INFOREQUESTER_USED).setValue(ui->cmbTuneDataRequester->itemData(ui->cmbTuneDataRequester->currentIndex()).toString());
    Options::node(OPV_TUNE_INFOREQUESTER_DISPLAYIMAGE).setValue(ui->cbDisplayImage->isChecked());
    Options::node(OPV_TUNE_INFOREQUESTER_QUERYURL).setValue(ui->cbQueryUrl->isChecked());
    emit childApply();
}

void TuneOptions::reset()
{
    ui->cmbTuneDataRequester->setCurrentIndex(ui->cmbTuneDataRequester->findData(Options::node(OPV_TUNE_INFOREQUESTER_USED).value().toString()));
    ui->cbDisplayImage->setChecked(Options::node(OPV_TUNE_INFOREQUESTER_DISPLAYIMAGE).value().toBool());
    ui->cbQueryUrl->setChecked(Options::node(OPV_TUNE_INFOREQUESTER_QUERYURL).value().toBool());
    emit childReset();
}
