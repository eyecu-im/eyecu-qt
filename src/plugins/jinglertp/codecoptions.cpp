#include "jinglertp.h"
#include <definitions/optionvalues.h>
#include <utils/iconstorage.h>

CodecOptions::CodecOptions(QWidget *parent):
	QWidget(parent),ui(new Ui::CodecOptions)
{
    ui->setupUi(this);

	connect(ui->lwAvailable->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(modified()));
	connect(ui->lwUsed->model(), SIGNAL(rowsInserted(QModelIndex,int,int)), SIGNAL(modified()));

	connect(ui->lwAvailable->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(modified()));
	connect(ui->lwUsed->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)), SIGNAL(modified()));

	ui->pbUsedUp->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowUp));
	ui->pbUsedDown->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowDown));
	ui->pbUse->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowRight));
	ui->pbUnuse->setIcon(QApplication::style()->standardIcon(QStyle::SP_ArrowLeft));

	// Build list of supported payload types
	for (QAVOutputFormat::Iterator it; it; ++it)
		if ((*it).name()=="rtp")
		{
			FRtp = *it;
			break;
		}

    reset();
}

CodecOptions::~CodecOptions()
{
    delete ui;
}

void CodecOptions::onAvailableCodecCurrentRowChanged(int ARow)
{
	ui->pbUse->setEnabled(ARow>-1);
}

void CodecOptions::onUsedCodecCurrentRowChanged(int ARow)
{
	if (ARow>-1)
	{
		ui->pbUsedUp->setEnabled(ARow>0);
		ui->pbUsedDown->setEnabled(ARow < ui->lwUsed->count()-1);
		ui->pbUnuse->setEnabled(true);
	}
	else
	{
		ui->pbUnuse->setDisabled(true);
		ui->pbUsedUp->setDisabled(true);
		ui->pbUsedDown->setDisabled(true);
	}
}

void CodecOptions::onUsedCodecPriorityChange()
{
	int row = ui->lwUsed->currentRow();
	QListWidgetItem *item = ui->lwUsed->takeItem(row);
	if (sender()==ui->pbUsedDown)
		row++;
	else
		row--;
	ui->lwUsed->insertItem(row, item);
	ui->lwUsed->setCurrentRow(row);
}

void CodecOptions::onCodecUse()
{
	QListWidgetItem *item = ui->lwAvailable->takeItem(ui->lwAvailable->currentRow());
	ui->lwUsed->addItem(item);
	ui->lwUsed->setCurrentItem(item);
}

void CodecOptions::onCodecUnuse()
{
	QListWidgetItem *item = ui->lwUsed->takeItem(ui->lwUsed->currentRow());
	ui->lwAvailable->addItem(item);
	ui->lwAvailable->sortItems();
	ui->lwAvailable->setCurrentItem(item);
}

void CodecOptions::apply()
{
	QList<int> usedCodecIds;

	for (int i=0; i<ui->lwUsed->count(); ++i)
		usedCodecIds.append(ui->lwUsed->item(i)->data(Qt::UserRole).toInt());

	Options::node(OPV_JINGLE_RTP_CODECS_USED).setValue(JingleRtp::stringFromInts(usedCodecIds));

    emit childApply();
}

void CodecOptions::reset()
{
	if (FRtp)
	{
		ui->lwAvailable->clear();
		ui->lwUsed->clear();
		const QStringList codecNames = QPayloadType::names(true);
		QList<int> usedCodecIds = JingleRtp::intsFromString(Options::node(OPV_JINGLE_RTP_CODECS_USED).value().toString());
		QSet<int> codecIds;

		for (QStringList::ConstIterator it = codecNames.constBegin(); it != codecNames.constEnd(); ++it)
		{
			int id = QPayloadType::idByName(*it);
			if (!codecIds.contains(id) && !usedCodecIds.contains(id))
			{
				codecIds.insert(id);
				if (FRtp.queryCodec(id))
				{
					QAVCodec decoder(QAVCodec::findDecoder(id));
					QAVCodec encoder(QAVCodec::findEncoder(id));
					if (decoder && encoder && decoder.type()==QAVCodec::MT_Audio)	// Video is not available yet
					{
						QListWidgetItem *item = new QListWidgetItem(decoder.longName());
						item->setData(Qt::UserRole, id);
						ui->lwAvailable->addItem(item);
					}
				}
			}
		}
		ui->lwAvailable->sortItems();

		for (QList<int>::ConstIterator it = usedCodecIds.constBegin(); it!=usedCodecIds.constEnd(); ++it)
			if (FRtp.queryCodec(*it))
			{
				QAVCodec decoder(QAVCodec::findDecoder(*it));
				QAVCodec encoder(QAVCodec::findEncoder(*it));
				if (decoder && encoder && decoder.type()==QAVCodec::MT_Audio)	// Video is not available yet
				{
					QListWidgetItem *item = new QListWidgetItem(decoder.longName());
					item->setData(Qt::UserRole, *it);
					ui->lwUsed->addItem(item);
				}
			}
	}

	if (ui->lwAvailable->currentRow()<0)
		onAvailableCodecCurrentRowChanged(-1);
	else
		ui->lwAvailable->setCurrentRow(-1);
	if (ui->lwUsed->currentRow()<0)
		onUsedCodecCurrentRowChanged(-1);
	else
		ui->lwUsed->setCurrentRow(-1);	

    emit childReset();
}

void CodecOptions::changeEvent(QEvent *e)
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
