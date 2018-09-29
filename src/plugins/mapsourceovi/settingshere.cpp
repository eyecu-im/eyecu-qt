#include <SourceOvi>
#include <definitions/optionvalues.h>
#include "settingshere.h"
#include "ui_settingshere.h"

SettingsHere::SettingsHere(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::SettingsHere)
{
	ui->setupUi(this);

	QStringList langs(SourceOvi::langs());

	ui->cmbPrimary->addItem(tr("Default (%1)")
							.arg(QLocale::languageToString(QLocale().language())));
	ui->cmbPrimary->addItem(tr("Multiple"), QString("mul"));
	ui->cmbSecondary->addItem(tr("None"));
	ui->cmbSecondary->addItem(tr("Multiple"), QString("mul"));
	for (QStringList::ConstIterator it=langs.constBegin();
		 it != langs.constEnd(); ++it)
	{
		QString lang(QLocale::languageToString(QLocale(*it).language()));
		QString lcode(SourceOvi::lang(*it));
		ui->cmbPrimary->addItem(lang, lcode);
		ui->cmbSecondary->addItem(lang, lcode);
	}

	QList<QLocale::Country> countries(SourceOvi::countries());
	ui->cmbPView->addItem(tr("For current locale (%1)")
							.arg(QLocale::countryToString(QLocale().country())));
	for (QList<QLocale::Country>::ConstIterator it=countries.constBegin();
		 it != countries.constEnd(); ++it)
	{
		QString cntry(*it == QLocale::AnyCountry?tr("Default")
												:QLocale::countryToString(*it));
		ui->cmbPView->addItem(cntry, SourceOvi::country(*it));
	}

	connect(ui->cmbPrimary, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->cmbSecondary, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));
	connect(ui->rbDay, SIGNAL(toggled(bool)), SIGNAL(modified()));
	connect(ui->cmbPView, SIGNAL(currentIndexChanged(int)), SIGNAL(modified()));

	reset();
}

SettingsHere::~SettingsHere()
{
	delete ui;
}

void SettingsHere::apply()
{
	Options::node(OPV_MAP_SOURCE_HERE_LANG_PRIMARY).setValue(ui->cmbPrimary->currentData().toString());
	Options::node(OPV_MAP_SOURCE_HERE_LANG_SECONDARY).setValue(ui->cmbSecondary->currentData().toString());
	Options::node(OPV_MAP_SOURCE_HERE_POLITICALVIEW).setValue(ui->cmbPView->currentData().toString());
	Options::node(OPV_MAP_SOURCE_HERE_MODE_NIGHT).setValue(ui->rbNight->isChecked());

	emit childApply();
}

void SettingsHere::reset()
{
	QString lg = Options::node(OPV_MAP_SOURCE_HERE_LANG_PRIMARY).value().toString();
	int index = lg.isEmpty()?0:ui->cmbPrimary->findData(lg);
	if (index<0)
		index = 0;
	ui->cmbPrimary->setCurrentIndex(index);

	lg = Options::node(OPV_MAP_SOURCE_HERE_LANG_SECONDARY).value().toString();
	index = lg.isEmpty()?0:ui->cmbSecondary->findData(lg);
	if (index<0)
		index = 0;
	ui->cmbSecondary->setCurrentIndex(index);

	QString pview = Options::node(OPV_MAP_SOURCE_HERE_POLITICALVIEW).value().toString();
	index = pview.isEmpty()?0:ui->cmbPView->findData(pview);
	if (index<0)
		index = 0;
	ui->cmbPView->setCurrentIndex(index);

	if (Options::node(OPV_MAP_SOURCE_HERE_MODE_NIGHT).value().toBool())
		ui->rbNight->setChecked(true);
	else
		ui->rbDay->setChecked(true);

	emit childReset();
}

void SettingsHere::changeEvent(QEvent *e)
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
