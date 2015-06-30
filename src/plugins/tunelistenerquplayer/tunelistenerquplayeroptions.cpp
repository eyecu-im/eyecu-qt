#include <definitions/optionvalues.h>

#include "tunelistenerquplayeroptions.h"
#include "ui_tunelistenerquplayeroptions.h"

TuneListenerQuPlayerOptions::TuneListenerQuPlayerOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneListenerQuPlayerOptions)
{
    ui->setupUi(this);
    reset();
    connect(ui->ledPipeName, SIGNAL(textChanged(QString)), SIGNAL(modified()));
}

TuneListenerQuPlayerOptions::~TuneListenerQuPlayerOptions()
{
    delete ui;
}

void TuneListenerQuPlayerOptions::apply()
{
    Options::node(OPV_TUNE_LISTENER_QUPLAYER_PIPENAME).setValue(ui->ledPipeName->text());
    emit childApply();
}

void TuneListenerQuPlayerOptions::reset()
{
    ui->ledPipeName->setText(Options::node(OPV_TUNE_LISTENER_QUPLAYER_PIPENAME).value().toString());
    emit childReset();
}
