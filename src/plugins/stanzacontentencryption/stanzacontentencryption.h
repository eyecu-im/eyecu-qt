#ifndef STANZACONTENTENCRYPTION_H
#define STANZACONTENTENCRYPTION_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/istanzacontentencryption.h>

class StanzaContentEncrytion:
		public QObject,
		public IPlugin,
		public IStanzaContentEncrytion
 {
	Q_OBJECT
	Q_INTERFACES(IPlugin IStanzaContentEncrytion)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IStanzaContentEncrytion")
#endif
public:
	StanzaContentEncrytion();
	~StanzaContentEncrytion();

	//IPlugin
	virtual QObject *instance() override { return this; }
	virtual QUuid pluginUuid() const override { return STANZACONTENTENCRYPTION_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo) override;
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	virtual bool initObjects() override;
	virtual bool initSettings() override;
	virtual bool startPlugin() override {return true;}

	//IStanzaContentEncrytion
	virtual bool addAcceptableElement(const QString &ANamespace, const QString &ATagName) override;
	virtual bool removeAcceptableElement(const QString &ANamespace, const QString &ATagName) override;
	virtual bool isElementAcceptable(const QString &ANamespace, const QString &ATagName) const override;
	virtual bool addBlacklistedElement(const QString &ANamespace, const QString &ATagName=QString()) override;
	virtual bool removeBlacklistedElement(const QString &ANamespace, const QString &ATagName=QString()) override;
	virtual bool isElementBlacklisted(const QString &ANamespace, const QString &ATagName) const override;
	virtual bool isStanzaAcceptable(const Stanza &AStanza) const override;
	virtual QByteArray getContentToEncrypt(const Stanza &AStanza, const QString &AFallbackBodyText) const override;
	virtual bool putEncryptedContent(const Stanza &AStanza, const QByteArray &AContent) const override;

private:
	QMultiHash<QString, QString> FAcceptableElements;
	QMultiHash<QString, QString> FBlackList;
};

#endif // STANZACONTENTENCRYPTION_H
