#ifndef WIZARDPAGES_H
#define WIZARDPAGES_H

#include <QWizard>
#include <QComboBox>
#include <QMetaType>
#include <QCheckBox>
#include <QTreeWidget>
#include <QLabel>
#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QCommandLinkButton>
#include <QListWidget>
#include <SelectableTreeWidget>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QSpacerItem>
#include <QStyle>

#include <interfaces/iregistraton.h>
#include <interfaces/iservicediscovery.h>
#include <utils/iconstorage.h>

class TransportWizard : public QWizard
{
	Q_OBJECT
public:
	enum {Page_Intro,Page_Transports,Page_Networks,Page_Gateway,Page_Process,Page_Result,Page_Conclusion};
	TransportWizard(const Jid &AStreamJid, const Jid &ATransportJid = Jid::null, QWidget *parent = 0);
	void setAutoSubscribe(bool AAutoSubscribe) {FAutoSubscribe = AAutoSubscribe;}

protected slots:
	void onFinished(int AStatus);

private:
	Jid             FStreamJid;
	bool			FAutoSubscribe;
};

//!---------------------
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
	QCommandLinkButton	*FClbConnectLegacyNetwork;
	QCommandLinkButton	*FClbChangeTransport;
	int					FNextId;
};

//!---------------------
class TransportsPage : public QWizardPage
{
	Q_OBJECT
public:
	TransportsPage(const Jid &AStreamJid, const IServiceDiscovery *AServiceDiscovery, QWidget *parent = 0);
	// QWizardPage interface
	int nextId() const;
	void initializePage();

protected slots:
	void onTransportSelected(const QString &ATransportJid);

private:
	const IServiceDiscovery *FServiceDiscovery;
	const Jid				FStreamJid;
	SelectableTreeWidget	*FTransportsList;
	IconStorage				*FIconStorage;
};

//!---------------------
class NetworksPage : public QWizardPage
{
	Q_OBJECT
public:
	NetworksPage(QWidget *parent = 0);
	QString networkName(const QString &ANetworkId) const {return FNetworkNames.value(ANetworkId);}
	// QWizardPage interface
	int nextId() const;

protected:
	void loadNetworksList();

private:
	SelectableTreeWidget	*FNetworksList;
	IconStorage				*FIconStorage;
	QHash<QString, QString>	FNetworkNames;
	QHash<QString,QString>	FNetworkDescriptions;
};

//!---------------------
class GatewayPage : public QWizardPage
{
	Q_OBJECT
public:
	GatewayPage(const Jid &AStreamJid, IServiceDiscovery *AServiceDiscovery, QWidget *parent = 0);
	QList <QDomElement> getExcepFields(){return FExcepFields; }

	// QWizardPage interface
	void initializePage();
	bool validatePage();
	int nextId() const {return TransportWizard::Page_Process;}

protected:
	enum TransportStatus
	{
		Unavailable,
		Available,
		Unknown
	};

	void setItemStatus(QTreeWidgetItem *AItem, TransportStatus AStatus);
	void loadTransportList();
	void appendLocalTransport(const IDiscoInfo &ADiscoInfo); //TODO: Get rid of it

protected slots:
	void onDiscoInfoReceived(const IDiscoInfo &ADiscoInfo);
	void onDiscoItemsReceived(const IDiscoItems &ADiscoItems);

private:
	QLabel				*FlblGatewaysList;
	SelectableTreeWidget *FTransportList;
	IServiceDiscovery	*FServiceDiscovery;
	IconStorage			*FIconStorageWizards;
	IconStorage			*FIconStorageServices;
	IconStorage			*FIconStorageMenu;
	QList<QDomElement>	FExcepFields;
	IDiscoItems			FDiscoItems;
	QList<Jid>			FPendingItems;
	QList<Jid>			FPendingItemsListed;
	const Jid			FStreamJid;
	QString				FNetwork;
};

//! ---Page_Process----
class ProcessPage : public QWizardPage
{
	Q_OBJECT
public:
	ProcessPage(Jid &AStreamJid, IRegistration *ARegistration,GatewayPage *AGatewayPage ,QWidget *parent = 0);
	void createGateway();
	IRegisterSubmit getSubmit();

	// QWizardPage interface
	void initializePage();
	int nextId() const;

protected:
	QWidget	*getWidget(const IDataField &AField);
	void	localTextLabel();
	QString	getLocalText(QString AKey);
	bool	checkField(const IDataField AField, QString AGateWay);

protected slots:
	void onRegisterFields(const QString &AId, const IRegisterFields &AFields);
	void onRegisterError(const QString &AId, const XmppError &AError);
	void onTextChanged(QString AText);
	void onComBoxChanged(QString AText);
	void onFormClicked(bool AState);
	void onMultiTextChanged();
	void onUserEditChanged(QString AText);
	void onPassEditChanged(QString AText);
	void onEmailEditChanged(QString AText);
	void onUrlEditChanged(QString AText);
	void onListMultiChanged(QString AText);
	void onLinkActivated();

private:
	Jid         FStreamJid;
	IRegistration *FRegistration;
	GatewayPage	*FGatewayPage;
	QScrollArea *FScrollArea;
	QGridLayout *FGridLayout;
	QCheckBox   *FAutoRegCheckBox;

	QLabel      *FInstrLabel;
	QLabel      *FErrorLabel;
	Jid         FServiceFrom;
	Jid         FServiceTo;
	QString     FRequestId;
	IRegisterSubmit FSubmit;
	bool        FDirection;
	IDataForm	FForm;
	QHash<QString,QVariant> FTmpFields;
	QString     FUserName;
	QString     FPassword;
	QString     FEmail;
	QString     FUrl;
	QList <QDomElement>	FExcepFields;
	QHash<QString,QString> FLocalText;
	const int	FFieldWidth;
};

//!-------ResultPage--------------
class ResultPage : public QWizardPage
{
	Q_OBJECT
public:
	ResultPage(Jid &AStreamJid, IRegistration *ARegistration,ProcessPage *AProcess,QWidget *parent = 0);

	// QWizardPage interface
	int nextId() const;
	bool isComplete() const;
	void initializePage();

protected slots:
	void onRegisterError(const QString &AId, const XmppError &AError);
	void onRegisterSuccessful(const QString &AId);
private:
	QVBoxLayout		*FLayout;
	QLabel          *FErrorLabel;
	Jid             FStreamJid;
	IRegistration   *FRegistration;
	IRegisterSubmit FSubmit;
	ProcessPage     *FProcess;
	QString         FRequestId;
	bool            FWizardGo;
};

//!---------------------
class ConclusionPage : public QWizardPage
{
	Q_OBJECT
public:
	ConclusionPage(NetworksPage *ANetworkPage, QWidget *parent = 0);
	void initializePage();
	int nextId() const;
private:
	QLabel *FLblTitle;
	QLabel *FLblText1;
	QLabel *FLblText2;
	QLabel *FLblText3;
	NetworksPage *FNetworkPage;
};

#endif // WIZARDPAGES_H
