#include <QtGui>
#include <QDir>
#include <QDomDocument>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLine>
#include <QMessageBox>
#include <QLineEdit>

#include "wizardpages.h"
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>
#include <definitions/serviceicons.h>
#include <definitions/resources.h>
#include <utils/options.h>
#include <utils/qt4qt5compat.h>

#define GATEWAY_DEF  "gateway.def.xml"
#define NETWORKS_DEF "networks"

#define FIELD_TYPE_HIDDEN       "hidden"
#define FIELD_TYPE_FIXED		"fixed"
#define FIELD_TYPE_BOOLEAN		"boolean"
#define FIELD_TYPE_TEXTSINGLE	"text-single"
#define FIELD_TYPE_TEXTPRIVATE	"text-private"
#define FIELD_TYPE_LISTSINGLE	"list-single"
#define FIELD_TYPE_JIDSINGLE	"jid-single"
#define FIELD_TYPE_TEXTMULTI	"text-multi"
#define FIELD_TYPE_JIDMULTI     "jid-multi"
#define FIELD_TYPE_LISTMULTI	"list-multi"

#define VALIDATE_TYPE_DATETIME  "xs:dateTime"
#define VALIDATE_TYPE_DATE      "xs:date"
#define VALIDATE_TYPE_TIME      "xs:time"



TransportWizard::TransportWizard(const Jid &AStreamJid,
						 IRegistration *ARegistration, IServiceDiscovery *AServiceDiscovery,
						 QWidget *parent) :
    QWizard(parent),
    FStreamJid(AStreamJid),
    FRegistration(ARegistration)
{
    setPage(Page_Intro, new IntroPage);
	NetworksPage *networkPage = new NetworksPage();
	setPage(Page_Networks, networkPage);
	GatewayPage *Gateway =new GatewayPage(AServiceDiscovery, FStreamJid);
	setPage(Page_Gateway, Gateway);
    ProcessPage *Process = new ProcessPage(FStreamJid,FRegistration,Gateway);
    setPage(Page_Process,Process);
    connect(this,SIGNAL(getGateway()),Process,SLOT(createGateway()));
    ResultPage *Result=new ResultPage(FStreamJid,FRegistration,Process);
    setPage(Page_Result,Result);
    connect(this,SIGNAL(getRegister()),Result,SLOT(onGetRegister()));

	setPage(Page_Conclusion, new ConclusionPage(networkPage));
    setStartId(Page_Intro);

    setOptions(NoBackButtonOnLastPage|NoBackButtonOnStartPage|NoCancelButton);
    connect(this,SIGNAL(currentIdChanged(int)),this,SLOT(onCurIdChange(int)));

#ifndef Q_WS_MAC
    setWizardStyle(ModernStyle);
#endif

    QString fileName = IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->fileFullName(MNI_WIZARD);
    setPixmap(QWizard::LogoPixmap, QPixmap(fileName));//LogoPixmap WatermarkPixmap

    fileName = IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->fileFullName(MNI_WIZARD_ACC_BAN);
    setPixmap(QWizard::BannerPixmap, QPixmap(fileName));

	setWindowTitle(tr("Legacy network connection Wizard"));
    setWindowIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAINWINDOW_LOGO16));
}

void TransportWizard::onCurIdChange(int id)
{
    if(id==Page_Process)
        emit getGateway();
    if(id==Page_Result)
        emit getRegister();
}

void TransportWizard::accept()
{
    QDialog::accept();   //QDialog::done(1);
}

//!------------------------------
IntroPage::IntroPage(QWidget *parent): QWizardPage(parent)
{
    QString style="style='color:blue;'";
	setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Legacy network connection")));
	setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("This Wizard will help you to connect to a legacy network via transport (gateway)")));

    QString fileName = IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->fileFullName(MNI_WIZARD_GT_BEG);
    setPixmap(QWizard::WatermarkPixmap, QPixmap(fileName));

	QVBoxLayout *layout = new QVBoxLayout;
	QLabel *lblText = new QLabel(tr("Before running this Wizard, make sure you have an account at legacy network.\n"
									"If don't, please register at legacy network on its web site or using native client."));
	lblText->setWordWrap(true);
	layout->addWidget(lblText);
	setLayout(layout);
}

int IntroPage::nextId() const
{
	return TransportWizard::Page_Networks;
}

//!------------------------------
NetworksPage::NetworksPage(QWidget *parent): QWizardPage(parent)
{
    QString style="style='color:blue;'";
	setTitle(QString("<span %2>%1</span>").arg(tr("Network selection")).arg(style));
	setSubTitle(QString("<span %2>%1</span>").arg(tr("Choose a legacy network you want to connect")).arg(style));

    QLabel *lblNetworksList = new QLabel(QString("<b>%1:</b>").arg(tr("Please select a network from the list")));
	FNetworksList = new SelectableTreeWidget();
    lblNetworksList->setBuddy(lblNetworksList);
	registerField("network*", FNetworksList, "value", SIGNAL(valueChanged(QString)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(lblNetworksList);
    layout->addWidget(FNetworksList);
    setLayout(layout);

    loadNetworksList();	
}

void NetworksPage::loadNetworksList()
{
    FIconStorage = IconStorage::staticStorage(RSR_STORAGE_SERVICEICONS);
    QTreeWidgetItem* pItem;
    FNetworksList->sortItems(0, Qt::AscendingOrder);

    QStringList headers;
    headers << tr("Netwok") << tr("Comment");
    FNetworksList->setHeaderLabels(headers);

	FNetworkHash.insert("aim", tr("AIM"));
//	FNetworkHash.insert("facebook", tr("Facebook"));
	FNetworkHash.insert("gadu-gadu", tr("Gadu-Gadu"));
	FNetworkHash.insert("icq", tr("ICQ"));
	FNetworkHash.insert("irc", tr("IRC"));
//	FNetworkHash.insert("msn", tr("Windows Live Messenger"));
	FNetworkHash.insert("mrim", tr("Mail.Ru Agent"));
//	FNetworkHash.insert("myspaceim", tr("MySpace"));
	FNetworkHash.insert("qq", tr("QQ"));
	FNetworkHash.insert("renren", tr("Renren"));
//	FNetworkHash.insert("sip", tr("SIP"));
	FNetworkHash.insert("skype", tr("Skype"));
	FNetworkHash.insert("sametime", tr("IBM Sametime"));
	FNetworkHash.insert("sms", tr("SMS"));
	FNetworkHash.insert("twitter", tr("Twitter"));
	FNetworkHash.insert("x-tlen", tr("Tlen.pl"));
	FNetworkHash.insert("vk", tr("vKontakte"));
//	FNetworkHash.insert("xfire", tr("Xfire"));
	FNetworkHash.insert("yahoo", tr("Yahoo!"));

	QHash<QString,QString> networkDesc;
	networkDesc.insert("aim", tr("AOL Instant Messenger"));
//	networkDesc.insert("facebook", tr("World's most popular social network"));
	networkDesc.insert("gadu-gadu", tr("Gadu-Gadu - Polish instant messenger"));
	networkDesc.insert("icq", tr("\"I seek You\" instant messenger, popular in exUSSR and Germany"));
	networkDesc.insert("irc", tr("Internet Relay Chat"));
//	networkDesc.insert("msn", tr("Instant messenger from Microsoft Network (MSN)"));
	networkDesc.insert("mrim", tr("Instant messenger from Mail.ru portal"));
//	networkDesc.insert("myspaceim", tr("International social network, Beverly Hills (California, USA)"));
	networkDesc.insert("qq", tr("Tencent QQ - Chinese instant messenger"));
	networkDesc.insert("renren", tr("Chinese social network with an interface similar to Facebook"));
//	networkDesc.insert("sip", tr("Session Initiation Protocol"));
	networkDesc.insert("skype", tr("IP-telephony software with voice, video and text communication"));
	networkDesc.insert("sametime", tr("Real-time communication services from IBM (formerly IBM Lotus Sametime)"));
	networkDesc.insert("sms", tr("Sending Short Messages (SMS) to mobile phones"));
	networkDesc.insert("twitter", tr("An online social networking service that enables users to send and read short 140-character messages called \"tweets\""));
	networkDesc.insert("x-tlen", tr("An adware licensed Polish instant messaging service, fully compatible with Gadu-Gadu"));
	networkDesc.insert("vk", tr("Russian social network with an interface similar to Facebook"));
//	networkDesc.insert("xfire", tr("A proprietary freeware instant messaging service for gamers"));
	networkDesc.insert("yahoo", tr("Instant messenger from Yahoo! portal"));

    FNetworksList->setColumnWidth(0,160);
    FNetworksList->setColumnWidth(1,440);
	for(QHash<QString,QString>::ConstIterator it=FNetworkHash.constBegin(); it!=FNetworkHash.constEnd(); ++it)
    {
        pItem = new QTreeWidgetItem(FNetworksList);
		QIcon icon=FIconStorage->getIcon(it.key());
		pItem->setText(0, *it);
		pItem->setData(0, SelectableTreeWidget::ValueRole, it.key());
        if(icon.isNull())
            icon=FIconStorage->getIcon("gateway");
        pItem->setIcon(0,icon);
		pItem->setText(1,networkDesc.value(it.key()));
	}
}

void NetworksPage::initializePage()
{
	connect(FNetworksList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), wizard(), SLOT(next()), Qt::UniqueConnection);
}

int NetworksPage::nextId() const
{
	return TransportWizard::Page_Gateway;
}

//!------------------------------
GatewayPage::GatewayPage(IServiceDiscovery *AServiceDiscovery, const Jid &AStreamJid, QWidget *parent):
    QWizardPage(parent), FServiceDiscovery(AServiceDiscovery), FStreamJid(AStreamJid) // , FDiscoItemsReceived(false)
{
    QString style="style='color:blue;'";
	setTitle(QString("<span %2>%1</span>").arg(tr("Gateway selection")).arg(style));
	setSubTitle(QString("<span %2>%1</span>").arg(tr("Choose a gateway you want to use")).arg(style));

    FlblGatewaysList = new QLabel(QString("<b>%1:</b>").arg(tr("Gateways &list")));
	FTransportList = new SelectableTreeWidget();
	FlblGatewaysList->setBuddy(FTransportList);

	registerField("gatewayselected*", FTransportList, "value", SIGNAL(valueChanged(QString)));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(FlblGatewaysList);
	layout->addWidget(FTransportList);
    setLayout(layout);

	FIconStorageWizards = IconStorage::staticStorage(RSR_STORAGE_WIZARDS);
	FIconStorageMenu = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);

	connect(FServiceDiscovery->instance(), SIGNAL(discoItemsReceived(IDiscoItems)), SLOT(onDiscoItemsReceived(IDiscoItems)));
	connect(FServiceDiscovery->instance(), SIGNAL(discoInfoReceived(IDiscoInfo)), SLOT(onDiscoInfoReceived(IDiscoInfo)));
}

void GatewayPage::initializePage()
{
    if (FNetwork!=field("network").toString())
    {
        FNetwork = field("network").toString();
		FDiscoItems = IDiscoItems();
        loadGatewayList();
    }	
    FlblGatewaysList->setText(QString("<span><b>%2 %3</b></span>").arg(QString(tr("List of Gateways for"))).arg(FNetwork));
	if (FServiceDiscovery && FDiscoItems.streamJid.isEmpty())
		FServiceDiscovery->requestDiscoItems(FStreamJid, FStreamJid.domain());
	connect(FTransportList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), wizard(), SLOT(next()), Qt::UniqueConnection);
}

bool GatewayPage::validatePage()
{
	switch (FTransportList->currentItem()->data(0, Qt::UserRole).toInt())
	{
		case Unavailable:
			if (QMessageBox::warning(wizard(), tr("Warning!"),
											   tr("The transport you selected is unavailable now. "
												  "An attempt to register at it will probably fail. "
												  "Press \"Ok\" to proceed anyway or \"Cancel\" to select another transport."), QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Cancel)
				return false;
			break;

		case Unknown:
			if (QMessageBox::warning(wizard(), tr("Warning!"),
											   tr("The transport you selected is untested yet. "
												  "An attempt to register at it will possibly fail. "
												  "Press \"Ok\" to proceed anyway or \"Cancel\" to select another transport."), QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Cancel)
				return false;
			break;

		case Available:;
	}
	return true;
}

//!--to make the file download session once-
void GatewayPage::loadGatewayList()
{
	FTransportList->clear();
	QDir dir(FIconStorageWizards->resourcesDirs().first());
	dir.cd(FIconStorageWizards->storage());
	dir.cd(FIconStorageWizards->subStorage());
    QFile file(dir.absoluteFilePath(GATEWAY_DEF));
    FExcepFields.clear();
    if(file.open(QFile::ReadOnly))
    {
        QDomDocument FDocGateway;
        FDocGateway.clear();
        if(FDocGateway.setContent(file.readAll(), true))
        {
			FIconStorageServices = IconStorage::staticStorage(RSR_STORAGE_SERVICEICONS);
			FTransportList->sortItems(0, Qt::AscendingOrder);
            QStringList headers;
            headers << tr("Gateway") << tr("Software");

			FTransportList->setHeaderLabels(headers);
			FTransportList->sortItems(0, Qt::AscendingOrder);
			FTransportList->setColumnWidth(0,180);
			FTransportList->setColumnWidth(1,180);
            for (QDomElement e = FDocGateway.documentElement().firstChildElement("gateway");
                 !e.isNull();
                 e = e.nextSiblingElement("gateway"))
            {
                if(e.attribute("name")== FNetwork)
					for(QDomElement s=e.firstChildElement("server"); !s.isNull(); s = s.nextSiblingElement("server"))
                    {
						QTreeWidgetItem *pItem = new QTreeWidgetItem(FTransportList);
						QString jid = s.firstChildElement("jid").text();
						pItem->setText(0, jid);
						pItem->setData(0, SelectableTreeWidget::ValueRole, jid);
						FPendingItemsListed.append(jid);
						if (FServiceDiscovery->hasDiscoInfo(FStreamJid, jid))
							setItemStatus(pItem, Available);
						else
						{
							FServiceDiscovery->requestDiscoInfo(FStreamJid, jid);
							setItemStatus(pItem, Unknown);
						}
						pItem->setText(1, s.firstChildElement("software").text());
                        //!---------------
                        if(!s.firstChildElement("exception").isNull())
							for(QDomElement f=s.firstChildElement("exception").firstChildElement("field"); !f.isNull(); f = f.nextSiblingElement("field"))
                            {
                                f.setAttribute("jid",s.firstChildElement("jid").text());
                                FExcepFields.append(f);
                            }
                    }
            }
        }
	}
}

void GatewayPage::appendLocalTransport(const IDiscoInfo &ADiscoInfo)
{
	int identity = FServiceDiscovery->findIdentity(ADiscoInfo.identity, "gateway", FNetwork);
	if (identity!=-1)
	{
		QTreeWidgetItem *pItem = new QTreeWidgetItem(FTransportList);
		pItem->setText(0, ADiscoInfo.contactJid.full());
		pItem->setText(1, ADiscoInfo.identity.at(identity).name);
		setItemStatus(pItem, Available);
	}
}

void GatewayPage::onDiscoInfoReceived(const IDiscoInfo &ADiscoInfo)
{
	if (FPendingItems.contains(ADiscoInfo.contactJid))
	{
		appendLocalTransport(ADiscoInfo);
		FPendingItems.removeOne(ADiscoInfo.contactJid);
	}
	else if (FPendingItemsListed.contains(ADiscoInfo.contactJid))
	{
		QList<QTreeWidgetItem *> found = FTransportList->findItems(ADiscoInfo.contactJid.full(), Qt::MatchExactly, 0);
		if (!found.isEmpty())
			setItemStatus(found.at(0), ADiscoInfo.error.isNull()?Available:Unavailable);
		FPendingItemsListed.removeOne(ADiscoInfo.contactJid);
	}
}

void GatewayPage::onDiscoItemsReceived(const IDiscoItems &ADiscoItems)
{
	if (ADiscoItems.streamJid == FStreamJid && ADiscoItems.contactJid == FStreamJid.domain())
	{
		FDiscoItems = ADiscoItems;
		for (QList<IDiscoItem>::ConstIterator it=FDiscoItems.items.constBegin(); it!=FDiscoItems.items.constEnd(); it++)
			if (FTransportList->findItems((*it).itemJid.full(), Qt::MatchExactly).isEmpty())
			{
				if (FServiceDiscovery->hasDiscoInfo(FStreamJid, (*it).itemJid))
					appendLocalTransport(FServiceDiscovery->discoInfo(FStreamJid, (*it).itemJid));
				else
				{
					FPendingItems.append((*it).itemJid);
					FServiceDiscovery->requestDiscoInfo(FStreamJid, (*it).itemJid, (*it).node);
				}
			}
	}
}

void GatewayPage::setItemStatus(QTreeWidgetItem *AItem, GatewayPage::TransportStatus AStatus)
{
	AItem->setData(0, Qt::UserRole, AStatus);
	switch (AStatus)
	{
		case Available:
			AItem->setIcon(0, FIconStorageMenu->getIcon(MNI_YES));
			break;
		case Unavailable:
			AItem->setIcon(0, FIconStorageMenu->getIcon(MNI_NO));
			break;
		case Unknown:
			AItem->setIcon(0, FIconStorageServices->getIcon(SRI_SERVICE_WAIT));
			break;
	}
}

//!------------------------------
ProcessPage::ProcessPage(Jid &AStreamJid, IRegistration *ARegistration, GatewayPage *AGatewayPage, QWidget *parent):
    QWizardPage(parent),
    FStreamJid(AStreamJid),
    FRegistration(ARegistration),
	FGatewayPage(AGatewayPage),
	FGridLayout(NULL),
	FFieldWidth(300)
{
    QString style="style='color:blue;'";
    setTitle(QString("<span %2>%1</span>").arg(tr("The registration page")).arg(style));
    setSubTitle(QString("<span %2>%1</span>").arg(tr("Fill in the fields")).arg(style));

	FScrollArea=new QScrollArea;
    //scrollArea->setBackgroundRole(QPalette::Dark);
	FScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	FScrollArea->setWidgetResizable(true);
	FScrollArea->resize(460,300);

    FInstrLabel=new QLabel;
    FInstrLabel->setWordWrap(true);
    FInstrLabel->setAlignment(Qt::AlignHCenter);

    FErrorLabel=new QLabel;
    FErrorLabel->setWordWrap(true);
    FErrorLabel->setAlignment(Qt::AlignHCenter);

	FAutoRegCheckBox= new QCheckBox(tr("Automatically accept subscription requests"));
	registerField("authorization", FAutoRegCheckBox);
	FAutoRegCheckBox->setChecked(false);
	connect(FAutoRegCheckBox,SIGNAL(clicked(bool)),SLOT(onClicked(bool)));

	QVBoxLayout *mainLayout = new QVBoxLayout;
	mainLayout->insertWidget(0, FInstrLabel);
	mainLayout->insertWidget(1, FScrollArea);
	mainLayout->insertWidget(2, FAutoRegCheckBox);
	mainLayout->insertWidget(3, FErrorLabel);
	setLayout(mainLayout);

    localTextLabel();

    //!------------
    if (FRegistration)
    {
        connect(FRegistration->instance(),SIGNAL(registerFields(const QString &, const IRegisterFields &)),
            SLOT(onRegisterFields(const QString &, const IRegisterFields &)));
        connect(FRegistration->instance(),SIGNAL(registerError(const QString &, const XmppError &)),
            SLOT(onRegisterError(const QString &, const XmppError &)));
	}
}

void ProcessPage::createGateway()
{
    //!-- doRegister --
    FServiceTo = Jid::fromUserInput(field("gatewayselected").toString().trimmed());
	FRequestId = FRegistration!=NULL ? FRegistration->sendRegisterRequest(FStreamJid, FServiceTo) : QString::null;
    QString style="style='color:brown;'";
    if (!FRequestId.isEmpty())
		FInstrLabel->setText(QString("<span %1>%2</span>").arg(style).arg(tr("Waiting for host response ...")));
	else
	{
        style="style='color:red;'";
		FInstrLabel->setText(QString("<span %1>%2</span>").arg(style).arg(tr("Error: Can't send request to host.")));
    }
}

void ProcessPage::onRegisterFields(const QString &AId, const IRegisterFields &AFields)
{
    if (FRequestId == AId)
    {
		FSubmit.serviceJid = FServiceTo;
        FSubmit.fieldMask = AFields.fieldMask;
        FSubmit.key = AFields.key;

		FTmpFields.clear();
        FUserName.clear();
        FPassword.clear();
        FEmail.clear();
        FUrl.clear();

        FExcepFields.clear();
        FExcepFields=FGatewayPage->getExcepFields();

		QString		instructions;
		QLabel		*lblUser	= new QLabel(tr("User Name"));
		QLabel		*lblPassword= new QLabel(tr("Password"));
		QLabel		*lblEmail	= new QLabel(tr("e-mail"));
		QLabel		*lblUrl		= new QLabel(tr("Web Link"));
		QLineEdit	*ledUser	= new QLineEdit;
		QLineEdit	*ledEmail	= new QLineEdit;
		QLineEdit	*ledUrl		= new QLineEdit;
		QLineEdit	*ledPassword= new QLineEdit;

		ledPassword->setEchoMode(QLineEdit::Password);

		int i=0;
        if(AFields.form.type.isEmpty())//if(AFields.registered)
        {
            FDirection=true;
            if (!AFields.instructions.isEmpty())
				instructions=AFields.instructions;

			if(AFields.fieldMask & IRegisterFields::Username)
			{
				FGridLayout->addWidget(lblUser, i, 0);//,Qt::AlignLeft
				FGridLayout->addWidget(ledUser, i++, 1);
                if(!AFields.username.isNull())
					ledUser->setText(AFields.username);
				connect(ledUser,SIGNAL(textChanged(QString)),SLOT(onUserEditChanged(QString)));
            }
			if(AFields.fieldMask & IRegisterFields::Password)
			{
				FGridLayout->addWidget(lblPassword, i, 0);
				FGridLayout->addWidget(ledPassword, i++, 1);
                if(!AFields.password.isNull())
					ledPassword->setText(AFields.password);
				connect(ledPassword,SIGNAL(textChanged(QString)),SLOT(onPassEditChanged(QString)));
            }
			if(AFields.fieldMask & IRegisterFields::Email)
			{
				FGridLayout->addWidget(lblEmail, i, 0);
				FGridLayout->addWidget(ledEmail, i++, 1);
                if(!AFields.email.isNull())
					ledEmail->setText(AFields.email);
				connect(ledEmail,SIGNAL(textChanged(QString)),SLOT(onEmailEditChanged(QString)));
            }
//FIXME: Redirect should not work this way!
			if(AFields.fieldMask & IRegisterFields::Redirect)
			{
				FGridLayout->addWidget(lblUrl, i, 0);
				FGridLayout->addWidget(ledUrl, i++, 1);
				if(AFields.redirect.isValid())
					ledUrl->setText(AFields.redirect.toString());
			   connect(ledUrl,SIGNAL(textChanged(QString)),SLOT(onUrlEditChanged(QString)));
			}
        }
        else //! form.type = "form"
        {
            FDirection=false;
            if (!AFields.form.instructions.isEmpty())   //! many instr...
				instructions=AFields.form.instructions[0];
            QList<IDataField> fields= AFields.form.fields;
			for(QList<IDataField>::ConstIterator it=fields.constBegin(); it!=fields.constEnd(); it++)
            {
				if((*it).type==FIELD_TYPE_HIDDEN)    //! similar to an <INPUT> tag
                {
					if((*it).var=="FORM_TYPE")
						FTmpFields.insert((*it).var, (*it).value.toString());
                }
                else
                {
                    //!--exception Handling
					if(checkField((*it), FServiceTo.full()))
                    {
						QWidget *widget = getWidget(*it);
						if(widget)
						{
							QLabel *lbl=new QLabel(getLocalText((*it).var));
							lbl->setFixedWidth(100);
							lbl->setWordWrap(true);
							FGridLayout->addWidget(lbl, i, 0, Qt::AlignLeft);
							FGridLayout->addWidget(widget, i++, 1, Qt::AlignLeft);
                            if (qobject_cast<QLineEdit*>(widget) || qobject_cast<QLineEdit*>(widget))
                                registerField((*it).var+'*', widget);
						}
                    }
                }
            }			
        }
		FGridLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), i, 2);
		FInstrLabel->setText(QString("%1").arg(instructions));
		emit completeChanged();
    }
}

bool ProcessPage::checkField(const IDataField AField, QString AGateWay)
{
	bool visible = true;
    for(int i=0;i<FExcepFields.size();i++)
    {
        QDomElement f=FExcepFields[i];
        if(AGateWay==f.attribute("jid") && AField.var==f.attribute("var"))
        {
            visible = false;
			QDomElement value = f.firstChildElement("value");
			if(!value.isNull())
				FTmpFields.insert(AField.var, value.text().isEmpty()?AField.value:value.text());
        }
    }
	return visible;
}

void ProcessPage::initializePage()
{
	FScrollArea->setWidget(new QWidget);
	FScrollArea->widget()->setLayout(FGridLayout = new QGridLayout);
}

//! bool required; QString var; QString type; QString desc; QVariant value;
//! IDataMedia media;IDataValidate validate; QList<IDataOption> options;
QWidget *ProcessPage::getWidget(const IDataField &AField)
{
	QString  var	= AField.var;
	QString  label	= !AField.label.isEmpty() ? AField.label : AField.desc;
	QString  descr	= !AField.desc.isEmpty()  ? QString("<span>%1</span>").arg(HTML_ESCAPE(AField.desc)) : QString::null;
	QVariant value	= AField.value;

	if(var=="link")
	{
		QPushButton *pbLink=new QPushButton;
		pbLink->setText(value.toString());
		pbLink->setToolTip(descr);
		pbLink->setFixedSize(FFieldWidth,24);
		connect(pbLink,SIGNAL(clicked()),SLOT(onLinkActivated()));
		return pbLink;
	}
	else if(AField.type==FIELD_TYPE_FIXED )
	{
		QLabel *lbl=new QLabel(value.toString());
        lbl->setWordWrap(true);
        lbl->setTextFormat(Qt::AutoText);
		lbl->setToolTip(descr);
		lbl->setFixedWidth(FFieldWidth);
        return lbl;
	}
	else if(AField.type==FIELD_TYPE_BOOLEAN)
	{
        QCheckBox *checkBox=new QCheckBox;
		checkBox->setObjectName(var);
		checkBox->setText(label);
		checkBox->setToolTip(descr);
		checkBox->setChecked(value.toBool());
		FTmpFields.insert(var,value);
        connect(checkBox,SIGNAL(clicked(bool)),SLOT(onFormClicked(bool)));
        return checkBox;
	}
	else if(AField.type==FIELD_TYPE_TEXTSINGLE)
	{
        QLineEdit *edit=new QLineEdit;
		edit->setObjectName(var);
		edit->setToolTip(descr);
		edit->setFixedWidth(FFieldWidth);
		if(!value.isNull())
		{
			edit->setText(value.toString());
			FTmpFields.insert(var,value);
        }
        connect(edit,SIGNAL(textChanged(QString)),SLOT(onTextChanged(QString)));
        return edit;
	}
	else if(AField.type==FIELD_TYPE_TEXTPRIVATE)
	{
        QLineEdit *editPass=new QLineEdit;
        editPass->setEchoMode(QLineEdit::Password);
		editPass->setObjectName(var);
		editPass->setToolTip(descr);
		editPass->setFixedWidth(FFieldWidth);
        connect(editPass,SIGNAL(textChanged(QString)),SLOT(onTextChanged(QString)));
        return editPass;
	}
	else if(AField.type==FIELD_TYPE_LISTSINGLE || AField.type==FIELD_TYPE_JIDSINGLE)
	{
        QComboBox *comBox=new QComboBox;
		comBox->setObjectName(var);
		comBox->setToolTip(descr);
		for(QList<IDataOption>::ConstIterator it=AField.options.constBegin(); it!=AField.options.constEnd(); it++)
			comBox->addItem((*it).label, (*it).value);
		comBox->setEditText(value.toString());
		FTmpFields.insert(var,value);
        comBox->setFixedWidth(comBox->sizeHint().width());
        connect(comBox,SIGNAL(currentIndexChanged(QString)),SLOT(onComBoxChanged(QString)));
        return comBox;
        //  FLineEdit->setText(Jid(AValue.toString()).uFull());
	}
	else if(AField.type==FIELD_TYPE_TEXTMULTI)
	{
        QTextEdit *editText=new QTextEdit;
		editText->setObjectName(var);
        editText->setAcceptRichText(false);
		editText->setToolTip(descr);
		editText->setFixedWidth(FFieldWidth);
        editText->clear();
		if(!value.isNull())
		{
			QStringList list = value.toStringList();
			for(QStringList::ConstIterator it = list.constBegin(); it!=list.constEnd(); it++)
				editText->append(*it);
        }
		FTmpFields.insert(var,value);
        connect(editText,SIGNAL(textChanged()),SLOT(onMultiTextChanged()));
        return editText;
	}
	else if(AField.type==FIELD_TYPE_JIDMULTI){
        QTextEdit *editText=new QTextEdit;
		editText->setObjectName(var);
        editText->setAcceptRichText(false);
		editText->setToolTip(descr);
		editText->setFixedWidth(FFieldWidth);
        editText->clear();
		if(!value.isNull())
		{
			QStringList list = value.toStringList();
			for(QStringList::ConstIterator it = list.constBegin(); it!=list.constEnd(); it++)
				editText->append(Jid(*it).uFull());
		}
		FTmpFields.insert(var,value);
		connect(editText,SIGNAL(textChanged()),SLOT(onMultiTextChanged()));
        return editText;
	}
	else if(AField.type==FIELD_TYPE_LISTMULTI)
	{
        QListWidget *listWdg=new QListWidget;
		listWdg->setObjectName(var);
		listWdg->setToolTip(descr);
		listWdg->setFixedWidth(FFieldWidth);
		for(QList<IDataOption>::ConstIterator it=AField.options.constBegin(); it!=AField.options.constEnd(); it++)
        {
			QListWidgetItem *item = new QListWidgetItem((*it).label);
			item->setData(Qt::UserRole, (*it).value);
            item->setFlags(!true ? Qt::ItemIsEnabled|Qt::ItemIsUserCheckable : Qt::ItemIsEnabled);
            listWdg->addItem(item);
        }
        listWdg->setWrapping(true);
		connect(listWdg,SIGNAL(currentTextChanged(QString)),SLOT(onListMultiChanged(QString)));
        return listWdg;
    }
	return NULL;
}
//! FField.validate.method == DATAVALIDATE_METHOD_OPEN
//! FField.validate.type == DATAVALIDATE_TYPE_DATE
//! FField.validate.type == DATAVALIDATE_TYPE_TIME

void ProcessPage::onFormClicked(bool st)
{
    QCheckBox *obj= qobject_cast<QCheckBox *>(sender());
    if(obj)
		FTmpFields.insert(obj->objectName(),st);
}
void ProcessPage::onTextChanged(QString text)
{
    QLineEdit *obj= qobject_cast<QLineEdit *>(sender());
    if(obj)
		FTmpFields.insert(obj->objectName(),text);   //! var-name,value
}
void ProcessPage::onComBoxChanged(QString text)
{
    QComboBox *obj= qobject_cast<QComboBox *>(sender());
    if(obj)
		FTmpFields.insert(obj->objectName(),text);
}
void ProcessPage::onListMultiChanged(QString text)
{
    QListWidget *obj= qobject_cast<QListWidget *>(sender());
    if(obj)
		FTmpFields.insert(obj->objectName(),text);
}
void ProcessPage::onMultiTextChanged()
{
    QTextEdit *obj= qobject_cast<QTextEdit *>(sender());
    if(!obj->document()->isEmpty())
		FTmpFields.insert(obj->objectName(),obj->toPlainText());
}
void ProcessPage::onLinkActivated()
{
    QPushButton *obj= qobject_cast<QPushButton *>(sender());
    if(!obj->text().isNull())
        QDesktopServices::openUrl(QUrl(obj->text()));
}

void ProcessPage::onUserEditChanged(QString text)	{FUserName=text;}
void ProcessPage::onPassEditChanged(QString text)	{FPassword=text;}
void ProcessPage::onEmailEditChanged(QString text)	{FEmail=text;}
void ProcessPage::onUrlEditChanged(QString text)	{FUrl=text;}

void ProcessPage::onClicked(bool st)
{
    Options::node(OPV_ROSTER_AUTOSUBSCRIBE).setValue(st);
}

void ProcessPage::doSubmit()
{
    //! FOperation = IRegistration::Register
     if(FDirection)
     {
         FSubmit.username   = FUserName;
         FSubmit.password   = FPassword;
         FSubmit.email      = FEmail;
         FSubmit.form       = IDataForm();
     }
     else
     {
         QList<IDataField> FNewFields;
		 QHashIterator<QString, QVariant> i(FTmpFields);
         while (i.hasNext())
         {
             IDataField field;
             i.next();
             field.var=i.key();
             field.value=i.value();
             FNewFields.append(field);
         }
         FForm.fields=FNewFields;
         FForm.type="submit";
         if(!FForm.instructions.isEmpty())
            FForm.instructions.clear();
         if(!FForm.title.isEmpty())
            FForm.title.clear();
         FSubmit.form = FForm;
     }
}

void ProcessPage::onRegisterError(const QString &AId, const XmppError &AError)
{
    if (FRequestId == AId)
    {
        //! "Remote server not found"
        QString style="style='color:red;'";
        FErrorLabel->setText(QString("<h2>%1:<span %2><br/>%3</span></h2>")
							 .arg(tr("Requested operation failed")).arg(style).arg(AError.errorMessage()));
        FInstrLabel->clear();
		FAutoRegCheckBox->setVisible(false);
    }
}

QString ProcessPage::getLocalText(QString AKey)
{
    if(FLocalText.value(AKey).isEmpty())
        return AKey;
    else
        return FLocalText.value(AKey);
}

int ProcessPage::nextId() const
{
	return TransportWizard::Page_Result;
}

//!------------------------------
ResultPage::ResultPage(Jid &AStreamJid, IRegistration *ARegistration, ProcessPage *AProcess, QWidget *parent):
    QWizardPage(parent),
    FStreamJid(AStreamJid),
    FRegistration(ARegistration),
    FProcess(AProcess),
    FWizardGo(false)
{
    QString style="style='color:blue;'";
	setTitle(QString("<span %2>%1</span>").arg(tr("Result Page")).arg(style));
	setSubTitle(QString("<span %2>%1</span>").arg(tr("Result Page")).arg(style));

    FErrorLabel =new QLabel;
    FErrorLabel->setWordWrap(true);
    FErrorLabel->setAlignment(Qt::AlignHCenter);

    layout = new QVBoxLayout;
    setLayout(layout);

    if (FRegistration)
    {
        connect(FRegistration->instance(),SIGNAL(registerError(const QString &, const XmppError &)),
            SLOT(onRegisterError(const QString &, const XmppError &)));
		connect(FRegistration->instance(),SIGNAL(registerSuccess(const QString &)),
            SLOT(onRegisterSuccessful(const QString &)));
    }
}

void ResultPage::onGetRegister()
{
	FSubmit = FProcess->getSubmit();
	FRequestId = FRegistration->sendRequestSubmit(FStreamJid,FSubmit);
}

void ResultPage::onRegisterError(const QString &AId, const XmppError &AError)
{
    if (FRequestId == AId)
    {
        layout->addWidget(FErrorLabel);

        FWizardGo=false;
        emit completeChanged();
        QString style="style='color:red;'";
        FErrorLabel->setText(QString("<h2>%1 <span %2><br/>%3</span></h2>")
				 .arg(tr("Requested operation failed:")).arg(style).arg(AError.errorMessage()));
		if(AError.errorString()=="Undefined error condition")
            setButtonText(QWizard::BackButton,tr("Retry"));
    }
}

void ResultPage::onRegisterSuccessful(const QString &AId)
{
    if (FRequestId == AId)
        wizard()->next();
}

bool ResultPage::isComplete() const
{
    return FWizardGo;
}

int ResultPage::nextId() const
{
	return TransportWizard::Page_Conclusion;
}


//!------------------------------
ConclusionPage::ConclusionPage(NetworksPage *ANetworkPage, QWidget *parent):
	QWizardPage(parent),
	FNetworkPage(ANetworkPage)
{
    QString style="style='color:blue;'";
    setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Done!")));
    setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Transport Wizard completed successfuly")));

    QString fileName = IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->fileFullName(MNI_WIZARD_GT_END);
    setPixmap(QWizard::WatermarkPixmap, QPixmap(fileName));
	FLblTitle = new QLabel(QString("<center><h2 style='color:green;'>%2</h2></center>").arg(tr("Congratulations!")));
	FLblTitle->setWordWrap(true);

	FLblText1 = new QLabel;
	FLblText1->setWordWrap(true);

	FLblText2 = new QLabel(QString("<b>%1: </b>%2").arg(tr("Attention"))
						   .arg(tr("Some transports may report about successful registreation, even if wrong credentials were provided. "
								   "They may inform you about authentication error later, with a message.\n"
								   "If you receive such message, please rerun the Wizard to register at transport with correct credentials.")));
	FLblText2->setWordWrap(true);

	FLblText3 = new QLabel(tr("Press \"Finish\" button to close Wizard.", "\"Finish\" should match the text of an appropriate Qt Wizard button"));
	FLblText3->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(FLblTitle);
	layout->addSpacerItem(new QSpacerItem(0, 10, QSizePolicy::Fixed));
	layout->addWidget(FLblText1);
	layout->addWidget(FLblText2);
	layout->addWidget(FLblText3);
    setLayout(layout);
}

int ConclusionPage::nextId() const
{
    return -1;
}

void ConclusionPage::initializePage()
{
	FLblText1->setText(QString("<span style='align:justify'>%1</span>")
					  .arg(tr("You successfuly connected to %1 via %2.").arg(FNetworkPage->networkName(field("network").toString()))
																		.arg(field("gatewayselected").toString())));
}

void ProcessPage::localTextLabel()
{
    FLocalText.insert("address",tr("Street"));
    FLocalText.insert("action",tr("Select Action"));
    FLocalText.insert("birthyear",tr("Birth Year"));
    FLocalText.insert("born",tr("Born"));
    FLocalText.insert("city",tr("City"));
    FLocalText.insert("connections_params",tr("Connections Parameters"));
    FLocalText.insert("email",tr("Email Address"));
    FLocalText.insert("encoding",tr("Encoding"));
    FLocalText.insert("first",tr("Given Name"));
    FLocalText.insert("firstname",tr("First Name"));
    FLocalText.insert("familyname",tr("Family Name"));
    FLocalText.insert("familycity",tr("Family City"));
    FLocalText.insert("friends_only",tr("Friends Only"));
    FLocalText.insert("invisible",tr("Invisible"));
    FLocalText.insert("gender",tr("Gender"));
    FLocalText.insert("lastname",tr("Last Name"));
    FLocalText.insert("last",tr("Family Name"));
    FLocalText.insert("language",tr("Language"));
    FLocalText.insert("link",tr("Link"));
    FLocalText.insert("locale",tr("Language"));
    FLocalText.insert("nick",tr("Nick Name"));
    FLocalText.insert("name",tr("Full Name"));
    FLocalText.insert("Password",tr("Password"));
    FLocalText.insert("password",tr("Password"));
	FLocalText.insert("phone",tr("Phone Number"));
    FLocalText.insert("state",tr("Region"));
    FLocalText.insert("uin",tr("UIN"));
    FLocalText.insert("unregister",tr("Unregister"));
    FLocalText.insert("url",tr("Your Web Page"));
	FLocalText.insert("username",tr("User Name"));
    FLocalText.insert("use_password",tr("Access-token"));
    FLocalText.insert("userlist",tr("User List"));
    FLocalText.insert("zip",tr("Zip Code"));
}
