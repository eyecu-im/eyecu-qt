#include <definitions/optionvalues.h>

#include "tunelistenerzoptions.h"
#include "ui_tunelistenerzoptions.h"

TuneListenerZOptions::TuneListenerZOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneListenerZOptions)
{
    ui->setupUi(this);
    reset();
    connect(ui->ledPipeName, SIGNAL(textChanged(QString)), SIGNAL(modified()));
}

TuneListenerZOptions::~TuneListenerZOptions()
{
    delete ui;
}

void TuneListenerZOptions::apply()
{
    Options::node(OPV_TUNE_LISTENER_Z_PIPENAME).setValue(ui->ledPipeName->text());
    emit childApply();
}

void TuneListenerZOptions::reset()
{
    ui->ledPipeName->setText(Options::node(OPV_TUNE_LISTENER_Z_PIPENAME).value().toString());
    emit childReset();
}
