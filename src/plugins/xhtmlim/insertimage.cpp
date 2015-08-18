#include <QMimeData>
#include <QNetworkReply>
#include <QImageWriter>
#include <QBuffer>
#include <QMovie>
#include <QFileDialog>
#include <QClipboard>

#include <definitions/optionvalues.h>
#include <definitions/shortcuts.h>
#include <utils/options.h>
#include <utils/logger.h>

#include "insertimage.h"
#include "xhtmlim.h"

IBitsOfBinary * InsertImage::FBitsOfBinary = NULL;

InsertImage::InsertImage(XhtmlIm *AXhtmlIm, QNetworkAccessManager *ANetworkAccessManager, const QByteArray &AImageData, const QUrl &AImageUrl, const QSize &AImageSize, const QString &AAlternativeText, QWidget *parent) :
    QDialog(parent),    
    ui(new Ui::InsertImage),        
    FUrlCurrent(AImageUrl),
    FOriginalImageData(AImageData),
    FNetworkAccessManager(ANetworkAccessManager),
	FXhtmlIm(AXhtmlIm),
    FSchemeMasks(QStringList() << "http" << "https" << "ftp" << "file"),
    FIgnoreSpbW(false),
    FIgnoreSpbH(false),
    FIgnoreDspbW(false),
    FIgnoreDspbH(false)
{
    ui->setupUi(this);

    ui->lblInfo->text().clear();
    ui->pbLoad->setDisabled(true);

	qulonglong maxAge = Options::node(OPV_XHTML_MAXAGE).value().toLongLong();
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
	ui->cmbMaxAgeUnits->setCurrentIndex(units);
    ui->ledUrl->setFocus();

    Shortcuts::bindObjectShortcut(SCT_MESSAGEWINDOWS_XHTMLIM_INSERTIMAGEDIALOG_BROWSE, ui->pbBrowse);

    FWriterFormats = QImageWriter::supportedImageFormats();
    FReaderFormats = QImageReader::supportedImageFormats();
    ui->cmbType->blockSignals(true);
    for (QList<QByteArray>::const_iterator it=FWriterFormats.constBegin(); it!=FWriterFormats.constEnd(); it++)
        ui->cmbType->addItem(QString(*it).toUpper(), *it);
    ui->cmbType->blockSignals(false);

    if (FUrlCurrent.isEmpty())
    {
        QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *data = clipboard->mimeData();
        if (data)
        {
            if (data->hasHtml())
            {
                QDomDocument doc;
                doc.setContent(data->html());
                QDomElement root = doc.documentElement();
                if (root.tagName()=="img" && root.hasAttribute("src"))
                {
					FUrlCurrent = QUrl::fromEncoded(root.attribute("src").toLatin1());
                    if (root.hasAttribute("alt"))
                        ui->ledAlt->setText(root.attribute("alt"));
                }
            }

            if (!FUrlCurrent.isValid())
                if (data->hasText())
                    FUrlCurrent = QUrl::fromUserInput(data->text());

			if (!FUrlCurrent.isValid())
				if (data->hasUrls())
				{
					QList<QUrl> urls = data->urls();
					if (urls.size()==1)
						FUrlCurrent = urls.first();
				}

            if (!FUrlCurrent.isValid() || !FSchemeMasks.contains(FUrlCurrent.scheme()))
                FUrlCurrent.clear();

            if (data->hasImage())
            {
                QByteArray format;
                if (FUrlCurrent.isValid())
                    format = QImageReader::imageFormat(FUrlCurrent.path());
                if (format.isEmpty())
                    format = Options::node(OPV_XHTML_DEFIMAGEFORMAT).value().toByteArray();
                QImage image = data->imageData().value<QImage>();
                QBuffer buffer(&FOriginalImageData);
                image.save(&buffer, format.data());
            }
        }
    }

    FImageData = FOriginalImageData;

    if (FUrlCurrent.isValid())
        ui->ledUrl->setText(FUrlCurrent.toString());
    else if (!FOriginalImageData.isEmpty() && FBitsOfBinary)
        calculateUrl(FOriginalImageData);

    ui->ledUrl->selectAll();

    if (!AAlternativeText.isEmpty())
        ui->ledAlt->setText(AAlternativeText);    

    if (!FOriginalImageData.isEmpty())
        readImageData(FUrlCurrent);
    else
        if (ui->lblImage->pixmap())
        {
            ui->pbInsert->setEnabled(true);
            ui->ledAlt->setFocus();
        }
        else
        {
            disableCommon();
            disableBOB();
        }

    ui->ledUrl->selectAll();

    if (AImageSize.isValid() && !AImageSize.isNull())
	{
        ui->spbWidth->setValue(AImageSize.width());
        ui->spbHeight->setValue(AImageSize.height());         
        QSize size=ui->lblImage->size();
        ui->chbKeepAspect->setChecked(size.width()*FSizeOld.height()==size.height()*FSizeOld.width());    // Check, if aspect ratio is the same
    }
    else
    {
        ui->spbWidth->setValue(FSizeOld.width());
        ui->spbHeight->setValue(FSizeOld.height());
        ui->chbKeepAspect->setChecked(true);
    }
}

InsertImage::~InsertImage()
{
    delete ui;
}

int InsertImage::getMaxAge() const
{
	int maxAge=ui->spbMaxAge->value();

    switch(ui->cmbMaxAgeUnits->currentIndex())
    {
        case XhtmlIm::Years:   maxAge *= 30780000; break;
        case XhtmlIm::Months:  maxAge *= 2565000; break;
        case XhtmlIm::Weeks:   maxAge *= 7;
        case XhtmlIm::Days:    maxAge *= 24;
        case XhtmlIm::Hours:   maxAge *= 60;
        case XhtmlIm::Minutes: maxAge *= 60;
        case XhtmlIm::Seconds: break;
    }
    return maxAge;
}

bool InsertImage::isRemote() const
{
    QString scheme=FUrlCurrent.scheme();
    return scheme=="ftp"||scheme=="http"||scheme=="https";
}

void InsertImage::onSchemeChanged(int AIndex)
{
    QString uri = ui->ledUrl->text();
    int   comma = uri.indexOf("://");
    if (comma!=-1)
    for (int i=0; i<comma; i++)
        if (uri[i]<'A' || (uri[i]>'Z' && uri[i]<'a') || uri[i]>'z')
        {
            comma=-1;
            break;
        }

	if (AIndex)
    {
        if (comma==-1)
        {
            if (!uri.startsWith("//"))
                uri.prepend("//");
            uri.prepend(":");
        }
        else
            uri.remove(0, comma);
		uri.prepend(FSchemeMasks[AIndex-1]);
    }
    else
        if (comma!=-1)
            uri.remove(0, comma+1);

    ui->ledUrl->setText(uri);
}

void InsertImage::onCheckBoxKeepAspect(int AState)
{
	if (AState==Qt::Checked)
    {
        double r=((double)ui->spbHeight->value()/FSizeOld.height()+(double)ui->spbWidth->value()/FSizeOld.width())/2;
        ui->spbHeight->blockSignals(true);
        ui->spbHeight->setValue(FSizeOld.height()*r);
        ui->spbHeight->blockSignals(false);
        ui->spbWidth->blockSignals(true);
        ui->spbWidth->setValue(FSizeOld.width()*r);
        ui->spbWidth->blockSignals(false);

        ui->dspbHeight->blockSignals(true);
        ui->dspbHeight->setValue(100*r);
        ui->dspbHeight->blockSignals(false);
        ui->dspbWidth->blockSignals(true);
        ui->dspbWidth->setValue(100*r);
        ui->dspbWidth->blockSignals(false);
    }
}

void InsertImage::onCheckBoxPhysResize(int AState)
{
	if (AState==Qt::Checked)
    {
        if (ui->cmbType->itemData(0).toByteArray().isNull())
        {
            ui->cmbType->removeItem(0);
            ui->cmbType->setCurrentIndex(ui->cmbType->findData(FOriginalFormat));
        }
    }
    else
    {
        if (!ui->cmbType->itemData(0).toByteArray().isNull())
        {
            ui->cmbType->insertItem(0, tr("Do not change"));
            ui->cmbType->setCurrentIndex(0);
        }
    }
}

void InsertImage::onSpbWidth(int AWidth)
{
	ui->spbWidth->setSuffix(tr("pixels", "", AWidth));
    FIgnoreSpbW=true;
    if (!FIgnoreDspbW)
		ui->dspbWidth->setValue(100.0*AWidth/FSizeOld.width());
    if(ui->chbKeepAspect->isChecked())
        if (!FIgnoreSpbH)
			ui->spbHeight->setValue(AWidth*FSizeOld.height()/FSizeOld.width());
	updateSpinboxPixels(ui->spbWidth);
    FIgnoreSpbW=false;
}

void InsertImage::onSpbHeight(int AHeight)
{
    FIgnoreSpbH=true;
    if (!FIgnoreDspbH)
		ui->dspbHeight->setValue(100.0*AHeight/FSizeOld.height());
    if(ui->chbKeepAspect->isChecked())
        if (!FIgnoreSpbW)
			ui->spbWidth->setValue(AHeight*FSizeOld.width()/FSizeOld.height());
	updateSpinboxPixels(ui->spbHeight);
    FIgnoreSpbH=false;
}

void InsertImage::onDSpbWidth(double AWidth)
{
    FIgnoreDspbW=true;
    if (!FIgnoreSpbW)
		ui->spbWidth->setValue(AWidth*FSizeOld.width()/100);
    if(ui->chbKeepAspect->isChecked())
        if (!FIgnoreDspbH)
			ui->dspbHeight->setValue(AWidth);
    FIgnoreDspbW=false;
}

void InsertImage::onDSpbHeight(double AHeight)
{
    FIgnoreDspbH=true;
    if (!FIgnoreSpbH)
		ui->spbHeight->setValue(AHeight*FSizeOld.height()/100);
    if(ui->chbKeepAspect->isChecked())
        if (!FIgnoreDspbW)
			ui->dspbWidth->setValue(AHeight);
    FIgnoreDspbH=false;
}

void InsertImage::onImageSettingsChanged()
{
    if (!isRemote())
        recalculateImageData();
}

void InsertImage::onMaxAgeChanged(int AValue)
{
	FXhtmlIm->updateUnitsComboBox(ui->cmbMaxAgeUnits, AValue);
}

void InsertImage::onTextChanged()
{
    QUrl url = QUrl::fromUserInput(ui->ledUrl->text());
    int index = ui->ledUrl->text().indexOf("://");
    QString scheme = ui->ledUrl->text().left(index);
    ui->cmbScheme->blockSignals(true);
    ui->cmbScheme->setCurrentIndex(FSchemeMasks.indexOf(scheme)+1);
    ui->cmbScheme->blockSignals(false);
    url.setScheme(scheme);
    ui->pbLoad->setEnabled(url.isValid());
    enableInsert();
}

void InsertImage::onButtonLoad()
{
    ui->cmbScheme->setEnabled(true);
    startLoadFile(QUrl::fromUserInput(ui->ledUrl->text()));
}

void InsertImage::onButtonBrowse()
{
    QString fileTypes;

    for (QList<QByteArray>::const_iterator it=FReaderFormats.constBegin(); it!=FReaderFormats.constEnd(); it++)
    {
        if (it!=FReaderFormats.constBegin())
            fileTypes.append(" ");
        fileTypes.append("*.").append(QString(*it));
    }
    QString fileName = QFileDialog::getOpenFileName(this,
						tr("Please, choose image file"), Options::node(OPV_XHTML_IMAGEOPENDIRECTORY).value().toString(),tr("Images (%1)").arg(fileTypes));
    if (!fileName.isNull())
    {
		Options::node(OPV_XHTML_IMAGEOPENDIRECTORY).setValue(QFileInfo(fileName).filePath());
        QUrl url = QUrl::fromLocalFile(fileName);
        ui->cmbScheme->setCurrentIndex(0);        
        ui->ledUrl->setText(url.toString());
        startLoadFile(url);
    }
}

void InsertImage::startLoadFile(const QUrl &AUrl)
{
    ui->pbLoad->setEnabled(false);
    ui->pbBrowse->setEnabled(false);
    ui->pbInsert->setEnabled(false);
    QNetworkReply *reply=FNetworkAccessManager->get(QNetworkRequest(AUrl));
    connect(reply, SIGNAL(finished()), SLOT(onLoadFinished()));
}

void InsertImage::disableCommon(bool ADisable)
{
    ui->pbInsert->setDisabled(ADisable);
    ui->chbKeepAspect->setDisabled(ADisable);
    ui->spbHeight->setDisabled(ADisable);
    ui->spbWidth->setDisabled(ADisable);
    ui->dspbHeight->setDisabled(ADisable);
    ui->dspbWidth->setDisabled(ADisable);
}

void InsertImage::disableBOB(bool ADisable)
{
    ui->spbMaxAge->setDisabled(ADisable);
    ui->cmbMaxAgeUnits->setDisabled(ADisable);
    ui->cmbType->setDisabled(ADisable);
    ui->chbEmbed->setDisabled(ADisable);
    ui->chbPhysResize->setDisabled(ADisable);
}

void InsertImage::readImageData(const QUrl &AUrl)
{
    if (FOriginalImageData.isEmpty())
    {
        ui->lblInfo->setText(tr("Error: image data is empty!"));
        disableCommon();
        disableBOB();
    }
    else
    {
		QBuffer buffer(&FOriginalImageData);
		QImageReader reader(&buffer);
        FOriginalFormat=reader.format();
        QSize size=reader.size();
        FMimeType=(FOriginalFormat.isEmpty())?QString():QString("image/").append(QString(FOriginalFormat));
        FSizeOld=size;
        ui->spbWidth->blockSignals(true);
        ui->spbWidth->setValue(size.width());
		updateSpinboxPixels(ui->spbWidth);
        ui->spbWidth->blockSignals(false);

        ui->spbHeight->blockSignals(true);
        ui->spbHeight->setValue(size.height());
		updateSpinboxPixels(ui->spbHeight);
        ui->spbHeight->blockSignals(false);

        ui->dspbWidth->blockSignals(true);
        ui->dspbWidth->setValue(100);
        ui->dspbWidth->blockSignals(false);

        ui->dspbHeight->blockSignals(true);
        ui->dspbHeight->setValue(100);
        ui->dspbHeight->blockSignals(false);

        ui->chbEmbed->setChecked(FOriginalImageData.size()<=Options::node(OPV_XHTML_EMBEDSIZE).value().toInt());
        size.scale(ui->lblImage->size(), Qt::KeepAspectRatio);

        if (reader.canRead())
        {
			disableCommon(false);
			updateInfoLine(buffer.size(), FOriginalFormat, FSizeOld.width(), FSizeOld.height());
            ui->cmbType->blockSignals(true);
            if (!ui->cmbType->itemData(0).toByteArray().isNull())
                ui->cmbType->insertItem(0, tr("Do not change"));
            ui->cmbType->setCurrentIndex(0);
            ui->cmbType->blockSignals(false);

            if (reader.supportsAnimation())
            {
				QMovie *movie = new QMovie(&buffer, FOriginalFormat, this);
                if (movie->isValid())
                {
                    movie->setScaledSize(size);
                    ui->lblImage->setMovie(movie);
                    movie->start();
                }
                else
                {
                    ui->lblInfo->setText(tr("Error: cannot create movie!"));
                    disableCommon();
                    disableBOB();                    
                }                
            }
            else    // No animation supported - let's try without it!
            {
                reader.setScaledSize(size);
                ui->lblImage->setPixmap(QPixmap::fromImage(reader.read()));
            }

            FUrlCurrent=AUrl;
            if (!isRemote())
                calculateUrl(FOriginalImageData);
            disableBOB(isRemote());
            enableInsert();
        }
        else
        {
            ui->lblInfo->setText(tr("Error: cannot read image!"));
            disableCommon();
            disableBOB();
        }
    }
}

void InsertImage::calculateUrl(const QByteArray &AImageData)
{
    FUrlCurrent.setPath(FBitsOfBinary->contentIdentifier(AImageData));
    FUrlCurrent.setScheme("cid");
	ui->ledUrl->setText(FUrlCurrent.toString());
}

void InsertImage::updateInfoLine(qint64 AImageSize, const QByteArray &AImageFormat, int AWidth, int AHeight)
{
	ui->lblInfo->setText(tr("%1: %2x%3 px; %n byte(s)", "FORMAT: WIDTHxHEIGHT px; SIZE byte(s)", AImageSize)
						.arg(QString(AImageFormat).toUpper())
						.arg(AWidth)
						.arg(AHeight));
}

void InsertImage::recalculateImageData()
{
	QByteArray format;
    QSize size = ui->chbPhysResize->isChecked()?QSize(ui->spbWidth->value(), ui->spbHeight->value()):FSizeOld;
    if (size == FSizeOld && (ui->cmbType->currentIndex()==0 || FWriterFormats.at(ui->cmbType->currentIndex()-1)==FOriginalFormat))
        FImageData = FOriginalImageData;
    else
    {
		FImageData.clear();
		QVariant data = ui->cmbType->itemData(ui->cmbType->currentIndex());
		format = data.isNull()?FOriginalFormat:data.toByteArray();
		LOG_INFO(QString("Image format: %1").arg(format.data()));
		QBuffer buffer(&FImageData);
        QImage image = QImage::fromData(FOriginalImageData);
        if (!image.isNull())
		{
			if (image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).save(&buffer, format.data()))
				LOG_INFO("Image saved successfuly");
			else
				LOG_ERROR("Image save failed!");
		}
		else
			LOG_ERROR("Image read failed!");
    }
    if (!FImageData.isEmpty())
	{
        calculateUrl(FImageData);
		updateInfoLine(FImageData.size(), format, size.width(), size.height());
	}
}

void InsertImage::enableInsert()
{
	ui->pbInsert->setEnabled(FUrlCurrent.isValid() && QUrl::fromUserInput(ui->ledUrl->text())==FUrlCurrent);
}

void InsertImage::updateSpinboxPixels(QSpinBox *ASpinBox)
{
	ASpinBox->setSuffix(tr("pixels", "", ASpinBox->value()).prepend(" "));
}

void InsertImage::onLoadFinished()
{
    ui->pbLoad->setEnabled(true);
    ui->pbBrowse->setEnabled(true);

    QNetworkReply *reply=qobject_cast<QNetworkReply *>(sender());
    if (reply->error()==QNetworkReply::NoError)
    {
        FOriginalImageData = reply->readAll();
        readImageData(reply->url());
        ui->pbInsert->setEnabled(true);
        ui->ledAlt->setFocus();
        FImageData = FOriginalImageData;
    }
    else
    {
        ui->lblInfo->setText(tr("Error: %1").append(reply->errorString()));
        disableCommon();
    }
    reply->close();
    reply->deleteLater();
}
