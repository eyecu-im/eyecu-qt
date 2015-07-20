#include <QDir>
#include <QDesktopServices>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QCloseEvent>

#include <utils/pluginhelper.h>

#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/networkicons.h>
#include <definitions/serviceicons.h>
#include <definitions/version.h>
#include <definitions/optionvalues.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodes.h>

#include "wizardpages.h"
#include "networkwarning.h"

#define ACCOUNT_CONNECTION_OPTIONS     Options::node(OPV_ACCOUNT_CONNECTION_ITEM,"CreateAccountWizard")
#define SERVERS_DEF  "servers.xml"

// Field names
#define WF_NETWORK_SELECTED		"network.selected"
#define WF_SERVER_NAME_PRE		"server.name.pre"
#define WF_SERVER_NAME			"server.name"
#define WF_USER_NAME			"user.name"
#define WF_USER_PASSWORD		"user.password"
#define WF_USER_PASSWORD_CONF	"user.password.conf"
#define WF_USER_RESOURCE		"user.resource"
#define WF_ACCOUNT_NAME			"account.name"
#define WF_GO_ONLINE			"go.online"

ConnectionWizard::ConnectionWizard(QWidget *AParent):
    QWizard(AParent),
	FConnectionManager(PluginHelper::pluginInstance<IConnectionManager>()),
	FCurrentId(-1),
	FRegisterAccount(false)
{
	setAttribute(Qt::WA_DeleteOnClose); // To delete it automatically

	IConnectionEngine	*connectionEngine = NULL;
	IAccountManager		*accountManager = PluginHelper::pluginInstance<IAccountManager>();

	if (FConnectionManager)
	{
		QString engineId = Options::defaultValue(OPV_ACCOUNT_CONNECTION_TYPE).toString();
		if (!FConnectionManager->connectionEngines().contains(engineId))
			engineId = FConnectionManager->connectionEngines().value(0);
		connectionEngine = FConnectionManager->findConnectionEngine(engineId);
		FConnectionSettingsWidget = connectionEngine->connectionSettingsWidget(ACCOUNT_CONNECTION_OPTIONS, NULL);
	}
	setPage(Page_Intro, new IntroPage);
	NetworkPage *networkPage = new NetworkPage(this);
	setPage(Page_Network, networkPage);
	ServerPage *serverPage = new ServerPage(networkPage, this);
	setPage(Page_Server, serverPage);

	setPage(Page_Credentials, new CredentialsPage(accountManager, this));
	setPage(Page_Connection, new ConnectionPage(serverPage, FConnectionSettingsWidget, this));

	ConnectPage *connectPage = new ConnectPage(connectionEngine, this);
	setPage(Page_Connect, connectPage);
	setPage(Page_Submit, new RegisterSubmitPage(connectPage, this));

	setPage(Page_InfoWeb, new WebRegistrationInfo(serverPage, this));
	setPage(Page_Conclusion, new ConclusionPage(accountManager, connectionEngine, FConnectionSettingsWidget, this));

	setStartId(Page_Intro);
	setOptions(NoBackButtonOnLastPage|NoBackButtonOnStartPage|NoCancelButton);

#ifndef Q_WS_MAC
	setWizardStyle(ModernStyle);
#endif

	QString fileName = IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->fileFullName(MNI_WIZARD);
	setPixmap(QWizard::LogoPixmap, QPixmap(fileName));//LogoPixmap WatermarkPixmap

	fileName = IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->fileFullName(MNI_WIZARD_ACC_BAN);
	setPixmap(QWizard::BannerPixmap, QPixmap(fileName));

	setWindowTitle(tr("Connection Wizard"));
	setWindowIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_MAINWINDOW_LOGO16));

	connect(this, SIGNAL(currentIdChanged(int)),SLOT(onCurrentIdChanged(int)));
}

ConnectionWizard::~ConnectionWizard()
{
	OptionsNode node = ACCOUNT_CONNECTION_OPTIONS;
	node.parent().removeNode(node.name(), node.nspace());
}

QString ConnectionWizard::serverName() const
{
	return (field(WF_SERVER_NAME).toString().isEmpty()?field(WF_SERVER_NAME_PRE):field(WF_SERVER_NAME)).toString();
}

QString ConnectionWizard::streamJid() const
{
	return Jid(field(WF_USER_NAME).toString(), serverName(), CLIENT_NAME).full();
}

void ConnectionWizard::onCurrentIdChanged(int AId)
{
	if (AId == Page_Connect && (FCurrentId > Page_Connect)) // Go back
	{
		page(Page_Connect)->cleanupPage();
		page(Page_Connect)->initializePage();
	}
//		back();
//	else
		FCurrentId = AId;
}

//!------------------------------
IntroPage::IntroPage(QWidget *parent):
	QWizardPage(parent)
{
    QString style="style='color:blue;'";
    setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Connect to Jabber")));
    setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("This wizard will help you to create a Jabber account")));

    setPixmap(QWizard::WatermarkPixmap, IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->getIcon(MNI_WIZARD_ACC1).pixmap(2048));

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(FLblTop               = new QLabel(QString("<span %1>%2</span>").arg(style).arg(tr("Are you already registered at Jabber network?"))));
    FLblTop->setWordWrap(true);
    layout->addWidget(FClbHaveAccountYes    = new QCommandLinkButton(tr("&Yes"), tr("I have an account on a Jabber server")));
    layout->addWidget(FClbHaveAccountNo		= new QCommandLinkButton(tr("&No"), tr("I want to register on a Jabber server")));
    setLayout(layout);

    connect(FClbHaveAccountYes, SIGNAL(clicked()), SLOT(onClicked()));
    connect(FClbHaveAccountNo, SIGNAL(clicked()), SLOT(onClicked()));
}

void IntroPage::onClicked()
{
	if (sender() == FClbHaveAccountNo)
	{
		wizard()->setProperty("registerAccount", true);
		FNextId = ConnectionWizard::Page_Server;
	}
	else
	{
		wizard()->setProperty("registerAccount", false);
		FNextId = ConnectionWizard::Page_Network;
	}
    wizard()->next();
}

//!------------------------------
NetworkPage::NetworkPage(QWidget *AParent): QWizardPage(AParent)
{
	FNetworks.insert(NetworkOther,			NetworkInfo(tr("Other XMPP")   , NWI_XMPP,			tr("An independent XMPP server (Jabber)")));
	FNetworks.insert(NetworkGoogle,			NetworkInfo(tr("Google Talk")  , NWI_GTALK,			tr("A social network and chat service from Google"), "https://accounts.google.com/SignUp?service=mail",
														QStringList() << "gmail.com" << "googlemail.com"));
	FNetworks.insert(NetworkYandex,			NetworkInfo(tr("Yandex Online"), NWI_YAONLINE,		tr("A popular Russian portal (internet serach, e-mail, news, chat and so on)"), "https://passport.yandex.com/registration/mail?from=mail&require_hint=1",
														QStringList() << "ya.ru" << "yandex.ru" << "yandex.net" << "yandex.com" << "yandex.by" << "yandex.kz" << "yandex.ua" << "yandex-co.ru" << "narod.ru"));
	FNetworks.insert(NetworkOdnoklassniki,	NetworkInfo(tr("Odnoklassniki"), NWI_ODNOKLASSNIKI,	tr("A popular Russian social network, owned by Mail.Ru Group"), "http://ok.ru/dk?st.cmd=anonymRegistrationEdit",
														QStringList() << "odnoklassniki.ru"));
	FNetworks.insert(NetworkLiveJournal,	NetworkInfo(tr("LiveJournal")  , NWI_LIVEJOURNAL,	tr("A popular blogging service"), "https://www.livejournal.com/create",
														QStringList() << "livejournal.com"));
	FNetworks.insert(NetworkQIP,			NetworkInfo(tr("QIP")          , NWI_QIP,			tr("A popular Russian portal (internet serach, e-mail, news, chat and so on), owned by OOO \"Media Mir\", mostly known by its multiprotocol IM client"), "http://qip.ru/mnt/pages/register",
														QStringList() << "qip.ru" << "pochta.ru" << "fromru.ru" << "front.ru" << "hotbox.ru" << "hotmail.ru" << "nightmail.ru" << "nm.ru" << "pisem.net" << "pochtamt.ru" << "newmail.ru" << "krovatka.su" << "land.ru" << "mail5.com" << "mail333.com" << "pop3.ru" << "rbcmail.ru" << "smtp.ru" << "5ballov.ru" << "aeterna.ru" << "ziza.ru" << "memori.ru" << "photofile.ru" << "fotoplenka.ru"));

	QString style="style='color:blue;'";
	setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Network selection")));
	setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Please, select a network, you have registered in")));

	QLabel *lblNotice = new QLabel(tr("There are some social networks, portals or blogging services, which have their own XMPP servers. "
				"If you have an account in some of those networks, you may use it in eyeCU.\n"
				"Please, note, that those XMPP servers usually have implemented their own specific features, "
				"which cannot be used with standard XMPP clients, like eyeCU. "
				"At the same time, they implenment only limited set of standard XMPP extensions, "
				"which could be used with standard XMPP clients."
				"So, it's hardly recommended to use such accounts only as additional account, to have convenient access "
				"to your social networks and services. To have full-featured XMPP experienece, it's recommended to have "
				"an account on independent XMPP (Jabber) server. To connect to such server, please, select \"Other XMPP\".\n"
				"If you don't have a Jabber account yet, please, go Back and select \"No\" to register in Jabber."));
	lblNotice->setWordWrap(true);
	FLwNetworkList = new QListWidget();
	FLwNetworkList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	connect(FLwNetworkList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(onCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));
	connect(FLwNetworkList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), parent(), SLOT(next()));
	registerField(WF_NETWORK_SELECTED, FLwNetworkList);
	IconStorage *iconStorage = IconStorage::staticStorage(RSR_STORAGE_WIZARDS);
	for (QMap<NetworkType, NetworkInfo>::ConstIterator it = FNetworks.constBegin(); it!=FNetworks.constEnd(); it++)
		(new QListWidgetItem(iconStorage->getIcon((*it).icon), (*it).name, FLwNetworkList, it.key()))->setData(NetworkDescription, (*it).description);
	QGridLayout *layout = new QGridLayout();
	layout->addWidget(lblNotice, 0, 0, 1, 2);
	layout->addWidget(FLwNetworkList, 1, 0, 1, 1);
	QGroupBox *gpbDescription = new QGroupBox(tr("Description"));
	gpbDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *lytDescription = new QVBoxLayout();
	FLblDescription = new QLabel("Test");
	FLblDescription->setWordWrap(true);
	FLblDescription->setAlignment(Qt::AlignTop);
	lytDescription->addWidget(FLblDescription);
	gpbDescription->setLayout(lytDescription);
	layout->addWidget(gpbDescription, 1, 1, 1, 1);
	setLayout(layout);
}

NetworkPage::NetworkType NetworkPage::serverNetwork(const QString &AServer) const
{
	for (QMap<NetworkType, NetworkInfo>::ConstIterator it=FNetworks.constBegin(); it!=FNetworks.constEnd(); it++)
		if ((*it).servers.contains(AServer))
			return it.key();
	return NetworkOther;
}

NetworkPage::NetworkInfo NetworkPage::networkInfo(NetworkPage::NetworkType ANetworkType) const
{
	if ((ANetworkType < NetworkOther) && (ANetworkType >= Network_Count))
		return NetworkInfo();
	else
		return FNetworks[ANetworkType];
}

void NetworkPage::initializePage()
{

}

void NetworkPage::onCurrentItemChanged(QListWidgetItem *ACurrent, QListWidgetItem *APrevious)
{
	Q_UNUSED(APrevious)
	if (ACurrent)
	{
		FLblDescription->setText(ACurrent->data(NetworkDescription).toString());
		FNextId = ACurrent->type() == NetworkOther?ConnectionWizard::Page_Server:ConnectionWizard::Page_Credentials;
	}
}

//!------------------------------
ServerPage::ServerPage(NetworkPage *ANtworkPage, QWidget *AParent):
	QWizardPage(AParent), FNetworkPage(ANtworkPage)
{
    QString style="style='color:blue;'";
	setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Server selection")));
    setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Specify a server you want to use.")));

	QLabel *lblServerList = new QLabel(QString("<b>%1:</b>").arg(tr("Please select a server from the list")));
	FServerList = new QTreeWidget();
	FServerList->setItemsExpandable(false);
	FServerList->setRootIsDecorated(false);
	lblServerList->setBuddy(FServerList);

	QLabel *serverValue = new QLabel(QString("&<b>%1:</b>").arg(tr("Or enter manually")));
	FLedSelectedServer= new QLineEdit;
	serverValue->setBuddy(FLedSelectedServer);

	registerField(WF_SERVER_NAME_PRE"*", FLedSelectedServer);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(lblServerList);
	layout->addWidget(FServerList);
	layout->addWidget(serverValue);
	layout->addWidget(FLedSelectedServer);
	setLayout(layout);

    loadServerList();

    // Tricky code to remove item selection on first widget show
    QTreeWidgetItem *first = FServerList->topLevelItem(0);
    if (first)
        FServerList->setCurrentItem(first);

	connect(FServerList,SIGNAL(itemSelectionChanged()),SLOT(onItemSelectionChanged()));
}

QUrl ServerPage::getRegistrationUrl() const
{
	return FServerInfo.value(field(WF_SERVER_NAME).toString()).url;
}

QString ServerPage::getInstructions() const
{
	return FServerInfo.value(field(WF_SERVER_NAME).toString()).instructions;
}

int ServerPage::getFlags() const
{
	return FServerInfo.value(field(WF_SERVER_NAME).toString()).flags;
}

//! listig fields
//! {ICQ,AIM,IRC,MSN,Gadu-Gadu,SMS,Yahoo,FaceBook,Google Talk,XMPP,Vk,MailRU,Live Jornal}
void ServerPage::loadServerList()
{
    IconStorage *storageWizards = IconStorage::staticStorage(RSR_STORAGE_WIZARDS);
    IconStorage *storageCountry = IconStorage::staticStorage(RSR_STORAGE_COUNTRY);
    IconStorage *storageMenu = IconStorage::staticStorage(RSR_STORAGE_MENUICONS);
    IconStorage *storageService = IconStorage::staticStorage(RSR_STORAGE_SERVICEICONS);
    QDir dir(storageWizards->resourcesDirs().first());
    dir.cd(storageWizards->storage());
    dir.cd(storageWizards->subStorage());
    QFile file(dir.absoluteFilePath(SERVERS_DEF));

    if(file.open(QFile::ReadOnly))
    {
        QDomDocument doc;
        if(doc.setContent(file.readAll(), true))
        {
            int columnWidth = 160;
			FServerList->sortItems(0, Qt::AscendingOrder);
            QStringList headers;
			headers << tr("Server")<< tr("Registration") << tr("PEP") << tr("Message Archive")
					<< tr("Message Carbons") << tr("User Search") << tr("MUC") << tr("Proxy")
					<< tr("File Store") << tr("Transports")  << tr("Country");
			FServerList->setHeaderLabels(headers);
            FServerList->setColumnWidth(0, columnWidth);//icon,jid
			FServerList->setColumnWidth(1, 80); //Registration type=[??=0,server=1,web=2,both=3]
			FServerList->setColumnWidth(2, 30); //pep
			FServerList->setColumnWidth(3, 30); //Message Archive
			FServerList->setColumnWidth(4, 30); //Message Carbons
			FServerList->setColumnWidth(5, 30); //vjud
			FServerList->setColumnWidth(6, 30); //muc
			FServerList->setColumnWidth(7, 30); //proxy
			FServerList->setColumnWidth(8, 30); //Store Files
			FServerList->setColumnWidth(9, 280); //Transports
			FServerList->setColumnWidth(10, 35); //country

            for (QDomElement e = doc.documentElement().firstChildElement("server");
                 !e.isNull();
                 e = e.nextSiblingElement("server"))
            {
				ServerInfo serverInfo(true);
                QString jid = e.firstChildElement("jid").text();
                //Registration type=[Unknown=0,Server=1,Web=2,Both=3]
                //!----------------------
				QDomElement registration = e.firstChildElement("registration");
				QString registrationType = registration.attribute("type");

				if(!registration.firstChildElement("url").isNull())
					serverInfo.url = QUrl::fromEncoded(e.firstChildElement("registration").firstChildElement("url").text().toLatin1());
				if(!registration.firstChildElement("instructions").isNull())
					serverInfo.instructions = registration.firstChildElement("instructions").text();
                if(registrationType=="in-band" || registrationType=="both")
                    serverInfo.flags|=ServerInfo::InBandRegistration;
                //!----------------------

				if (e.firstChildElement("legacy").text()=="true")
					serverInfo.flags |= ServerInfo::LegacySSL;
				if (e.firstChildElement("compression").text()=="true")
					serverInfo.flags |= ServerInfo::StreamCompress;                

                QString trusted = e.firstChildElement("trusted").text();
                QString soft = e.firstChildElement("soft").text();
                QString pep = e.firstChildElement("pep").text();
				QString archive = e.firstChildElement("archive").text();
				QString carbons = e.firstChildElement("carbons").text();
				QString vjud = e.firstChildElement("vjud").text();
                QString muc = e.firstChildElement("muc").text();
                QString proxy = e.firstChildElement("proxy").text();
                QString transports = e.firstChildElement("transports").text();
                QString filestore = e.firstChildElement("filestore").text();
                QString country = e.firstChildElement("country").text();

				QTreeWidgetItem* pItem = new QTreeWidgetItem(FServerList);
                pItem->setText(0, jid);
                pItem->setIcon(0, storageWizards->getIcon(soft));
				pItem->setToolTip(0, soft);
                if (serverInfo.flags&ServerInfo::InBandRegistration)
                {
					pItem->setIcon(1, storageMenu->getIcon(MNI_REGISTRATION));
                    pItem->setToolTip(1, tr("In-band registration available"));
                }

                if(pep == "true")
                {
                     pItem->setIcon(2, storageService->getIcon(SRI_PUBSUB_PEP));
                     serverInfo.flags |= ServerInfo::Pep;
                }
				if(archive == "true")
					pItem->setIcon(3, storageMenu->getIcon(MNI_HISTORY));
				if(carbons == "true")
					pItem->setIcon(4, storageMenu->getIcon(MNI_MESSAGECARBONS));
				if(vjud == "true")
					pItem->setIcon(5, storageService->getIcon(SRI_DIRECTORY_USER));
                if(muc == "true")
					pItem->setIcon(6, storageService->getIcon(SRI_CONFERENCE_TEXT));
                if(proxy == "true")
					pItem->setIcon(7, storageService->getIcon(SRI_PROXY_BYTESTREAMS));
                if(filestore == "true")
					pItem->setIcon(8, storageService->getIcon("store/file"));
				pItem->setText(9, transports);
				pItem->setIcon(10, storageCountry->getIcon(country));
				pItem->setToolTip(10, country);

                FServerInfo.insert(jid, serverInfo);
             }
        }
	}
}

void ServerPage::initializePage()
{
    FServerList->clearSelection();
	connect(FServerList, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), wizard(), SLOT(next()), Qt::UniqueConnection);
}

bool ServerPage::validatePage()
{
    int flags = FServerInfo.value(FLedSelectedServer->text().toLower()).flags;
    if (flags&ServerInfo::Valid && !(flags&ServerInfo::Pep))
        if (QMessageBox::warning(wizard(), tr("Warning!"),
                                           tr("The Server you selected do not support Personal Events (PEP)!\n"
                                              "Some %1 features (like User Location, User Activity, User Mood and User Tune) will not work!\n"
                                              "Using this server for your primary account is deprecated.\n"
											  "Press \"Ok\" to proceed or \"Cancel\" to select another server.").arg(CLIENT_NAME), QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Cancel)
            return false;

    if (wizard()->property("registerAccount").toBool())
	{
		NetworkPage::NetworkType networkType = FNetworkPage->serverNetwork(FLedSelectedServer->text().toLower());
		if (networkType!=NetworkPage::NetworkOther)
		{
			NetworkPage::NetworkInfo info = FNetworkPage->networkInfo(networkType);
			(new NetworkWarning(tr("Attention!"),
				tr( "The Server you selected belongs to %1 network!\n"
					"It is not recommended to use such servers as your primary account, "
					"due to restricted functionality they have. "
					"In-band registration is also unavailable.\n"
					"Please, choose another server. "
					"If you have an account in such network, please, "
					"go back and select \"Yes\" on the first page. "
					"If you don't have an account, but want to have it, "
					"you'll need to visit %2 to register first.")
					.arg(QString("<b>%1</b>").arg(info.name))
					.arg(QString("<a href=\"%2\">%1</a>").arg(tr("%1 website").arg(info.name)).arg(QString::fromLatin1(info.url.toEncoded())))))->exec();
			return false;
		}
		else if (flags&ServerInfo::Valid && !(flags&ServerInfo::InBandRegistration))
            if (QMessageBox::warning(wizard(), tr("Attention!"), tr("The Server you selected do not support in-band registration!\nYou'll have to register via web!\nPress \"Ok\" to proceed or \"Cancel\" to select another server."), QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Cancel)
				return false;
	}

	return true;
}

int ServerPage::nextId() const
{
	return wizard()->property("registerAccount").toBool()?ConnectionWizard::Page_Connection:ConnectionWizard::Page_Credentials;
}

void ServerPage::onButtonClicked(QAbstractButton *AButton)
{
	if (qobject_cast<QMessageBox *>(sender())->buttonRole(AButton)==QMessageBox::ActionRole)
	{
		NetworkPage::NetworkType networkType = FNetworkPage->serverNetwork(FLedSelectedServer->text().toLower());
		NetworkPage::NetworkInfo info = FNetworkPage->networkInfo(networkType);
		QDesktopServices::openUrl(info.url);
	}
}


void ServerPage::onItemSelectionChanged()
{
    QList<QTreeWidgetItem *> selection = FServerList->selectedItems();
    if (!selection.isEmpty())
        FLedSelectedServer->setText(selection.first()->text(0));
}

//!------------------------------
ServerComboBox::ServerComboBox(QWidget *AParent): QComboBox(AParent)
{
	connect(this, SIGNAL(currentIndexChanged(int)), SLOT(onCurrentIndexChanged(int)));
}

QString ServerComboBox::currentItemText() const
{
	return itemText(currentIndex());
}

void ServerComboBox::setCurrentItemText(const QString &AText)
{
	int found = findText(AText, Qt::MatchExactly);
	if (found!=-1)
		setCurrentIndex(found);
}

void ServerComboBox::onCurrentIndexChanged(int AIndex)
{
	emit currentItemTextChanged(itemText(AIndex));
}

//!------------------------------
CredentialsPage::CredentialsPage(IAccountManager *AAccountManager, QWidget *AParent):
    QWizardPage(AParent), FAccountManager(AAccountManager)
{
    QString style="style='color:blue;'";
	setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Credentials")));
	setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Please, enter your user name, password and resource")));

    QLabel *lblUser   = new QLabel(QString("<b>%1:</b>").arg(tr("User name and resource")));
	lblUser->setAlignment(Qt::AlignRight);
	FLedUsername = new QLineEdit;
	lblUser->setBuddy(FLedUsername);

	FLedResource = new QLineEdit;
    FLedResource->setText(CLIENT_NAME);
    FLedResource->selectAll();
	FLblServer = new QLabel;
	FCmbServer = new ServerComboBox();
	FCmbServer->setEditable(false);
    FLblServer->setBuddy(FLedResource);

	FLblPassword = new QLabel(QString("<b>%1:</b>").arg(tr("Password")));
	FLedPassword = new QLineEdit;
	FLedPassword->setEchoMode(QLineEdit::Password);//PasswordEchoOnEdit
	FLblPassword->setBuddy(FLedPassword);

	FLblPasswordRetype = new QLabel(QString("&<b>%1<b/>:").arg(tr("Re-type password")));
	FLedPasswordRetype = new QLineEdit;
	FLedPasswordRetype->setEchoMode(QLineEdit::Password);
	FLblPasswordRetype->setBuddy(FLedPasswordRetype);

	registerField(WF_SERVER_NAME, FCmbServer, "currentItemText", SIGNAL(currentItemTextChanged));
	registerField(WF_USER_RESOURCE, FLedResource);
	registerField(WF_USER_NAME, FLedUsername);
	registerField(WF_USER_PASSWORD, FLedPassword);
	registerField(WF_USER_PASSWORD_CONF, FLedPasswordRetype);

    QGridLayout *layout = new QGridLayout;
	layout->setHorizontalSpacing(0);
	layout->addWidget(lblUser, 0, 0);
	layout->addItem(new QSpacerItem(4, 0), 0, 1);
	layout->addWidget(FLedUsername, 0, 2);
	layout->addWidget(new QLabel("@"), 0, 3);
	layout->addWidget(FCmbServer, 0, 4);
	layout->addWidget(FLblServer, 0, 4);
	layout->addWidget(new QLabel("/"), 0, 5);
	layout->addWidget(FLedResource, 0, 6);
	layout->addWidget(FLblPassword, 1, 0);
	layout->addItem(new QSpacerItem(4, 0), 1, 1);
	layout->addWidget(FLedPassword, 1, 2);
	layout->addWidget(FLblPasswordRetype, 2, 0);
	layout->addItem(new QSpacerItem(4, 0), 2, 1);
	layout->addWidget(FLedPasswordRetype, 2, 2);
	setLayout(layout);

	connect(FLedPassword,SIGNAL(textChanged(QString)),SIGNAL(completeChanged()));
	connect(FLedPasswordRetype,SIGNAL(textChanged(QString)),SIGNAL(completeChanged()));
}

bool CredentialsPage::isComplete() const
{
	return !FLedUsername->text().isEmpty() && !FLedResource->text().isEmpty() && !FLedPassword->text().isEmpty() && (FLedPassword->text()==FLedPasswordRetype->text() || !wizard()->property("registerAccount").toBool());
}

void CredentialsPage::initializePage()
{
	FCmbServer->clear();
	if (field(WF_SERVER_NAME_PRE).toString().isEmpty())
	{
		FCmbServer->addItems(getServerList());
		FCmbServer->updateGeometry();
		FCmbServer->setVisible(true);
		FLblServer->setVisible(false);
	}
	else
	{		
		FCmbServer->addItem(field(WF_SERVER_NAME_PRE).toString());
		FLblServer->setText(field(WF_SERVER_NAME_PRE).toString());
		FLblServer->updateGeometry();
		FCmbServer->setVisible(false);
		FLblServer->setVisible(true);
	}

	if (wizard()->property("registerAccount").toBool())
	{
		FLblPasswordRetype->setVisible(true);
		FLedPasswordRetype->setVisible(true);
	}
	else
	{
		FLblPasswordRetype->setVisible(false);
		FLedPasswordRetype->setVisible(false);
	}	
}

bool CredentialsPage::validatePage()
{
	if (FAccountManager->findAccountByStream(Jid::fromUserInput(QString("%1@%2/%3").arg(FLedUsername->text()).arg(field(WF_SERVER_NAME).toString()).arg(FLedResource->text()))))
    {
        QMessageBox::critical(wizard(), tr("Account exists"), tr("Account with specified Server, User Name and Resource exists already! Please choose different Server, User Name or Resource."), QMessageBox::Ok);
        return false;
    }
	return true;
}

QStringList CredentialsPage::getServerList()
{
	QWizardPage::initializePage();

	switch (field(WF_NETWORK_SELECTED).toInt())
	{
		case NetworkPage::NetworkGoogle:
		{
			QStringList domains = QStringList() << "gmail.com" << "googlemail.com";
			return domains;
		}

		case NetworkPage::NetworkYandex:
		{
			QStringList domains = QStringList()
				<< "ya.ru" << "yandex.ru" << "yandex.net" << "yandex.com" << "yandex.by"
				<< "yandex.kz" << "yandex.ua" << "yandex-co.ru" << "narod.ru";
			return domains;
		}

		case NetworkPage::NetworkOdnoklassniki:
		{
			QStringList domains = QStringList()
				<< "odnoklassniki.ru";
			return domains;
		}

		case NetworkPage::NetworkLiveJournal:
		{
			QStringList domains = QStringList()
				<< "livejournal.com";
			return domains;
		}

	case NetworkPage::NetworkQIP:
		{
			QStringList domains = QStringList()
				<< "qip.ru" << "pochta.ru" << "fromru.com" << "front.ru" << "hotbox.ru"
				<< "hotmail.ru"	<< "krovatka.su" << "land.ru"	<< "mail15.com"	<< "mail333.com"
				<< "newmail.ru"	<< "nightmail.ru"	<< "nm.ru"	<< "pisem.net"	<< "pochtamt.ru"
				<< "pop3.ru"	<< "rbcmail.ru"	<< "smtp.ru"	<< "5ballov.ru"	<< "aeterna.ru"
				<< "ziza.ru"	<< "memori.ru"	<< "photofile.ru"	<< "fotoplenka.ru";
			return domains;
		}
	}
	return QStringList();
}


//!------------------------------
ConnectionPage::ConnectionPage(ServerPage *AServerPage, IOptionsDialogWidget *AConnectionSettingsWidget, QWidget *AParent):
	QWizardPage(AParent),
	FServerPage(AServerPage),
	FConnectionSettingsWidget(AConnectionSettingsWidget)
{
	QString style="style='color:blue;'";
	setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Connection settings")));
	setSubTitle(QString("<span %1>%2<br>%3</span>").arg(style)
													.arg(tr("Check your connection settings."))
													.arg(tr("If you're not certain about it, please, contact your system administartor.")));

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setMargin(0);
	if (FConnectionSettingsWidget)
		layout->addWidget(FConnectionSettingsWidget->instance());
	setLayout(layout);
}

bool ConnectionPage::validatePage()
{
	FConnectionSettingsWidget->apply();
	return QWizardPage::validatePage();
}

int ConnectionPage::nextId() const
{
	int flags = FServerPage->getFlags();
	return (wizard()->property("registerAccount").toBool() && (flags&ServerInfo::Valid) && !(flags&ServerInfo::InBandRegistration))
			?ConnectionWizard::Page_InfoWeb
			:ConnectionWizard::Page_Connect;
}

//!------------------------------
ConnectPage::ConnectPage(IConnectionEngine *AConnectionEngine, QWidget *parent):
	QWizardPage(parent),
	FRegistration(PluginHelper::pluginInstance<IRegistration>()),
	FDataForms(PluginHelper::pluginInstance<IDataForms>()),
	FConnectionEngine(AConnectionEngine),
	FXmppStream(NULL),
	FDfwRegisterForm(NULL)
{
    QString style="style='color:blue;'";
    setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Connect to Jabber server")));
    setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Trying to logon or register at Jabber server")));

	FStackedWidget = new QStackedWidget();
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(FStackedWidget);
	FStackedWidget->addWidget(new QWidget());

    FLblStatus=new QLabel();
    FLblStatus->setWordWrap(true);
    FLblError=new QLabel();
    FLblError->setWordWrap(true);
    FLblAdvice=new QLabel();
    FLblAdvice->setWordWrap(true);

    FProgressBar = new QProgressBar();
    FProgressBar->setMinimum(0);
    FProgressBar->setMaximum(100);
    FProgressBar->setValue(0);
    FProgressBar->setTextVisible(false);

	QVBoxLayout *firstLayout = new QVBoxLayout(FStackedWidget->widget(0));
	firstLayout->addWidget(FLblStatus);
	firstLayout->addWidget(FLblError);
	firstLayout->addWidget(FLblAdvice);
	firstLayout->addWidget(FProgressBar);
	firstLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Expanding));

	if (FRegistration)
	{
		connect(FRegistration->instance(),SIGNAL(registerFields(QString, IRegisterFields)),
			SLOT(onRegisterFields(QString, IRegisterFields)));
		connect(FRegistration->instance(),SIGNAL(registerError(QString, XmppError)),
			SLOT(onRegisterError(QString, XmppError)));
//		connect(FRegistration->instance(),SIGNAL(registerSuccess(QString)),
//			SLOT(onRegisterSuccess(QString)));
	}
}

ConnectPage::~ConnectPage()
{
	if (FXmppStream)
		FXmppStream->close();
}

void ConnectPage::cleanupPage()
{
	if (FDfwRegisterForm)
	{
		FStackedWidget->removeWidget(FDfwRegisterForm->instance());
		FDfwRegisterForm->instance()->deleteLater();
		delete layout();
	}
	if (FXmppStream)
		FXmppStream->close();
}

void ConnectPage::initializePage()
{
	FLblStatus->setVisible(true);
	FXmppStream = createXmppStream();

	if (FXmppStream)
	{
		QString style="style='color:blue;'";
		int flags = qobject_cast<ServerPage *>(wizard()->page(ConnectionWizard::Page_Server))->getFlags();

		FLblStatus->setText(QString("<span %1>%2</span>")
							.arg(style)
							.arg(wizard()->property("registerAccount").toBool() && (flags&ServerInfo::InBandRegistration || !(flags&ServerInfo::Valid))?
									 tr("Requesting registration form..."):tr("Connecting...")));
		FProgressBar->setMaximum(0);
		FProgressBar->setPalette(QPalette());   // Reset palette

		FLblError->clear();
		FLblAdvice->clear();

		if (FRegistration &&
			wizard()->property("registerAccount").toBool() &&
			(flags&ServerInfo::InBandRegistration || !(flags&ServerInfo::Valid)))
			FRegisterId = FRegistration->startStreamRegistration(FXmppStream);
		else
			FXmppStream->setPassword(field(WF_USER_PASSWORD).toString());
	}

	if (FXmppStream==NULL || (FRegisterId.isEmpty() && !FXmppStream->open()))
	{
		FLblStatus->setText(QString("<h2>%1</h2>").arg(tr("Failed to check connection :(")));
		FLblError->setText(tr("Internal Error"));
		FLblAdvice->setText(tr("Click 'Back' button to change the account credentials or the 'Finish' button to add the account as is."));

		FLblError->setVisible(true);
		FLblAdvice->setVisible(true);
		FProgressBar->setVisible(false);
	}
	else
	{
		FLblError->setVisible(false);
		FLblAdvice->setVisible(false);
		FProgressBar->setVisible(true);
	}

	emit completeChanged();
}

bool ConnectPage::isComplete() const
{
	if (FDfwRegisterForm && FDataForms->isSubmitValid(FDfwRegisterForm->dataForm(), FDfwRegisterForm->userDataForm()))
	{
		IDataForm form = FDfwRegisterForm ->userDataForm();
		return FDataForms->fieldValue("password", form.fields).toString() == FDataForms->fieldValue("password-confirm", form.fields).toString();
	}
	else
		return false;
}

bool ConnectPage::validatePage()
{
	if (FDfwRegisterForm)
	{
		FRegisterSubmit.key = FRegisterFields.key;
		FRegisterSubmit.serviceJid = FRegisterFields.serviceJid;
		if (FRegisterFields.fieldMask & IRegisterFields::Form)
		{
			FRegisterSubmit.form = FDataForms->dataSubmit(FDfwRegisterForm ->userDataForm());
			FRegisterSubmit.fieldMask = IRegisterFields::Form;
			for (QList<IDataField>::Iterator it = FRegisterSubmit.form.fields.begin(); it!=FRegisterSubmit.form.fields.end();)
			{
				if ((*it).var == "username")
					setField(WF_USER_NAME, (*it).value);
				else if ((*it).var == "password")
					setField(WF_USER_PASSWORD, (*it).value);
				if ((*it).var=="password-confirm")				// Ignore "password-confirm" field,
					it = FRegisterSubmit.form.fields.erase(it);	// 'cause we added it ourself
				else
					it++;
			}
		}
		else
		{
			IDataForm form = FDfwRegisterForm ->userDataForm();
			FRegisterSubmit.username = FDataForms->fieldValue("username",form.fields).toString();
			FRegisterSubmit.password = FDataForms->fieldValue("password",form.fields).toString();
			FRegisterSubmit.email = FDataForms->fieldValue("email",form.fields).toString();
			FRegisterSubmit.fieldMask = FRegisterFields.fieldMask;
			setField(WF_USER_NAME, FRegisterSubmit.username);
			setField(WF_USER_PASSWORD, FRegisterSubmit.password);
		}
		return FRegistration->submitStreamRegistration(FXmppStream, FRegisterSubmit) == FRegisterId;
	}
	return true;
}

int ConnectPage::nextId() const
{
	return FDfwRegisterForm?ConnectionWizard::Page_Submit:ConnectionWizard::Page_Conclusion;
}

void ConnectPage::onXmppStreamError(const XmppError &AError)
{
    QString style="color:red;";
    FLblStatus->setStyleSheet(style);
    FLblStatus->setText(tr("Connection failed!"));
	FLblError->setText(QString("<b>%1:</b> %2").arg(tr("Error")).arg(AError.errorText()));
	if (AError.condition()== "not-authorized")
        FLblAdvice->setText(tr("Please go back and check your credentials."));
	else if (AError.condition() == "connectionmanager-connect-error")
		FLblAdvice->setText(tr("Please go back and check your connection and server name."));
	else if (AError.condition() == "forbidden")
        FLblAdvice->setText(tr("Please make sure the server supports in-band registration."));

    QPalette palette;
    palette.setColor(QPalette::Highlight, QColor(Qt::red));
    FProgressBar->setPalette(palette);
    FProgressBar->setMaximum(100);
    FProgressBar->setValue(100);

    wizard()->button(QWizard::NextButton)->setDisabled(true);
}

void ConnectPage::onXmppStreamOpened()
{
    FProgressBar->setMaximum(100);
    FProgressBar->setValue(100);
	FXmppStream->close();	
	wizard()->next();
}

void ConnectPage::onXmppStreamDestroyed(QObject *AXmppStream)
{
	Q_UNUSED(AXmppStream)
	FXmppStream=NULL;
}

void ConnectPage::onRegisterFields(const QString &AId, const IRegisterFields &AFields)
{
	if ((wizard()->currentId() == ConnectionWizard::Page_Connect) && (FRegisterId == AId))
	{
		FRegisterFields = AFields;
		FRegisterSubmit.key = FRegisterFields.key;
		FRegisterSubmit.serviceJid = FRegisterFields.serviceJid;

		if ((AFields.fieldMask & IRegisterFields::Form) == 0)
		{
			FRegisterFields.form.type = DATAFORM_TYPE_FORM;
			FRegisterFields.form.instructions.append(AFields.instructions);
			if (AFields.fieldMask & IRegisterFields::Username)
			{
				IDataField dataField;
				dataField.var = "username";
				dataField.type = DATAFIELD_TYPE_TEXTSINGLE;
				dataField.value = field(WF_USER_NAME).toString();
				FRegisterFields.form.fields.append(dataField);
			}
			if (AFields.fieldMask & IRegisterFields::Password)
			{
				IDataField dataField;
				dataField.var = "password";
				dataField.type = DATAFIELD_TYPE_TEXTPRIVATE;
				dataField.label = tr("Password");
				FRegisterFields.form.fields.append(dataField);
				dataField.var = "password-confirm";
				dataField.label = tr("Retype password");
				FRegisterFields.form.fields.append(dataField);
			}
			if (AFields.fieldMask & IRegisterFields::Email)
			{
				IDataField dataField;
				dataField.var = "email";
				dataField.type = DATAFIELD_TYPE_TEXTSINGLE;
				dataField.label = tr("e-mail");
				dataField.required = true;
				dataField.value = FRegisterSubmit.email;
				FRegisterFields.form.fields.append(dataField);
			}
		}
		else
		{
			for (QList<IDataField>::Iterator it = FRegisterFields.form.fields.begin(); it != FRegisterFields.form.fields.end(); it++)
			{
				// Hide unneeded fields
				if ((((*it).type == DATAFIELD_TYPE_TEXTSINGLE) &&
					 ((*it).var == "phone" || (*it).var == "url")) ||
					(*it).type == DATAFIELD_TYPE_FIXED)
					(*it).type = DATAFIELD_TYPE_HIDDEN;

				if ((*it).type == DATAFIELD_TYPE_TEXTSINGLE)
				{
					if ((*it).var == "username")
						(*it).label = tr("User name");
					else if ((*it).var == "ocr")
						(*it).label = tr("Enter the text you see");
					else if ((*it).var == "email")
						(*it).label = tr("e-mail");
				}

				// Duplicate password
				if (((*it).type == DATAFIELD_TYPE_TEXTPRIVATE) &&
					((*it).var == "password"))
				{
					(*it).label = tr("Password");
					IDataField dataField = *it;
					dataField.var = "password-confirm";
					dataField.label = tr("Retype password");
					it = FRegisterFields.form.fields.insert(it+1, dataField);
				}

				QVariant value;
				if (((*it).type != DATAFIELD_TYPE_HIDDEN) &&	// Ignore old values for passwords
					((*it).var != "ocr"))						// and CAPTCHAs
					value = FDataForms->fieldValue((*it).var, FRegisterSubmit.form.fields);

				if (!value.isNull())
					(*it).value = value;
			}
		}

		FDfwRegisterForm = FDataForms->formWidget(FRegisterFields.form,this);
		FStackedWidget->setCurrentIndex(FStackedWidget->addWidget(FDfwRegisterForm->instance()));
		connect(FDfwRegisterForm->instance(),SIGNAL(fieldChanged(IDataFieldWidget *)),SIGNAL(completeChanged()));
		connect(FDfwRegisterForm->instance(),SIGNAL(destroyed(QObject*)),SLOT(onFormDeleted()));

		emit completeChanged();
	}
}

void ConnectPage::onRegisterError(const QString &AId, const XmppError &AError)
{
	if ((wizard()->currentId() == ConnectionWizard::Page_Connect) && (FRegisterId == AId))
	{
		FLblStatus->setText(QString("<h2>%1</h2>").arg(tr("Failed to register :(")));
		FLblError->setText(AError.errorMessage());

		if (FDfwRegisterForm != NULL)
		{
			FStackedWidget->removeWidget(FDfwRegisterForm->instance());
			FDfwRegisterForm->instance()->deleteLater();
		}

		FLblStatus->setVisible(true);
		FLblError->setVisible(true);
		FProgressBar->setVisible(false);
		FLblAdvice->setVisible(false);
		emit completeChanged();
	}
}

void ConnectPage::onFormDeleted()
{
	FDfwRegisterForm = NULL;
}

IXmppStream *ConnectPage::createXmppStream() const
{
	IXmppStreamManager *xmppStreamManager = PluginHelper::pluginInstance<IXmppStreamManager>();
	IConnectionManager *connManager = PluginHelper::pluginInstance<IConnectionManager>();
	if (xmppStreamManager!=NULL && connManager!=NULL && FConnectionEngine!=NULL)
	{
		IXmppStream *xmppStream;
		if (wizard()->property("registerAccount").toBool())
			xmppStream = xmppStreamManager->createXmppStream(field(WF_SERVER_NAME_PRE).toString());
		else
			xmppStream = xmppStreamManager->createXmppStream(Jid(field(WF_USER_NAME).toString(), field(WF_SERVER_NAME).toString(), "Wizard"));
		xmppStream->setEncryptionRequired(true);
		connect(xmppStream->instance(),SIGNAL(opened()),SLOT(onXmppStreamOpened()));
		connect(xmppStream->instance(),SIGNAL(error(const XmppError &)),SLOT(onXmppStreamError(const XmppError &)));
		connect(xmppStream->instance(),SIGNAL(closed()),xmppStream->instance(),SLOT(deleteLater()));
		connect(xmppStream->instance(),SIGNAL(destroyed(QObject*)),SLOT(onXmppStreamDestroyed(QObject*)));

		IConnection *conn = FConnectionEngine->newConnection(ACCOUNT_CONNECTION_OPTIONS,xmppStream->instance());
		xmppStream->setConnection(conn);
		return xmppStream;
	}
	return NULL;
}

RegisterSubmitPage::RegisterSubmitPage(ConnectPage *AConnectPage, QWidget *parent):
	QWizardPage(parent),
	FRegistration(PluginHelper::pluginInstance<IRegistration>()),
	FDataForms(PluginHelper::pluginInstance<IDataForms>()),
	FDfwRegisterForm(NULL),
	FConnectPage(AConnectPage)
{
	QString style="style='color:blue;'";
	setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Connect to Jabber server")));
	setSubTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Trying to logon or register at Jabber server")));

	FLblStatus=new QLabel();
	FLblStatus->setWordWrap(true);
	FLblError=new QLabel();
	FLblError->setWordWrap(true);
	FLblAdvice=new QLabel();
	FLblAdvice->setWordWrap(true);

	FProgressBar = new QProgressBar();
	FProgressBar->setMinimum(0);
	FProgressBar->setMaximum(100);
	FProgressBar->setValue(0);
	FProgressBar->setTextVisible(false);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(FLblStatus);
	layout->addWidget(FLblError);
	layout->addWidget(FLblAdvice);
	layout->addWidget(FProgressBar);

	setLayout(layout);

	if (FRegistration)
	{
		connect(FRegistration->instance(),SIGNAL(registerError(QString, XmppError)),
			SLOT(onRegisterError(QString, XmppError)));
		connect(FRegistration->instance(),SIGNAL(registerSuccess(QString)),
			SLOT(onRegisterSuccess(QString)));
	}
}

void RegisterSubmitPage::initializePage()
{
	QString style="style='color:blue;'";

	FLblStatus->setText(QString("<span %1>%2</span>")
						.arg(style)
						.arg(tr("Registering...")));
	FProgressBar->setMaximum(0);
	FProgressBar->setPalette(QPalette());   // Reset palette

	FLblError->clear();
	FLblAdvice->clear();
}

void RegisterSubmitPage::onRegisterError(const QString &AId, const XmppError &AError)
{
	if ((wizard()->currentId() == ConnectionWizard::Page_Submit) && (FConnectPage->registerId() == AId))
	{
		FLblStatus->setText(QString("<h2>%1</h2>").arg(tr("Failed to register :(")));
		FLblError->setText(AError.errorMessage());

		if (FDfwRegisterForm != NULL)
		{
			FDfwRegisterForm->instance()->deleteLater();
			FDfwRegisterForm = NULL;
		}

		FLblStatus->setVisible(true);
		FLblError->setVisible(true);
		FProgressBar->setVisible(false);
		FLblAdvice->setVisible(false);
		emit completeChanged();
	}
}

void RegisterSubmitPage::onRegisterSuccess(const QString &AId)
{
	if ((wizard()->currentId() == ConnectionWizard::Page_Submit) && (FConnectPage->registerId() == AId))
		wizard()->next();
}

//!------------------------------
WebRegistrationInfo::WebRegistrationInfo(ServerPage *AServerPage, QWidget *AParent):
    QWizardPage(AParent),
    FIconStorage(NULL),
    FServerPage(AServerPage)
{
    QString style="style='color:blue;'";
	setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Web registration")));
	setSubTitle(QString("<span %1>%2<br>%3</span>")
				.arg(style)
                .arg(tr("The server you selected doesn't support in-band registration."))
				.arg(tr("Follow instructions below to register via web.")));

	FIconStorage	= IconStorage::staticStorage(RSR_STORAGE_WIZARDS);

	QVBoxLayout *layout = new QVBoxLayout;

	QFont font;
	font.setStyleHint(QFont::Serif);
	font.setStyleStrategy(QFont::PreferAntialias);
    font.setStretch(200);
	FLblHeader=new QLabel;
	FLblHeader->setAlignment(Qt::AlignCenter);
	FLblHeader->setStyleSheet("color:green;");
	FLblHeader->setFont(font);
	FLblHeader->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	FLblHeader->setWordWrap(true);
	layout->addWidget(FLblHeader);

	FLblInstructions=new QLabel;
	FLblInstructions->setWordWrap(true);
	FLblInstructions->setFrameShape(QFrame::Panel);
	FLblInstructions->setFrameShadow(QFrame::Raised);
	FLblInstructions->setBackgroundRole(QPalette::Window);
	FLblInstructions->setMargin(10);
	layout->addWidget(FLblInstructions);

	FClbOpenLink = new QCommandLinkButton(tr("Open registration website"));
	FClbOpenLink->setIcon(IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_LINK));
	connect(FClbOpenLink, SIGNAL(clicked()), SLOT(onOpenLinkClicked()));
	layout->addWidget(FClbOpenLink);

	setLayout(layout);
}

void WebRegistrationInfo::initializePage()
{
	FComplete = false;
	FLblHeader->setText(tr("How to register at %1").arg(field(WF_SERVER_NAME).toString()));

	FRegisterUrl	= FServerPage->getRegistrationUrl();
	FClbOpenLink->setDescription(FRegisterUrl.toString());

	loadInstructions(FServerPage->getInstructions());
}

void WebRegistrationInfo::cleanupPage()
{
	FLblInstructions->clear();
	FLblHeader->clear();
}

void WebRegistrationInfo::loadInstructions(const QString &AInstructions)
{
	if(!AInstructions.isEmpty())
    {		
        QString styleRed  = "style='color:red;'";
        QString style     = "style='color:blue;'";
        QString fileName  = QString("%1.%2.html").arg(AInstructions);
        QDir dir(FIconStorage->resourcesDirs().first());
        dir.cd(FIconStorage->storage());
        dir.cd(FIconStorage->subStorage());

        QFile file(dir.absoluteFilePath(fileName.arg(QLocale().name().left(2))));
		if(!file.open(QFile::ReadOnly))
		{
			file.setFileName(dir.absoluteFilePath(fileName.arg("en")));
			file.open(QFile::ReadOnly);
		}
        FLblInstructions->setText(file.isOpen()?QString::fromUtf8(file.readAll())
                                               :QString("<span %1><b>%2</b></span><br><span %3>%4</span>")
                                                .arg(styleRed).arg(tr("Warning!"))
                                                .arg(style).arg(tr("Cannot open instructions.")));
    }
}

void WebRegistrationInfo::onOpenLinkClicked()
{
	if (QDesktopServices::openUrl(FRegisterUrl))
	{
		FComplete = true;
		emit completeChanged();
	}
}

//!------------------------------

ConclusionPage::ConclusionPage(IAccountManager *AAccountManager, IConnectionEngine *AConnectionEngine, IOptionsDialogWidget *AConnectionSettingsWidget, QWidget *AParent):
	QWizardPage(AParent),
	FAccountManager(AAccountManager),
	FAccount(NULL),
	FConnectionEngine(AConnectionEngine),
	FConnectionSettingsWidget(AConnectionSettingsWidget)
{
    setPixmap(QWizard::WatermarkPixmap, IconStorage::staticStorage(RSR_STORAGE_WIZARDS)->getIcon(MNI_WIZARD_ACC2).pixmap(2048));

	QGridLayout *layout = new QGridLayout;
	FLblTitle = new QLabel();
	layout->addWidget(FLblTitle, 0, 0, 1, 2);
	layout->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed), 1, 0, 1, 2);
    FLblText1 = new QLabel();
    FLblText1->setWordWrap(true);
	layout->addWidget(FLblText1, 3, 0, 1, 2);

	FLblText2 = new QLabel(tr("Welcome to Jabber network!"));
	FLblText2->setWordWrap(true);
	layout->addWidget(FLblText2, 4, 0, 1, 2);

	FLblAccountName = new QLabel(tr("Account name"));
	layout->addWidget(FLblAccountName, 5, 0, 1, 1);
	FLedAccountName = new QLineEdit();
	layout->addWidget(FLedAccountName, 5, 1, 1, 1);

    FLblText3 = new QLabel(tr("Press \"Finish\" button to close Wizard.", "\"Finish\" should match the text of an appropriate Qt Wizard button"));
    FLblText3->setWordWrap(true);
	layout->addWidget(FLblText3, 6, 0, 1, 2);

	FLbAccountSettingsLink = new QLabel(QString("<a href=\"open settings\">%1</a>").arg(tr("Additional account settings...")));
	connect(FLbAccountSettingsLink, SIGNAL(linkActivated(QString)), SLOT(onAccountSettingsLinkActivated(QString)));
	layout->addWidget(FLbAccountSettingsLink, 7, 0, 1, 2);

	FChbGoOnline = new QCheckBox("Go online now");
	layout->addWidget(FChbGoOnline, 8, 0, 1, 2);
	registerField(WF_GO_ONLINE, FChbGoOnline);

	setLayout(layout);
	connect(AParent, SIGNAL(accepted()), SLOT(onWizardAccepted()));
}

void ConclusionPage::initializePage()
{
	if (createAccount())
	{
		QString style="style='color:blue;'";
		setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Done!")));
		setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Connection Wizard cempleted successfuly.")));

		FLblTitle->setText(QString("<h1 align='center' style='color: green;'>%1</h1>").arg(tr("Congratulations")));

		FLblText1->setText((wizard()->property("registerAccount").toBool()?
			tr("You successfully connected to Jabber as %1."):
			tr("You successfully registered at Jabber as %1.")).arg(QString("<font color=blue><b>%1@%2</b></font>").arg(field(WF_USER_NAME).toString()).arg(field(WF_SERVER_NAME).toString())));

		FLblText2->setVisible(true);
		FLblText3->setVisible(true);

		FLedAccountName->setText(wizard()->property("streamJid").toString());

		FLbAccountSettingsLink->setVisible(true);
		FChbGoOnline->setVisible(true);

		FChbGoOnline->setVisible(true);
		FChbGoOnline->setChecked(true);
	}
	else
	{
		QString style="style='color:red;'";
		setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Failure!")));
		setTitle(QString("<span %1>%2</span>").arg(style).arg(tr("Connection Wizard failed to create an account for you")));
		FLblTitle->setText(QString("<h1 align='center' style='color: red;'>%1</h1>").arg(tr("Sorry")));
		FLblText1->setText(tr("Internal error occured!"));
		FLblText2->setVisible(false);
		FLblText3->setVisible(false);
		FLbAccountSettingsLink->setVisible(false);
		FChbGoOnline->setVisible(false);
	}
}

void ConclusionPage::onAccountSettingsLinkActivated(const QString &ALink)
{
	Q_UNUSED(ALink)
	IOptionsManager *optionsManager = PluginHelper::pluginInstance<IOptionsManager>();
	if (optionsManager != NULL)
	{
		QString rootId = OPN_ACCOUNTS"."+FAccount->accountId().toString();
		optionsManager->showOptionsDialog(QString(OPN_ACCOUNTS_ADDITIONAL).replace("[id]", FAccount->accountId().toString()), rootId, parentWidget());
	}
}

void ConclusionPage::onWizardAccepted()
{
	FAccount->setName(FLedAccountName->text());
	if (field(WF_GO_ONLINE).toBool())
		PluginHelper::pluginInstance<IStatusChanger>()->setStreamStatus(Jid(field(WF_USER_NAME).toString(), field(WF_SERVER_NAME).toString(), field(WF_USER_RESOURCE).toString()), STATUS_ONLINE);
}

bool ConclusionPage::createAccount()
{
	QString streamJid = wizard()->property("streamJid").toString();
	FAccount = FAccountManager->createAccount(streamJid, streamJid);
	if (FAccount)
	{
		FAccount->setPassword(field(WF_USER_PASSWORD).toString());
		FAccount->setActive(true);
		FAccount->optionsNode().node(OPV_ACCOUNT_AUTOCONNECT).setValue(true);

		if (FConnectionEngine)
			FConnectionEngine->saveConnectionSettings(FConnectionSettingsWidget, FAccount->optionsNode().node("connection",FConnectionEngine->engineId()));
		return true;
	}
	return false;
}
