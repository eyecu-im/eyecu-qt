#include <QImageReader>

#include "xhtmloptions.h"
#include "xhtmlim.h"

XhtmlOptions::XhtmlOptions(XhtmlIm *AXhtmlIm, QWidget *AParent):
    QWidget(AParent),
	ui(new Ui::XhtmlOptions),
	FXhtmlIm(AXhtmlIm)
{
    ui->setupUi(this);
    QList<QByteArray> formats=QImageReader::supportedImageFormats();
    for (QList<QByteArray>::const_iterator it=formats.constBegin(); it!=formats.constEnd(); it++)
        ui->cmbDefImageFormat->addItem(QString(*it).toUpper(), *it);

	connect(ui->spbMaxAge,SIGNAL(valueChanged(int)),SIGNAL(modified()));
    connect(ui->spbMaxEmbedDataSize,SIGNAL(valueChanged(int)),SIGNAL(modified()));
    connect(ui->cmbAgeUnits,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));
    connect(ui->cmbDefImageFormat,SIGNAL(currentIndexChanged(int)),SIGNAL(modified()));	
    reset();
	onMaxAgeChanged(ui->spbMaxAge->value());
	onMaxEmbedChanged(ui->spbMaxEmbedDataSize->value());
}

XhtmlOptions::~XhtmlOptions(){delete ui;}

void XhtmlOptions::apply()
{
	qulonglong maxAge = ui->spbMaxAge->value();
	switch(ui->cmbAgeUnits->currentIndex())
	{
		case XhtmlIm::Years:   maxAge *= 30780000; break;
		case XhtmlIm::Months:  maxAge *= 2565000; break;
		case XhtmlIm::Weeks:   maxAge *= 7;
		case XhtmlIm::Days:    maxAge *= 24;
		case XhtmlIm::Hours:   maxAge *= 60;
		case XhtmlIm::Minutes: maxAge *= 60;
		case XhtmlIm::Seconds: break;
	}
	Options::node(OPV_XHTML_MAXAGE).setValue(maxAge);
    Options::node(OPV_XHTML_EMBEDSIZE).setValue(ui->spbMaxEmbedDataSize->value());
	Options::node(OPV_XHTML_DEFAULTIMAGEFORMAT).setValue(ui->cmbDefImageFormat->itemData(ui->cmbDefImageFormat->currentIndex()));
    emit childApply();
}

void XhtmlOptions::reset()
{
	int maxAge = Options::node(OPV_XHTML_MAXAGE).value().toLongLong();
	int	units = maxAge/30780000*30780000==maxAge?XhtmlIm::Years:
				maxAge/2565000*2565000==maxAge?XhtmlIm::Months:
				maxAge/604800*604800==maxAge?XhtmlIm::Weeks:
				maxAge/86400*86400==maxAge?XhtmlIm::Days:
				maxAge/3600*3600==maxAge?XhtmlIm::Hours:
				maxAge/60*60==maxAge?XhtmlIm::Minutes:
				XhtmlIm::Seconds;

	switch(units)
	{
		case XhtmlIm::Years:   maxAge /= 30780000; break;
		case XhtmlIm::Months:  maxAge /= 2565000; break;
		case XhtmlIm::Weeks:   maxAge /= 7;
		case XhtmlIm::Days:    maxAge /= 24;
		case XhtmlIm::Hours:   maxAge /= 60;
		case XhtmlIm::Minutes: maxAge /= 60;
		case XhtmlIm::Seconds: break;
	}

	ui->spbMaxAge->setValue(maxAge);

	ui->cmbAgeUnits->setCurrentIndex(units);	
    ui->spbMaxEmbedDataSize->setValue(Options::node(OPV_XHTML_EMBEDSIZE).value().toInt());	
	ui->cmbDefImageFormat->setCurrentIndex(ui->cmbDefImageFormat->findData(Options::node(OPV_XHTML_DEFAULTIMAGEFORMAT).value()));
	emit childReset();
}

void XhtmlOptions::onMaxEmbedChanged(int AValue)
{
	ui->lblBytes->setText(tr("byte(s)", "", AValue));
}

void XhtmlOptions::onMaxAgeChanged(int AValue)
{
	FXhtmlIm->updateUnitsComboBox(ui->cmbAgeUnits, AValue);
}
