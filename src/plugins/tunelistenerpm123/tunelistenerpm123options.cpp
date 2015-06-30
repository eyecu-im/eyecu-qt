#include <definitions/optionvalues.h>

#include "tunelistenerpm123options.h"
#include "ui_tunelistenerpm123options.h"

TuneListenerPm123Options::TuneListenerPm123Options(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TuneListenerPm123Options)
{
    ui->setupUi(this);
    reset();
    connect(ui->ledPipeName, SIGNAL(textChanged(QString)), SIGNAL(modified()));
}

TuneListenerPm123Options::~TuneListenerPm123Options()
{
    delete ui;
}

void TuneListenerPm123Options::apply()
{
    Options::node(OPV_TUNE_LISTENER_PM123_PIPENAME).setValue(ui->ledPipeName->text());
    emit childApply();
}

void TuneListenerPm123Options::reset()
{
    ui->ledPipeName->setText(Options::node(OPV_TUNE_LISTENER_PM123_PIPENAME).value().toString());
    emit childReset();
}
