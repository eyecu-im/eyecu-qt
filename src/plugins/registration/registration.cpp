#include "registration.h"

#include <definitions/namespaces.h>
#include <definitions/xmppfeatureorders.h>
#include <definitions/xmppfeaturefactoryorders.h>
#include <definitions/discofeaturehandlerorders.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/dataformtypes.h>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/xmppurihandlerorders.h>
#include <definitions/internalerrors.h>
#include <utils/xmpperror.h>
#include <utils/options.h>
#include <utils/stanza.h>
#include <utils/logger.h>

#define REGISTRATION_TIMEOUT    30000

#define ADR_STREAM_JID          Action::DR_StreamJid
#define ADR_SERVICE_JID         Action::DR_Parametr1
#define ADR_OPERATION           Action::DR_Parametr2

Registration::Registration()
{
	FDataForms = NULL;
	FXmppStreamManager = NULL;
	FStanzaProcessor = NULL;
	FDiscovery = NULL;
	FPresenceManager = NULL;
	FXmppUriQueries = NULL;
// *** <<< eyeCU <<< ***
	FOptionsManager = NULL;
	FAccountManager = NULL;
// *** >>> eyeCU >>> ***
}

Registration::~Registration()
{

}

void Registration::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Registration");
	APluginInfo->description = tr("Allows to register on the Jabber servers and services");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Potapov S.A. aka Lion";
	APluginInfo->homePage = "http://www.vacuum-im.org";
	APluginInfo->dependences.append(DATAFORMS_UUID);
	APluginInfo->dependences.append(STANZAPROCESSOR_UUID);
}

bool Registration::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	IPlugin *plugin = APluginManager->pluginInterface("IStanzaProcessor").value(0,NULL);
	if (plugin)
	{
		FStanzaProcessor = qobject_cast<IStanzaProcessor *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IDataForms").value(0,NULL);
	if (plugin)
	{
		FDataForms = qobject_cast<IDataForms *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IServiceDiscovery").value(0,NULL);
	if (plugin)
	{
		FDiscovery = qobject_cast<IServiceDiscovery *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IPresenceManager").value(0,NULL);
	if (plugin)
	{
		FPresenceManager = qobject_cast<IPresenceManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IXmppUriQueries").value(0,NULL);
	if (plugin)
	{
		FXmppUriQueries = qobject_cast<IXmppUriQueries *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IXmppStreamManager").value(0,NULL);
	if (plugin)
	{
		FXmppStreamManager = qobject_cast<IXmppStreamManager *>(plugin->instance());
	}
// *** <<< eyeCU <<< ***
	plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
	if (plugin)
	{
		FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	}

	plugin = APluginManager->pluginInterface("IAccountManager").value(0,NULL);
	if (plugin)
	{
		FAccountManager = qobject_cast<IAccountManager *>(plugin->instance());
	}
// *** >>> eyeCU >>> ***
	return FStanzaProcessor!=NULL && FDataForms!=NULL;
}

bool Registration::initObjects()
{
	XmppError::registerError(NS_INTERNAL_ERROR,IERR_REGISTER_UNSUPPORTED,tr("Registration is not supported"));
	XmppError::registerError(NS_INTERNAL_ERROR,IERR_REGISTER_INVALID_FIELDS,tr("Invalid registration fields"));
	XmppError::registerError(NS_INTERNAL_ERROR,IERR_REGISTER_REJECTED_BY_USER,tr("Registration rejected by user"));

	if (FXmppStreamManager)
	{
		FXmppStreamManager->registerXmppFeature(XFO_REGISTER,NS_FEATURE_REGISTER);
		FXmppStreamManager->registerXmppFeatureFactory(XFFO_DEFAULT,NS_FEATURE_REGISTER,this);
	}
	if (FDiscovery)
	{
		registerDiscoFeatures();
		FDiscovery->insertFeatureHandler(NS_JABBER_REGISTER,this,DFO_DEFAULT);
	}
	if (FDataForms)
	{
		FDataForms->insertLocalizer(this,DFT_REGISTER);
	}
	if (FXmppUriQueries)
	{
		FXmppUriQueries->insertUriHandler(XUHO_DEFAULT,this);
	}
	return true;
}

bool Registration::initSettings()
{
// *** <<< eyeCU <<< ***
	Options::setDefaultValue(OPV_ACCOUNT_REGISTER,false);
	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
// *** >>> eyeCU >>> ***
	return true;
}

void Registration::stanzaRequestResult(const Jid &AStreamJid, const Stanza &AStanza)
{
	XmppStanzaError err = !AStanza.isResult() ? XmppStanzaError(AStanza) : XmppStanzaError::null;

	if (FSendRequests.contains(AStanza.id()))
	{
		QDomElement queryElem = AStanza.firstElement("query",NS_JABBER_REGISTER);
		IRegisterFields fields = readFields(AStanza.from(),queryElem);
		if (AStanza.isResult() || (fields.fieldMask & IRegisterFields::Form)>0)
		{
			LOG_STRM_INFO(AStreamJid,QString("Registration fields loaded, from=%1, id=%2").arg(AStanza.from(),AStanza.id()));
			emit registerFields(AStanza.id(),fields);
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to load registration fields from=%1, id=%2: %3").arg(AStanza.from(),AStanza.id(),err.condition()));
			emit registerError(AStanza.id(),err);
		}
		FSendRequests.removeAll(AStanza.id());
	}
	else if (FSubmitRequests.contains(AStanza.id()))
	{
		if (AStanza.isResult())
		{
			LOG_STRM_INFO(AStreamJid,QString("Registration submit accepted, from=%1, id=%2").arg(AStanza.from(),AStanza.id()));
			emit registerSuccess(AStanza.id());
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Registration submit rejected, from=%1, id=%2: %3").arg(AStanza.from(),AStanza.id(),err.condition()));
			emit registerError(AStanza.id(),err);
		}
		FSubmitRequests.removeAll(AStanza.id());
	}
}

bool Registration::xmppUriOpen(const Jid &AStreamJid, const Jid &AContactJid, const QString &AAction, const QMultiMap<QString, QString> &AParams)
{
	Q_UNUSED(AParams);
	if (AAction == "register")
		return showRegisterDialog(AStreamJid,AContactJid,IRegistration::Register,NULL)!=NULL;
	else if (AAction == "unregister")
		return showRegisterDialog(AStreamJid,AContactJid,IRegistration::Unregister,NULL)!=NULL;
	return false;
}

bool Registration::execDiscoFeature(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo)
{
	if (AFeature == NS_JABBER_REGISTER)
		return showRegisterDialog(AStreamJid,ADiscoInfo.contactJid,IRegistration::Register,NULL)!=NULL;
	return false;
}

Action *Registration::createDiscoFeatureAction(const Jid &AStreamJid, const QString &AFeature, const IDiscoInfo &ADiscoInfo, QWidget *AParent)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence && presence->isOpen() && AFeature==NS_JABBER_REGISTER)
	{
		Menu *regMenu = new Menu(AParent);
		regMenu->setTitle(tr("Registration"));
		regMenu->setIcon(RSR_STORAGE_MENUICONS,MNI_REGISTRATION);

		Action *action = new Action(regMenu);
		action->setText(tr("Register"));
		action->setIcon(RSR_STORAGE_MENUICONS,MNI_REGISTRATION);
		action->setData(ADR_STREAM_JID,AStreamJid.full());
		action->setData(ADR_SERVICE_JID,ADiscoInfo.contactJid.full());
		action->setData(ADR_OPERATION,IRegistration::Register);
		connect(action,SIGNAL(triggered(bool)),SLOT(onRegisterActionTriggered(bool)));
		regMenu->addAction(action,AG_DEFAULT,false);

		action = new Action(regMenu);
		action->setText(tr("Unregister"));
		action->setIcon(RSR_STORAGE_MENUICONS,MNI_REGISTRATION_REMOVE);
		action->setData(ADR_STREAM_JID,AStreamJid.full());
		action->setData(ADR_SERVICE_JID,ADiscoInfo.contactJid.full());
		action->setData(ADR_OPERATION,IRegistration::Unregister);
		connect(action,SIGNAL(triggered(bool)),SLOT(onRegisterActionTriggered(bool)));
		regMenu->addAction(action,AG_DEFAULT,false);

		action = new Action(regMenu);
		action->setText(tr("Change password"));
		action->setIcon(RSR_STORAGE_MENUICONS,MNI_REGISTRATION_CHANGE);
		action->setData(ADR_STREAM_JID,AStreamJid.full());
		action->setData(ADR_SERVICE_JID,ADiscoInfo.contactJid.full());
		action->setData(ADR_OPERATION,IRegistration::ChangePassword);
		connect(action,SIGNAL(triggered(bool)),SLOT(onRegisterActionTriggered(bool)));
		regMenu->addAction(action,AG_DEFAULT,false);

		return regMenu->menuAction();
	}
	return NULL;
}

QList<QString> Registration::xmppFeatures() const
{
	return QList<QString>() << NS_FEATURE_REGISTER;
}

IXmppFeature *Registration::newXmppFeature(const QString &AFeatureNS, IXmppStream *AXmppStream)
{
// *** <<< eyeCU <<< ***	
	if (AFeatureNS==NS_FEATURE_REGISTER)
	{
		IAccount *account = FAccountManager!=NULL ? FAccountManager->findAccountByStream(AXmppStream->streamJid()) : NULL;
		if (account && account->optionsNode().value("register-on-server").toBool() && !FStreamRegisterId.contains(AXmppStream))
		{
			LOG_INFO(QString("XMPP account automatic registration started, server=%1").arg(AXmppStream->streamJid().pDomain()));
			FAutoRegisterStreams.insert(AXmppStream);

			QString id = QUuid::createUuid().toString();
			FStreamRegisterId.insert(AXmppStream,id);

			connect(AXmppStream->instance(),SIGNAL(opened()),SLOT(onXmppStreamOpened()));
			connect(AXmppStream->instance(),SIGNAL(closed()),SLOT(onXmppStreamClosed()));
			connect(AXmppStream->instance(),SIGNAL(error(const XmppError &)),SLOT(onXmppStreamError(const XmppError &)));

			account->optionsNode().setValue(false,"register-on-server");
		}
	if (FStreamRegisterId.contains(AXmppStream) && !FStreamFeatures.contains(AXmppStream))
// *** >>> eyeCU >>> ***
	{
		LOG_INFO(QString("XMPP account registration feature created, server=%1").arg(AXmppStream->streamJid().pDomain()));

		RegisterFeature *feature = new RegisterFeature(AXmppStream);
		connect(feature->instance(),SIGNAL(registerFields(const IRegisterFields &)),SLOT(onXmppFeatureFields(const IRegisterFields &)));
		connect(feature->instance(),SIGNAL(finished(bool)),SLOT(onXmppFeatureFinished(bool)));
		connect(feature->instance(),SIGNAL(featureDestroyed()),SLOT(onXmppFeatureDestroyed()));

		FStreamFeatures.insert(AXmppStream,feature);
		emit featureCreated(feature);
		return feature;
	}
	}	// *** <<< eyeCU >>> ***
	return NULL;
}
// *** <<< eyeCU <<< ***
QMultiMap<int, IOptionsDialogWidget *> Registration::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (Options::node(OPV_COMMON_ADVANCED).value().toBool())
	{
		QStringList nodeTree = ANodeId.split(".",QString::SkipEmptyParts);
		if (FOptionsManager && nodeTree.count()==3 && nodeTree.at(0)==OPN_ACCOUNTS && nodeTree.at(2)=="Parameters")
		{
			widgets.insertMulti(OHO_ACCOUNT_REGISTRATION, FOptionsManager->newOptionsDialogHeader(tr("Registration"), AParent));
			widgets.insertMulti(OWO_ACCOUNT_REGISTER, FOptionsManager->newOptionsDialogWidget(Options::node(OPV_ACCOUNT_ITEM,nodeTree.at(1)).node("register-on-server"),tr("Register new account on server"),AParent));
		}
	}
	return widgets;
}
// *** >>> eyeCU >>> ***
IDataFormLocale Registration::dataFormLocale(const QString &AFormType)
{
	IDataFormLocale locale;
	if (AFormType == DFT_REGISTER)
	{
		locale.title = tr("Registration Form");
		locale.fields["username"].label = tr("Account Name");
		locale.fields["nick"].label = tr("Nickname");
		locale.fields["password"].label = tr("Password");
		locale.fields["name"].label = tr("Full Name");
		locale.fields["first"].label = tr("Given Name");
		locale.fields["last"].label = tr("Family Name");
		locale.fields["email"].label = tr("Email Address");
		locale.fields["address"].label = tr("Street");
		locale.fields["city"].label = tr("City");
		locale.fields["state"].label = tr("Region");
		locale.fields["zip"].label = tr("Zip Code");
		locale.fields["phone"].label = tr("Telephone Number");
		locale.fields["url"].label = tr("Your Web Page");
	}
	return locale;
}

QString Registration::startStreamRegistration(IXmppStream *AXmppStream)
{
	if (AXmppStream!=NULL && !FStreamRegisterId.contains(AXmppStream) && AXmppStream->open())
	{
		LOG_INFO(QString("XMPP account registration started, server=%1").arg(AXmppStream->streamJid().pDomain()));

		QString id = QUuid::createUuid().toString();
		FStreamRegisterId.insert(AXmppStream,id);

		connect(AXmppStream->instance(),SIGNAL(opened()),SLOT(onXmppStreamOpened()));
		connect(AXmppStream->instance(),SIGNAL(closed()),SLOT(onXmppStreamClosed()));
		connect(AXmppStream->instance(),SIGNAL(error(const XmppError &)),SLOT(onXmppStreamError(const XmppError &)));

		return id;
	}
	else if (AXmppStream != NULL)
	{
		LOG_ERROR(QString("Failed to create XMPP account registration feature, server=%1").arg(AXmppStream->streamJid().pDomain()));
	}
	return QString();
}

QString Registration::submitStreamRegistration(IXmppStream *AXmppStream, const IRegisterSubmit &ASubmit)
{
	RegisterFeature *feature = FStreamFeatures.value(AXmppStream);
	if (feature != NULL)
	{
		if (feature->sendSubmit(ASubmit))
			return FStreamRegisterId.value(feature->xmppStream());
	}
	return QString();
}

QString Registration::sendRegisterRequest(const Jid &AStreamJid, const Jid &AServiceJid)
{
	if (FStanzaProcessor && AStreamJid.isValid() && AServiceJid.isValid())
	{
		Stanza request(STANZA_KIND_IQ);
		request.setType(STANZA_TYPE_GET).setTo(AServiceJid.full()).setUniqueId();
		request.addElement("query",NS_JABBER_REGISTER);
		if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,REGISTRATION_TIMEOUT))
		{
			LOG_STRM_INFO(AStreamJid,QString("Registration register request sent, to=%1, id=%2").arg(AStreamJid.full(),request.id()));
			FSendRequests.append(request.id());
			return request.id();
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send registration register request, to=%1, id=%2").arg(AServiceJid.full(),request.id()));
		}
	}
	else if (FStanzaProcessor)
	{
		REPORT_ERROR("Failed to send registration register request: Invalid parameters");
	}
	return QString();
}

QString Registration::sendUnregisterRequest(const Jid &AStreamJid, const Jid &AServiceJid)
{
	if (FStanzaProcessor && AStreamJid.isValid() && AServiceJid.isValid())
	{
		Stanza request(STANZA_KIND_IQ);
		request.setType(STANZA_TYPE_SET).setTo(AServiceJid.full()).setUniqueId();
		request.addElement("query",NS_JABBER_REGISTER).appendChild(request.createElement("remove"));
		if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,REGISTRATION_TIMEOUT))
		{
			LOG_STRM_INFO(AStreamJid,QString("Registration unregister request sent, to=%1, id=%2").arg(AStreamJid.full(),request.id()));
			FSubmitRequests.append(request.id());
			return request.id();
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send registration unregister request, to=%1").arg(AStreamJid.full()));
		}
	}
	else if (FStanzaProcessor)
	{
		REPORT_ERROR("Failed to send registration unregister request: Invalid parameters");
	}
	return QString();
}

QString Registration::sendChangePasswordRequest(const Jid &AStreamJid, const Jid &AServiceJid, const QString &AUserName, const QString &APassword)
{
	if (FStanzaProcessor && AStreamJid.isValid() && AServiceJid.isValid())
	{
		Stanza request(STANZA_KIND_IQ);
		request.setType(STANZA_TYPE_SET).setTo(AServiceJid.full()).setUniqueId();
		QDomElement elem = request.addElement("query",NS_JABBER_REGISTER);
		elem.appendChild(request.createElement("username")).appendChild(request.createTextNode(AUserName));
		elem.appendChild(request.createElement("password")).appendChild(request.createTextNode(APassword));
		if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,REGISTRATION_TIMEOUT))
		{
			LOG_STRM_INFO(AStreamJid,QString("Registration change password request sent, to=%1, id=%2").arg(AServiceJid.full(),request.id()));
			FSubmitRequests.append(request.id());
			return request.id();
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send registration change password request, to=%1").arg(AServiceJid.full()));
		}
	}
	else if (FStanzaProcessor)
	{
		REPORT_ERROR("Failed to send registration change password request: Invalid parameters");
	}
	return QString();
}

QString Registration::sendRequestSubmit(const Jid &AStreamJid, const IRegisterSubmit &ASubmit)
{
	if (FStanzaProcessor && AStreamJid.isValid())
	{
		Stanza request(STANZA_KIND_IQ);
		request.setType(STANZA_TYPE_SET).setTo(ASubmit.serviceJid.full()).setUniqueId();

		QDomElement queryElem = request.addElement("query",NS_JABBER_REGISTER);
		writeSubmit(queryElem,ASubmit);

		if (FStanzaProcessor->sendStanzaRequest(this,AStreamJid,request,REGISTRATION_TIMEOUT))
		{
			LOG_STRM_INFO(AStreamJid,QString("Registration submit request sent, to=%1, id=%2").arg(ASubmit.serviceJid.full(),request.id()));
			FSubmitRequests.append(request.id());
			return request.id();
		}
		else
		{
			LOG_STRM_WARNING(AStreamJid,QString("Failed to send registration submit request, to=%1, id=%2").arg(ASubmit.serviceJid.full(),request.id()));
		}
	}
	else if (FStanzaProcessor)
	{
		REPORT_ERROR("Failed to send registration submit request: Invalid parameters");
	}
	return QString();
}

QDialog *Registration::showRegisterDialog(const Jid &AStreamJid, const Jid &AServiceJid, int AOperation, QWidget *AParent)
{
	IPresence *presence = FPresenceManager!=NULL ? FPresenceManager->findPresence(AStreamJid) : NULL;
	if (presence && presence->isOpen())
	{
		RegisterDialog *dialog = new RegisterDialog(this,FDataForms,AStreamJid,AServiceJid,AOperation,AParent);
		connect(presence->instance(),SIGNAL(closed()),dialog,SLOT(reject()));
		dialog->show();
		return dialog;
	}
	return NULL;
}

void Registration::registerDiscoFeatures()
{
	IDiscoFeature dfeature;
	dfeature.active = false;
	dfeature.icon = IconStorage::staticStorage(RSR_STORAGE_MENUICONS)->getIcon(MNI_REGISTRATION);
	dfeature.var = NS_JABBER_REGISTER;
	dfeature.name = tr("Registration");
	dfeature.description = tr("Supports the registration");
	FDiscovery->insertDiscoFeature(dfeature);
}

IRegisterFields Registration::readFields(const Jid &AServiceJid, const QDomElement &AQuery) const
{
	IRegisterFields fields;
	fields.serviceJid = AServiceJid;
	fields.registered = !AQuery.firstChildElement("registered").isNull();
	fields.instructions = AQuery.firstChildElement("instructions").text();
	fields.key = AQuery.firstChildElement("key").text();

	fields.fieldMask = 0;
	if (!AQuery.firstChildElement("username").isNull())
	{
		fields.fieldMask |= IRegisterFields::Username;
		fields.username = AQuery.firstChildElement("username").text();
	}
	else if (!AQuery.firstChildElement("name").isNull())
	{
		fields.fieldMask |= IRegisterFields::Username;
		fields.username = AQuery.firstChildElement("name").text();
	}
	if (!AQuery.firstChildElement("password").isNull())
	{
		fields.fieldMask |= IRegisterFields::Password;
		fields.password = AQuery.firstChildElement("password").text();
	}
	if (!AQuery.firstChildElement("email").isNull())
	{
		fields.fieldMask |= IRegisterFields::Email;
		fields.email = AQuery.firstChildElement("email").text();
	}

	QDomElement oob = AQuery.firstChildElement("x");
	while (!oob.isNull())
	{
		if (oob.namespaceURI() == NS_JABBER_OOB_X)
		{
			fields.fieldMask |= IRegisterFields::Redirect;
			fields.redirect = oob.firstChildElement("url").text();
			break;
		}
		oob = oob.nextSiblingElement("x");
	}

	QDomElement formElem = AQuery.firstChildElement("x");
	while (FDataForms!=NULL && !formElem.isNull())
	{
		if (formElem.namespaceURI()==NS_JABBER_DATA && formElem.attribute("type",DATAFORM_TYPE_FORM)==DATAFORM_TYPE_FORM)
		{
			fields.fieldMask |= IRegisterFields::Form;
			fields.form = FDataForms->dataForm(formElem);
			break;
		}
		formElem = formElem.nextSiblingElement("x");
	}

	return fields;
}

bool Registration::writeSubmit(QDomElement &AQuery, const IRegisterSubmit &ASubmit) const
{
	if ((ASubmit.fieldMask & IRegisterFields::Form) == 0)
	{
		QDomDocument doc = AQuery.ownerDocument();
		if (ASubmit.fieldMask & IRegisterFields::Username)
			AQuery.appendChild(doc.createElement("username")).appendChild(doc.createTextNode(ASubmit.username));
		if (ASubmit.fieldMask & IRegisterFields::Password)
			AQuery.appendChild(doc.createElement("password")).appendChild(doc.createTextNode(ASubmit.password));
		if (ASubmit.fieldMask & IRegisterFields::Email)
			AQuery.appendChild(doc.createElement("email")).appendChild(doc.createTextNode(ASubmit.email));
		if (!ASubmit.key.isEmpty())
			AQuery.appendChild(doc.createElement("key")).appendChild(doc.createTextNode(ASubmit.key));
		return true;
	}
	else if (FDataForms)
	{
		FDataForms->xmlForm(ASubmit.form,AQuery);
		return true;
	}
	return false;
}

void Registration::onXmppFeatureFields(const IRegisterFields &AFields)
{
	RegisterFeature *feature = qobject_cast<RegisterFeature *>(sender());
	if (feature != NULL)
	{
		QString registerId = FStreamRegisterId.value(feature->xmppStream());
// *** <<< eyeCU <<< ***
		if (FAutoRegisterStreams.contains(feature->xmppStream()))
			processFields(registerId, AFields);
		else
// *** >>> eyeCU >>> ***
		emit registerFields(registerId,AFields);
	}
}
// *** <<< eyeCU <<< ***
void Registration::processFields(const QString &AId, const IRegisterFields &AFields)
{
	IXmppStream *xmppStream = FStreamRegisterId.key(AId);
	if (xmppStream)
	{
		IRegisterFields registerFields = AFields;
		bool extraFields = false;
		if ((AFields.fieldMask & IRegisterFields::Form) == 0)
		{
			registerFields.form.type = DATAFORM_TYPE_FORM;
			registerFields.form.instructions.append(AFields.instructions);
			if (AFields.fieldMask & IRegisterFields::Username)
			{
				IDataField dataField;
				dataField.var = "username";
				dataField.type = DATAFIELD_TYPE_HIDDEN;
				dataField.value = xmppStream->streamJid().node();
				registerFields.form.fields.append(dataField);
			}
			if (AFields.fieldMask & IRegisterFields::Password)
			{
				IDataField dataField;
				dataField.var = "password";
				dataField.type = DATAFIELD_TYPE_HIDDEN;
				dataField.label = tr("Password");
				dataField.value = xmppStream->password();
				registerFields.form.fields.append(dataField);
			}
			if (AFields.fieldMask & IRegisterFields::Email)
			{
				IDataField dataField;
				dataField.var = "email";
				dataField.type = DATAFIELD_TYPE_TEXTSINGLE;
				dataField.label = tr("e-mail");
				dataField.required = true;
				registerFields.form.fields.append(dataField);
				extraFields = true;
			}
		}
		else
		{
			for (QList<IDataField>::Iterator it = registerFields.form.fields.begin(); it != registerFields.form.fields.end(); it++)
			{
				// Hide unneeded fields
				if (((*it).type == DATAFIELD_TYPE_TEXTSINGLE) && ((*it).var == "username"))
				{
					(*it).value = xmppStream->streamJid().node();
					(*it).type = DATAFIELD_TYPE_HIDDEN;
				}

				else if (((*it).type == DATAFIELD_TYPE_TEXTPRIVATE) && ((*it).var == "password"))
				{
					(*it).value = xmppStream->password();
					(*it).type = DATAFIELD_TYPE_HIDDEN;
				}
				else
					extraFields = true;
			}
		}

		IRegisterSubmit registerSubmit;
		registerSubmit.key = registerFields.key;
		registerSubmit.serviceJid = registerFields.serviceJid;
		if (extraFields)
		{
			QDialog *registerDialog = new QDialog();			
			if (!registerFields.form.title.isEmpty())
				registerDialog->setWindowTitle(registerFields.form.title);
			IDataFormWidget *dfwRegisterForm = FDataForms->formWidget(registerFields.form, registerDialog);
			dfwRegisterForm->instance()->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
			registerDialog->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
			QVBoxLayout *dialogLayout = new QVBoxLayout(registerDialog);
			dialogLayout->setMargin(0);
			dialogLayout->addWidget(dfwRegisterForm->instance());
			QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel, Qt::Horizontal);
			dialogLayout->addWidget(buttonBox);
			dfwRegisterForm->instance()->layout()->setSizeConstraint(QLayout::SetMinimumSize);
			dialogLayout->setSizeConstraint(QLayout::SetMinimumSize);
			connect(buttonBox, SIGNAL(accepted()), registerDialog, SLOT(accept()));
			connect(buttonBox, SIGNAL(rejected()), registerDialog, SLOT(reject()));
			if (registerDialog->exec()==QDialog::Accepted)
			{
				if (registerFields.fieldMask & IRegisterFields::Form)
				{
					registerSubmit.form = FDataForms->dataSubmit(dfwRegisterForm ->userDataForm());
					registerSubmit.fieldMask = IRegisterFields::Form;
				}
				else
				{
					IDataForm form = dfwRegisterForm ->userDataForm();
					registerSubmit.username = FDataForms->fieldValue("username",form.fields).toString();
					registerSubmit.password = FDataForms->fieldValue("password",form.fields).toString();
					registerSubmit.email = FDataForms->fieldValue("email",form.fields).toString();
					registerSubmit.fieldMask = registerFields.fieldMask;
				}
				submitStreamRegistration(xmppStream, registerSubmit);
			}
			else
				xmppStream->abort(XmppError(IERR_REGISTER_REJECTED_BY_USER));
			registerDialog->deleteLater();
		}
		else
		{
			if (registerFields.fieldMask & IRegisterFields::Form)
			{
				registerSubmit.form = registerFields.form;
				registerSubmit.fieldMask = IRegisterFields::Form;
			}
			else
			{
				registerSubmit.username = registerFields.username;
				registerSubmit.password = registerFields.password;
				registerSubmit.email = registerFields.email;
				registerSubmit.fieldMask = registerFields.fieldMask;
			}
			submitStreamRegistration(xmppStream, registerSubmit);
		}
	}
}
// *** >>> eyeCU >>> ***

void Registration::onXmppFeatureFinished(bool ARestart)
{
	Q_UNUSED(ARestart);
	RegisterFeature *feature = qobject_cast<RegisterFeature *>(sender());
	if (feature != NULL)
	{
		IRegisterSubmit submit = feature->sentSubmit();
		QString username = submit.username;
		QString password = submit.password;
		if (FDataForms && (submit.fieldMask & IRegisterFields::Form)>0)
		{
			username = FDataForms->fieldValue("username",submit.form.fields).toString();
			password = FDataForms->fieldValue("password",submit.form.fields).toString();
		}

		IXmppStream *xmppStream = feature->xmppStream();
		xmppStream->setStreamJid(Jid(username,submit.serviceJid.domain(),"Registration"));
		xmppStream->setPassword(password);
	}
}

void Registration::onXmppFeatureDestroyed()
{
	RegisterFeature *feature = qobject_cast<RegisterFeature *>(sender());
	if (feature)
	{
		LOG_INFO(QString("XMPP account registration feature destroyed, server=%1").arg(feature->xmppStream()->streamJid().pDomain()));
		emit featureDestroyed(feature);
	}
}

void Registration::onXmppStreamOpened()
{
	IXmppStream *xmppStream = qobject_cast<IXmppStream *>(sender());
	if (FStreamRegisterId.contains(xmppStream))
	{
// *** <<< eyeCU <<< ***
		if (FAutoRegisterStreams.contains(xmppStream))
			FAutoRegisterStreams.remove(xmppStream);
		else
		{
// *** >>> eyeCU >>> ***
		QString registerId = FStreamRegisterId.value(xmppStream);
		if (FStreamFeatures.contains(xmppStream))
			emit registerSuccess(registerId);
		else
			emit registerError(registerId,XmppError(IERR_REGISTER_UNSUPPORTED));
		xmppStream->close();
		} // *** <<< eyeCU >>> ***
	}
}

void Registration::onXmppStreamClosed()
{
	IXmppStream *xmppStream = qobject_cast<IXmppStream *>(sender());
	if (xmppStream)
	{
		disconnect(xmppStream->instance());
		FStreamFeatures.remove(xmppStream);
		FStreamRegisterId.remove(xmppStream);
// *** <<< eyeCU <<< ***
		if (FAutoRegisterStreams.contains(xmppStream))
			FAutoRegisterStreams.remove(xmppStream);
// *** >>> eyeCU >>> ***
	}
}

void Registration::onXmppStreamError(const XmppError &AError)
{
	IXmppStream *xmppStream = qobject_cast<IXmppStream *>(sender());
	if (FStreamRegisterId.contains(xmppStream))
	{
// *** <<< eyeCU <<< ***
		if (FAutoRegisterStreams.contains(xmppStream))
			FAutoRegisterStreams.remove(xmppStream);
// *** >>> eyeCU >>> ***
		QString registerId = FStreamRegisterId.value(xmppStream);
		RegisterFeature *feature = FStreamFeatures.value(xmppStream);
		if (feature!=NULL && feature->isFinished())
			emit registerSuccess(registerId);
		else if (feature!=NULL || AError.errorNs()!=NS_XMPP_SASL_ERROR)
			emit registerError(registerId,AError);
		else
			emit registerError(registerId,XmppError(IERR_REGISTER_UNSUPPORTED));
	}
}

void Registration::onRegisterActionTriggered(bool)
{
	Action *action = qobject_cast<Action *>(sender());
	if (action)
	{
		Jid streamJid = action->data(ADR_STREAM_JID).toString();
		Jid serviceJid = action->data(ADR_SERVICE_JID).toString();
		int operation = action->data(ADR_OPERATION).toInt();
		showRegisterDialog(streamJid,serviceJid,operation,NULL);
	}
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_registration, Registration)
#endif
