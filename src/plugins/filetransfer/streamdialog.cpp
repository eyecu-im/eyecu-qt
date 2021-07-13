#include "streamdialog.h"

#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QDesktopServices>
// *** <<< eyeCU <<< ***
#include <QCheckBox>
// *** >>> eyeCU >>> ***
#include <definitions/optionvalues.h>
#include <definitions/internalerrors.h>
#include <utils/iconstorage.h>
#include <utils/options.h>
#include <utils/logger.h>
#include <utils/jid.h>
#include <utils/qt4qt5compat.h>

StreamDialog::StreamDialog(IDataStreamsManager *ADataManager, IFileStreamsManager *AFileManager, IFileTransfer *AFileTransfer, IFileStream *AFileStream, QWidget *AParent) : QDialog(AParent)
{
	REPORT_VIEW;
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose,true);

	FFileStream = AFileStream;
	FFileTransfer = AFileTransfer;
	FFileManager = AFileManager;
	FDataManager = ADataManager;
// *** <<< eyeCU <<< ***
	FStreamSelection			= Options::node(OPV_COMMON_ADVANCED).value().toBool() || FFileStream->streamKind()==IFileStream::ReceiveFile;
	FMultipleStreamSelection	= Options::node(OPV_COMMON_ADVANCED).value().toBool() && FFileStream->streamKind()==IFileStream::SendFile;
// *** >>> eyeCU >>> ***
	ui.pgbPrgress->setMinimum(0);
	ui.pgbPrgress->setMaximum(100);

	if (FFileStream->streamKind() == IFileStream::SendFile)
	{
		setWindowTitle(tr("Send File - %1").arg(FFileStream->streamJid().uFull()));
		ui.lblContactLabel->setText(tr("To:"));
	}
	else
	{
		setWindowTitle(tr("Receive File - %1").arg(FFileStream->streamJid().uFull()));
		ui.lblContactLabel->setText(tr("From:"));
	}
// *** <<< eyeCU <<< ***
	if (FMultipleStreamSelection)
	{
		QVBoxLayout			*layout = new QVBoxLayout();
		layout->setSpacing(0);
		ui.formLayout->setLayout(5, QFormLayout::FieldRole, layout);
		ui.lblMethod->setText(tr("Methods:"));
	}
	else if (FStreamSelection)
	{
		FMethodSelection = new QComboBox(this);
		ui.formLayout->setWidget(5, QFormLayout::FieldRole, FMethodSelection);
		if (!AFileStream->methodNS().isEmpty())
		{
			IDataStreamMethod *method = FDataManager->method(AFileStream->methodNS());
			if (method)
			{
				FMethodSelection->addItem(method->methodName(),method->methodNS());
				FMethodSelection->setCurrentIndex(0);
			}
		}
	}
	else
		ui.lblMethod->hide();
// *** >>> eyeCU >>> ***
	ui.lblContact->setText(HTML_ESCAPE(FFileStream->contactJid().uFull()));

	connect(FFileStream->instance(),SIGNAL(stateChanged()),SLOT(onStreamStateChanged()));
	connect(FFileStream->instance(),SIGNAL(speedChanged()),SLOT(onStreamSpeedChanged()));
	connect(FFileStream->instance(),SIGNAL(propertiesChanged()),SLOT(onStreamPropertiesChanged()));
	connect(FFileStream->instance(),SIGNAL(streamDestroyed()),SLOT(onStreamDestroyed()));

	connect(ui.tlbFile,SIGNAL(clicked(bool)),SLOT(onFileButtonClicked(bool)));
	connect(ui.bbxButtons,SIGNAL(clicked(QAbstractButton *)),SLOT(onDialogButtonClicked(QAbstractButton *)));

	onStreamPropertiesChanged();
	onStreamStateChanged();
	onStreamSpeedChanged();
}

StreamDialog::~StreamDialog()
{
	if (FFileStream)
	{
		if (FFileStream->streamState()==IFileStream::Finished || FFileStream->streamState()==IFileStream::Aborted)
			FFileStream->instance()->deleteLater();
		else if (FFileStream->streamKind()==IFileStream::SendFile && FFileStream->streamState()==IFileStream::Creating)
			FFileStream->instance()->deleteLater();
	}
	emit dialogDestroyed();
}

IFileStream *StreamDialog::stream() const
{
	return FFileStream;
}

void StreamDialog::setContactName(const QString &AName)
{
	ui.lblContact->setText(AName);
}

QList<QString> StreamDialog::selectedMethods() const
{
	QList<QString> methods;
// *** <<< eyeCU <<< ***
	if (FMultipleStreamSelection)
	{
		for (QHash<QCheckBox*, QString>::ConstIterator it=FMethods.constBegin(); it!=FMethods.constEnd(); ++it)
			if (it.key()->isChecked())
				methods.append(*it);
	}
	else if (FStreamSelection)
	{
		QComboBox *cmbMethod = qobject_cast<QComboBox *>(ui.formLayout->itemAt(5, QFormLayout::FieldRole)->widget());
		if (cmbMethod->currentIndex() >= 0)
			methods.append(cmbMethod->itemData(cmbMethod->currentIndex()).toString());
	}
	else
		methods = FAvailableMethods;
// *** >>> eyeCU >>> ***
	return methods;
}

void StreamDialog::setSelectableMethods(const QList<QString> &AMethods)
{
// *** <<< eyeCU <<< ***
	if (FMultipleStreamSelection)
	{
		QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(ui.formLayout->itemAt(5, QFormLayout::FieldRole)->layout());
		foreach(const QString &methodNS, AMethods)
		{
			IDataStreamMethod *method = FDataManager->method(methodNS);
			if (method)
			{
				QCheckBox *checkBox = new QCheckBox(method->methodName());
				checkBox->setChecked(true);
				layout->addWidget(checkBox);
				FMethods.insert(checkBox, methodNS);
			}
		}
	}
	else if (FStreamSelection)
	{
		QComboBox *cmbMethod = qobject_cast<QComboBox *>(ui.formLayout->itemAt(5, QFormLayout::FieldRole)->widget());
		cmbMethod->clear();
		foreach(const QString &methodNS, AMethods)
		{
			IDataStreamMethod *method = FDataManager->method(methodNS);
			if (method)
				cmbMethod->addItem(method->methodName(),method->methodNS());
		}
		if (cmbMethod->count())
		{
			int index = cmbMethod->findData(Options::node(OPV_FILESTREAMS_DEFAULTMETHOD).value().toString());
			cmbMethod->setCurrentIndex(index==-1?0:index);
		}
	}
	else
		FAvailableMethods = AMethods;
// *** >>> eyeCU >>> ***
}

qint64 StreamDialog::minPosition() const
{
	return FFileStream->rangeOffset();
}

qint64 StreamDialog::maxPosition() const
{
	return FFileStream->rangeLength()>0 ? FFileStream->rangeOffset()+FFileStream->rangeLength() : FFileStream->fileSize();
}

qint64 StreamDialog::curPosition() const
{
	return minPosition() + FFileStream->progress();
}

int StreamDialog::curPercentPosition() const
{
	qint64 maxPos = maxPosition();
	return maxPos>0 ? curPosition()*100/maxPos : 0;
}

bool StreamDialog::acceptFileName(const QString &AFile)
{
	QFileInfo fileInfo(AFile);
	if (fileInfo.exists() && FFileStream->streamKind()==IFileStream::ReceiveFile)
	{
		if (FFileStream->isRangeSupported() && fileInfo.size()<FFileStream->fileSize())
		{
			QMessageBox::StandardButton button = QMessageBox::question(this,tr("Continue file transfer"),
				tr("A file with this name, but a smaller size already exists.")+"<br>"+
				tr("If you want to download the rest of file press 'Yes'")+"<br>"+
				tr("If you want to start download from the beginning press 'Retry'")+"<br>"+
				tr("If you want to change file name press 'Cancel'"),
				QMessageBox::Yes|QMessageBox::Retry|QMessageBox::Cancel);
			if (button == QMessageBox::Yes)
			{
				FFileStream->setRangeOffset(fileInfo.size());
			}
			else if (button == QMessageBox::Retry)
			{
				if (!QFile::remove(fileInfo.absoluteFilePath()))
				{
					QMessageBox::warning(this,tr("Warning"),tr("Can not delete existing file"));
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			QMessageBox::StandardButton button = QMessageBox::question(this,tr("Remove file"),
				tr("A file with this name already exists. Do you want to remove existing file?"),
				QMessageBox::Yes|QMessageBox::Cancel);
			if (button == QMessageBox::Yes)
			{
				if (!QFile::remove(AFile))
				{
					QMessageBox::warning(this,tr("Warning"),tr("Can not delete existing file"));
					return false;
				}
			}
			else
			{
				return false;
			}
		}
	}
	else if (!fileInfo.exists() && FFileStream->streamKind()==IFileStream::SendFile)
	{
		QMessageBox::warning(this,tr("Warning"),tr("Selected file does not exists"));
		return false;
	}
	return !AFile.isEmpty();
}

QString StreamDialog::sizeName(qint64 ABytes) const
{
	static int md[] = {1, 10, 100};
	QString units = tr("B","Byte");
	qreal value = ABytes;

	if (value > 1024)
	{
		value = value / 1024;
		units = tr("KB","Kilobyte");
	}
	if (value > 1024)
	{
		value = value / 1024;
		units = tr("MB","Megabyte");
	}
	if (value > 1024)
	{
		value = value / 1024;
		units = tr("GB","Gigabyte");
	}

	int prec = 0;
	if (value < 10)
		prec = 2;
	else if (value < 100)
		prec = 1;

	while (prec>0 && (qreal)qRound64(value*md[prec-1])/md[prec-1]==(qreal)qRound64(value*md[prec])/md[prec])
		prec--;

	value = (qreal)qRound64(value*md[prec])/md[prec];

	return QString::number(value,'f',prec)+units;
}
// *** <<< eyeCU <<< ***
void StreamDialog::enableMethodSelection(bool AEnable)
{
	if (FMultipleStreamSelection)
		for (QHash<QCheckBox*, QString>::ConstIterator it=FMethods.constBegin(); it!=FMethods.constEnd(); ++it)
			it.key()->setEnabled(AEnable);
	else if (FStreamSelection)
		qobject_cast<QComboBox *>(ui.formLayout->itemAt(5, QFormLayout::FieldRole)->widget())->setEnabled(AEnable);
}
// *** >>> eyeCU >>> ***
void StreamDialog::onStreamStateChanged()
{
	switch (FFileStream->streamState())
	{
	case IFileStream::Creating:
		ui.tlbFile->setEnabled(true);
		enableMethodSelection(true); // *** <<< eyeCU >>> ***
		ui.lneFile->setReadOnly(FFileStream->streamKind()==IFileStream::SendFile);
		ui.pteDescription->setReadOnly(FFileStream->streamKind()!=IFileStream::SendFile);
		if (FFileStream->streamKind()==IFileStream::SendFile)
			ui.bbxButtons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
		else
			ui.bbxButtons->setStandardButtons(QDialogButtonBox::Ok|QDialogButtonBox::Abort);
		break;
	case IFileStream::Negotiating:
	case IFileStream::Connecting:
	case IFileStream::Transfering:
		ui.tlbFile->setEnabled(false);
		ui.lneFile->setReadOnly(true);
		ui.pteDescription->setReadOnly(true);
		enableMethodSelection(false); // *** <<< eyeCU >>> ***
		ui.bbxButtons->setStandardButtons(QDialogButtonBox::Abort|QDialogButtonBox::Close);
		break;
	case IFileStream::Disconnecting:
	case IFileStream::Finished:
	case IFileStream::Aborted:
		ui.tlbFile->setEnabled(false);
		ui.lneFile->setReadOnly(true);
		ui.pteDescription->setReadOnly(true);
		enableMethodSelection(false); // *** <<< eyeCU >>> ***
		if (FFileStream->streamKind()==IFileStream::SendFile && FFileStream->streamState()==IFileStream::Aborted)
			ui.bbxButtons->setStandardButtons(QDialogButtonBox::Retry|QDialogButtonBox::Close);
		else if (FFileStream->streamKind()==IFileStream::ReceiveFile && FFileStream->streamState()==IFileStream::Finished)
			ui.bbxButtons->setStandardButtons(QDialogButtonBox::Open|QDialogButtonBox::Close);
		else
			ui.bbxButtons->setStandardButtons(QDialogButtonBox::Close);
		break;
	}
	ui.lblStatus->setText(FFileStream->stateString());
	resize(width(),minimumSizeHint().height());
}

void StreamDialog::onStreamSpeedChanged()
{
	if (FFileStream->streamState() == IFileStream::Transfering)
	{
		ui.pgbPrgress->setValue(curPercentPosition());
		ui.lblProgress->setText(tr("Transferred %1 of %2.").arg(sizeName(curPosition())).arg(sizeName(maxPosition())) + " " + tr("Speed %1.").arg(sizeName(FFileStream->speed())+tr("/sec")));
	}
	else if (FFileStream->fileSize() > 0)
	{
		ui.pgbPrgress->setValue(curPercentPosition());
		ui.lblProgress->setText(tr("Transferred %1 of %2.").arg(sizeName(curPosition())).arg(sizeName(maxPosition())));
	}
	else
	{
		ui.pgbPrgress->setValue(0);
		ui.lblProgress->setText(QString());
	}
}

void StreamDialog::onStreamPropertiesChanged()
{
	ui.lneFile->setText(FFileStream->fileName());
	ui.pteDescription->setPlainText(FFileStream->fileDescription());
	onStreamSpeedChanged();
}

void StreamDialog::onStreamDestroyed()
{
	FFileStream = NULL;
	close();
}

void StreamDialog::onFileButtonClicked(bool)
{
	if (FFileStream->streamState() == IFileStream::Creating)
	{
		static QString lastSelectedPath = QDir::homePath();
		QString file = QDir(lastSelectedPath).absoluteFilePath(FFileStream->fileName());

		if (FFileStream->streamKind() == IFileStream::ReceiveFile)
			file = QFileDialog::getSaveFileName(this,tr("Select file for receive"),file,QString(),NULL,QFileDialog::DontConfirmOverwrite);
		else
			file = QFileDialog::getOpenFileName(this,tr("Select file to send"),file);

		if (!file.isEmpty())
		{
			lastSelectedPath = QFileInfo(file).absolutePath();
			FFileStream->setFileName(file);
		}
	}
}

void StreamDialog::onDialogButtonClicked(QAbstractButton *AButton)
{
	if (ui.bbxButtons->standardButton(AButton) ==  QDialogButtonBox::Ok)
	{
		QList<QString> methods = selectedMethods();
		if (!methods.isEmpty())
		{
			if (acceptFileName(ui.lneFile->text()))
			{
				if (FFileStream->streamKind() == IFileStream::SendFile)
				{
					FFileStream->setFileName(ui.lneFile->text());
					FFileStream->setFileDescription(ui.pteDescription->toPlainText());
					if (!FFileStream->initStream(methods))
						QMessageBox::warning(this,tr("Warning"),tr("Unable to send request for file transfer, check settings and try again!"));
				}
				else
				{
					IDataStreamMethod *streamMethod = FDataManager->method(methods.first());
					if (streamMethod)
					{
						FFileStream->setFileName(ui.lneFile->text());
						FFileStream->setFileDescription(ui.pteDescription->toPlainText());
						if (!FFileStream->startStream(methods.first()))
							QMessageBox::warning(this,tr("Warning"),tr("Unable to start the file transfer, check settings and try again!"));
					}
					else
					{
						QMessageBox::warning(this,tr("Warning"),tr("Selected data stream is not available"));
					}
				}
			}
		}
		else
		{
			QMessageBox::warning(this,tr("Warning"),tr("Please select at least one data stream"));
		}
	}
	else if (ui.bbxButtons->standardButton(AButton) == QDialogButtonBox::Abort)
	{
		if (QMessageBox::question(this,tr("Cancel file transfer"),tr("Are you sure you want to cancel a file transfer?"),QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
		{
			FFileStream->abortStream(XmppError(IERR_FILETRANSFER_TRANSFER_TERMINATED));
		}
	}
	else if (ui.bbxButtons->standardButton(AButton) == QDialogButtonBox::Retry)
	{
		FFileTransfer->sendFile(FFileStream->streamJid(), FFileStream->contactJid(), FFileStream->fileName(), FFileStream->fileDescription());
		close();
	}
	else if (ui.bbxButtons->standardButton(AButton) == QDialogButtonBox::Open)
	{
		QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(FFileStream->fileName()).absolutePath()));
	}
	else if (ui.bbxButtons->standardButton(AButton) == QDialogButtonBox::Close)
	{
		close();
	}
	else if (ui.bbxButtons->standardButton(AButton) == QDialogButtonBox::Cancel)
	{
		close();
	}
}
