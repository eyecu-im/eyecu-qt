#include <QMovie>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/notificationdataroles.h>
#include <definitions/messagedataroles.h>
#include <definitions/optionvalues.h>

#include <utils/iconstorage.h>
#include <utils/options.h>

#include "attentiondialog.h"

AttentionDialog::AttentionDialog(int ANotifyId, const INotification &ANotification, INotifications *ANotifications, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AttentionDialog),
    notifyId(ANotifyId),
    FNotifications(ANotifications)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
	Qt::WindowFlags flags = Qt::Dialog|Qt::CustomizeWindowHint|Qt::WindowTitleHint|Qt::WindowStaysOnTopHint;
#ifndef Q_WS_PM
	flags|=Qt::WindowSystemMenuHint;
#endif
#ifdef Q_WS_X11
	flags|=Qt::X11BypassWindowManagerHint;
#endif
	setWindowFlags(flags);
    setWindowTitle(ANotification.data.value(NDR_POPUP_TITLE).toString());
    IconStorage *iconStorage=IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
	setWindowIcon(iconStorage->getIcon(MNI_ATTENTION));

	QMovie *movie = new QMovie(iconStorage->fileFullName(MNI_BELL), QByteArray(), this);   // Set "this" as parent, to delete it automatically when not needed
    ui->lblBell1->setMovie(movie);
    ui->lblBell2->setMovie(movie);
    ui->lblBell3->setMovie(movie);
    ui->lblBell4->setMovie(movie);
    movie->start();

	movie = new QMovie(iconStorage->fileFullName(MNI_EXCLAMATION), QByteArray(), this);   // Set "this" as parent, to delete it automatically when not needed
	ui->lblExclamation1->setMovie(movie);
	ui->lblExclamation2->setMovie(movie);
	movie->start();

	QString avatarFileName=ANotification.data.value(NDR_ATTENTION_DIALOG_AVATAR_FILE_NAME).toString();
	if (!avatarFileName.isEmpty())   // Has avatar data
    {
		QMovie *movie=new QMovie(avatarFileName, QByteArray(), this); // Set "this" as parent, to delete it automatically when not needed
        if (movie->isValid())
        {
			int extent = Options::node(OPV_AVATARS_LARGESIZE).value().toInt();
			ui->lblAvatar->setFixedSize(extent, extent);
			QSize size = QImageReader(avatarFileName).size();
			size.scale(ui->lblAvatar->maximumSize(),Qt::KeepAspectRatio);
			movie->setScaledSize(size);
			ui->lblAvatar->setMovie(movie);
			movie->start();
        }
		else
			ui->lblAvatar->hide();
    }

    QString text=ANotification.data.value(NDR_POPUP_HTML).toString();
    if (!text.isEmpty())
        ui->lblText->setText(text);
    connect(this->ui->pushButton,SIGNAL(clicked()),SLOT(onOkClick()));
}

AttentionDialog::~AttentionDialog()
{
    delete ui;
}

void AttentionDialog::onOkClick()
{
    FNotifications->activateNotification(notifyId);
}
