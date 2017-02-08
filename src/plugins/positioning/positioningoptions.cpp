#include <definitions/optionnodes.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>
#include <utils/iconstorage.h>
#include "positioningoptions.h"

PositioningOptions::PositioningOptions(IPositioning *APosition, QHash<QUuid, IPositioningMethod *> ASenderList, IOptionsManager *AOptionsManager, QWidget *parent) :
    QWidget(parent),
	ui(new Ui::PositioningOptions),
	FOptionsManager(AOptionsManager)
{
	Q_UNUSED(APosition)

    ui->setupUi(this);
	IconStorage *menuicons = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	ui->cBoxSource->addItem(menuicons->getIcon(MNI_GEOLOC_OFF), tr("No positioning"), QUuid().toString());
    for (QHash<QUuid, IPositioningMethod *>::ConstIterator it=ASenderList.constBegin(); it!=ASenderList.constEnd(); it++)
		ui->cBoxSource->addItem(menuicons->getIcon((*it)->iconId()), (*it)->name(), it.key().toString());

	ui->pbOptions->setIcon(menuicons->getIcon(MNI_OPTIONS_DIALOG));

    connect(ui->cBoxSource,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    reset();
}

PositioningOptions::~PositioningOptions()
{
    delete ui;
}

void PositioningOptions::apply()
{    
    emit childApply();
    Options::node(OPV_POSITIONING_METHOD).setValue(ui->cBoxSource->itemData(ui->cBoxSource->currentIndex()).toString());
}

void PositioningOptions::reset()
{
    ui->cBoxSource->setCurrentIndex(ui->cBoxSource->findData(Options::node(OPV_POSITIONING_METHOD).value().toString()));
    emit childReset();
}

void PositioningOptions::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
	}
}

void PositioningOptions::onOptionsClicked()
{
	FOptionsManager->showOptionsDialog(OPN_GEOLOC"."+ui->cBoxSource->itemData(ui->cBoxSource->currentIndex()).toString(), OPN_GEOLOC, window());
}

void PositioningOptions::onMethodSelected(int AIndex)
{
	ui->pbOptions->setEnabled(AIndex);
}
