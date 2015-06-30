#include "wizardtransportoptions.h"
#include "ui_wizardtransportoptions.h"

WizardTransportOptions::WizardTransportOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WizardTransportOptions)
{
    ui->setupUi(this);
    connect(ui->pButStart,SIGNAL(clicked()),SLOT(onStartWizard()));
    reset();
}

WizardTransportOptions::~WizardTransportOptions()
{
    delete ui;
}


void WizardTransportOptions::apply()
{
    emit childApply();
}

void WizardTransportOptions::reset()
{
    emit childReset();
}

void WizardTransportOptions::onStartWizard()
{

}
