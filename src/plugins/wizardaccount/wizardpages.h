#ifndef WIZARDPAGES_H
#define WIZARDPAGES_H

#include <QWizard>
#include <QComboBox>
#include <QMetaType>
#include <QTextBrowser>
#include <QLabel>
#include <QListWidget>
#include <QTreeView>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QCommandLinkButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <SelectableTreeWidget>
#include <QScrollArea>

#include <interfaces/ixmppstreammanager.h>
#include <interfaces/iconnectionmanager.h>
#include <interfaces/ioptionsmanager.h>
#include <interfaces/iaccountmanager.h>
#include <interfaces/istatuschanger.h>
#include <interfaces/iregistraton.h>

#include <utils/jid.h>
#include <utils/iconstorage.h>

//!---------- Connection Wizard -----------
class ConnectionWizard : public QWizard
{
    Q_OBJECT
    Q_PROPERTY(bool registerAccount READ registerAccount WRITE setRegisterAccount)
	Q_PROPERTY(QString serverName READ serverName WRITE setServerName)
	Q_PROPERTY(QString streamJid READ streamJid)

public:
	enum {Page_Intro, Page_Network, Page_Server, Page_InfoWeb, Page_Credentials, Page_Connection, Page_Connect, Page_Submit, Page_Conclusion};
    enum {No, Yes, Unknown};

	ConnectionWizard(QWidget *AParent = 0);
	~ConnectionWizard();

    bool registerAccount() const {return FRegisterAccount;}
    void setRegisterAccount(int ARegisterAccount) {FRegisterAccount = ARegisterAccount;}

	QString serverName() const;
	void setServerName(const QString &AServerName);
	QString streamJid() const;

protected:
    void init();

protected slots:
	void onCurrentIdChanged(int AId);

private:
	IConnectionManager	*FConnectionManager;
	IOptionsDialogWidget *FConnectionSettingsWidget;
	int					FCurrentId;
	bool				FRegisterAccount;
	QString				FServerName;
};

//!---------- Intro Page -----------
class IntroPage : public QWizardPage
{
    Q_OBJECT
public:
    IntroPage(QWidget *parent = 0);
	// QWizardPage interface
	int nextId() const {return FNextId;}
    bool isComplete() const {return false;}

protected slots:
    void onClicked();

private:
	QLabel				*FLblTop;
    QCommandLinkButton	*FClbHaveAccountYes;
    QCommandLinkButton	*FClbHaveAccountNo;	
	int					FNextId;
};

//!---------- Network Page -----------
class NetworkPage : public QWizardPage
{
	Q_OBJECT
public:
	enum NetworkType {
		NetworkOther,
		NetworkGoogle,
		NetworkOdnoklassniki,
		NetworkLiveJournal,
		NetworkQIP,
		Network_Count
	};

	struct NetworkInfo
	{
		NetworkInfo(){}
		NetworkInfo(QString AName, QString AIcon, QString ADescription, QString AUrl=QString::null, QStringList AServers=QStringList()): name(AName), icon(AIcon), description(ADescription), url(AUrl), servers(AServers) {}
		QString name; QString icon; QString description; QUrl url; QStringList servers;
	};

	enum DataRole {
		NetworkDescription=Qt::UserRole
	};

	NetworkPage(QWidget *AParent = 0);

	NetworkType serverNetwork(const QString &AServer) const;
	NetworkInfo networkInfo(NetworkType ANetworkType) const;

	// QWizardPage interface
	void initializePage();
	int nextId() const {return FNextId;}

protected slots:
	void onCurrentItemChanged(QListWidgetItem *ACurrent, QListWidgetItem *APrevious);

private:
	QLabel		*FLblTop;
	QLabel		*FLblDescription;
	QListWidget	*FLwNetworkList;
	int			FNextId;
	QMap<NetworkType, NetworkInfo> FNetworks;
};

//!---------- Server Page -----------
struct ServerInfo
{
	enum ServerFlags {
		LegacySSL			= 0x01,
		StreamCompress		= 0x02,
        InBandRegistration	= 0x04,
        Pep                 = 0x08,
		Valid				= 0x80
	};

	ServerInfo(bool valid=false): flags(valid?Valid:0) {}
	QUrl    url;
	QString instructions;
	int     flags;
};
Q_DECLARE_METATYPE(ServerInfo)

class ServerPage : public QWizardPage
{
    Q_OBJECT

public:
	ServerPage(NetworkPage *ANtworkPage, QWidget *AParent = 0);
	QUrl	getRegistrationUrl() const;
	QString	getInstructions(const QString &AServerName) const;
	int		getFlags() const;

public:
	//QWizardPage
    void initializePage();
    bool validatePage();
	int nextId() const;

private slots:
    void onSelectionChanged(const QItemSelection &ASelected, const QItemSelection &ADeselected);

protected:
    void accept();
    void loadServerList();

protected slots:
	void onButtonClicked(QAbstractButton *AButton);

private:
	QTreeView *FServerList;
	QLineEdit   *FLedSelectedServer;
	QHash<QString, ServerInfo> FServerInfo;
	NetworkPage	*FNetworkPage;
};

class ServerComboBox: public QComboBox
{
	Q_OBJECT
	Q_PROPERTY(QString currentItemText READ currentItemText WRITE setCurrentItemText NOTIFY currentItemTextChanged)
public:
	ServerComboBox(QWidget *AParent = NULL);
	QString currentItemText() const;
	void setCurrentItemText(const QString &AText);
protected slots:
	void onCurrentIndexChanged(int AIndex);
signals:
	void currentItemTextChanged(const QString &AText);
};

//!------ Web Registration Info Page -------
class WebRegistrationInfo : public QWizardPage
{
	Q_OBJECT
public:
	WebRegistrationInfo(ServerPage *AServerPage, QWidget *AParent = 0);

	// QWizardPage interface
	void initializePage();
	void cleanupPage();
	bool isComplete() const {return FComplete;}

protected:
	void loadInstructions(const QString &AInstructions, const QString &AServerName);

protected slots:
	void onOpenLinkClicked();

private:
	QLabel		*FLblHeader;
	QTextBrowser	*FLblInstructions;
	QCommandLinkButton *FClbOpenLink;
	IconStorage *FIconStorage;
	ServerPage	*FServerPage;
	QUrl		FRegisterUrl;
	bool		FComplete;
};


//!---------- Credentials Page -----------
class CredentialsPage : public QWizardPage
{
    Q_OBJECT
public:
    CredentialsPage(IAccountManager *AAccountManager, QWidget *AParent = 0);

    // QWizardPage interface
    bool isComplete() const;
    void initializePage();
    bool validatePage();

protected:
	QStringList getServerList();

private:
	QLabel		*FLblServer;
	ServerComboBox	*FCmbServer;
	QLabel		*FLblPassword;
	QLabel		*FLblPasswordRetype;
	QLineEdit   *FLedUsername;
	QLineEdit   *FLedResource;
	QLineEdit   *FLedPassword;
	QLineEdit   *FLedPasswordRetype;
    IAccountManager *FAccountManager;
};

class ConnectionPage : public QWizardPage
{
	Q_OBJECT
public:
	ConnectionPage(ServerPage *AServerPage, IOptionsDialogWidget *AConnectionSettingsWidget, QWidget *AParent = 0);

	// QWizardPage interface
	bool validatePage();

private:
	ServerPage  *FServerPage;
	IOptionsDialogWidget *FConnectionSettingsWidget;
};

//!--------- Connect Page ------------
class ConnectPage : public QWizardPage
{
    Q_OBJECT
public:
	ConnectPage(IConnectionEngine *AConnectionEngine, QWidget *parent = 0);
	~ConnectPage();
	const QString &registerId() const {return FRegisterId;}

    // QWizardPage interface
    void initializePage();
	bool isComplete() const;
	void cleanupPage();
	bool validatePage();
	int nextId() const;

protected:
	IXmppStream *createXmppStream(bool ARegister) const;

protected slots:
	void onXmppStreamError(const XmppError &AError);
	void onXmppStreamOpened();
	void onXmppStreamDestroyed(QObject *AXmppStream);
	void onRegisterFields(const QString &AId, const IRegisterFields &AFields);
	void onRegisterError(const QString &AId, const XmppError &AError);
	void onFormDeleted();

private:
	QLabel			*FLblStatus;
	QLabel			*FLblError;
	QLabel			*FLblAdvice;
    QProgressBar    *FProgressBar;
	QStackedWidget	*FStackedWidget;
	IRegistration	*FRegistration;
	IDataForms		*FDataForms;
	IConnectionEngine	*FConnectionEngine;
	IXmppStream		*FXmppStream;
	IRegisterFields FRegisterFields;
	IRegisterSubmit FRegisterSubmit;
	IDataFormWidget *FDfwRegisterForm;
	QString			FRegisterId;
};

//!------- Register Submit Page --------------
class RegisterSubmitPage : public QWizardPage
{
	Q_OBJECT
public:
	RegisterSubmitPage(ConnectPage *AConnectPage, QWidget *parent = 0);
	// QWizardPage interface
	void initializePage();
	bool isComplete() const {return false;}
	int nextId() const {return ConnectionWizard::Page_Conclusion;}

protected slots:
	void onRegisterError(const QString &AId, const XmppError &AError);
	void onRegisterSuccess(const QString &AId);

private:
	QLabel			*FLblStatus;
	QLabel			*FLblError;
	QLabel			*FLblAdvice;
	QProgressBar    *FProgressBar;
	IRegistration	*FRegistration;
	IDataForms		*FDataForms;
	IRegisterFields FRegisterFields;
	IRegisterSubmit FRegisterSubmit;
	IDataFormWidget *FDfwRegisterForm;
	ConnectPage		*FConnectPage;
};

//!---------- Conclusion Page -----------

class ConclusionPage : public QWizardPage
{
    Q_OBJECT
public:
	ConclusionPage(IAccountManager *AAccountManager, IConnectionEngine	*AConnectionEngine, IOptionsDialogWidget *AConnectionSettingsWidget, QWidget *AParent = 0);
    // QWizardPage interface
    void initializePage();

protected:
	bool createAccount();

protected slots:
	void onAccountSettingsLinkActivated(const QString &ALink);
	void onWizardAccepted();

private:
	IAccountManager		*FAccountManager;
	IAccount	*FAccount;
	IConnectionEngine	*FConnectionEngine;
	IOptionsDialogWidget *FConnectionSettingsWidget;
	QLabel		*FLblTitle;
	QLabel		*FLblText1;
	QLabel		*FLblText2;
	QLabel		*FLblText3;
	QLabel		*FLbAccountSettingsLink;

	QLabel		*FLblAccountName;
	QLineEdit	*FLedAccountName;

	QCheckBox	*FChbGoOnline;
};

#endif // WIZARDPAGES_H
