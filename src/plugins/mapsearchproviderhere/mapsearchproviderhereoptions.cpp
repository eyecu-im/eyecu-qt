#include <definitions/optionvalues.h>
#include "mapsearchproviderhere.h"
#include "mapsearchproviderhereoptions.h"
#include "ui_mapsearchproviderhereoptions.h"

MapSearchProviderHereOptions::MapSearchProviderHereOptions(MapSearchProviderHere *AMapSearchProviderHere,
														   QWidget *AParent):
	QWidget(AParent),
	ui(new Ui::MapSearchProviderHereOptions),
	FMapSearchProviderHere(AMapSearchProviderHere)
{
	ui->setupUi(this);

	QList<QLocale::Country> countries(FMapSearchProviderHere->countries());
	ui->cmbPView->addItem(tr("For current locale (%1)")
							.arg(QLocale::countryToString(QLocale().country())));
	for (QList<QLocale::Country>::ConstIterator it=countries.constBegin();
		 it != countries.constEnd(); ++it)
	{
		QString cntry(*it == QLocale::AnyCountry?tr("Default")
												:QLocale::countryToString(*it));
		ui->cmbPView->addItem(cntry, FMapSearchProviderHere->country(*it));
	}

	reset();

	connect(ui->cmbPView, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
}

MapSearchProviderHereOptions::~MapSearchProviderHereOptions()
{
	delete ui;
}

void MapSearchProviderHereOptions::apply()
{
	Options::node(OPV_MAP_SEARCH_PROVIDER_HERE_POLITICALVIEW).setValue(ui->cmbPView->currentData().toString());

	emit childApply();
}

void MapSearchProviderHereOptions::reset()
{
	QString pview = Options::node(OPV_MAP_SEARCH_PROVIDER_HERE_POLITICALVIEW).value().toString();
	int index = pview.isEmpty()?0:ui->cmbPView->findData(pview);
	if (index<0)
		index = 0;
	ui->cmbPView->setCurrentIndex(index);

	emit childReset();
}

void MapSearchProviderHereOptions::changeEvent(QEvent *e)
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
