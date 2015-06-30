#ifndef ABBREVIATIONS_H
#define ABBREVIATIONS_H

#include <QHash>
#include <QStringList>
#include <interfaces/ipluginmanager.h>
#include <interfaces/iabbreviations.h>
#include <interfaces/imessageprocessor.h>
#include <interfaces/imessagewidgets.h>
#include <interfaces/ioptionsmanager.h>

struct AbbreviationTreeItem {
    QString translation;
    QMap<QChar, AbbreviationTreeItem *> children;
};

class Abbreviations :
	public QObject,
	public IPlugin,
	public IAbbreviations,
	public IMessageWriter
{
	Q_OBJECT
	Q_INTERFACES(IPlugin IAbbreviations IMessageWriter)
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IAbbreviations")
#endif
public:
	Abbreviations();
	~Abbreviations();
	//IPlugin
	virtual QObject *instance() { return this; }
	virtual QUuid pluginUuid() const { return ABBREVIATIONS_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo);
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder);
	virtual bool initObjects();
	virtual bool initSettings();
	virtual bool startPlugin() { return true; }
	//IMessageWriter
	virtual void writeTextToMessage(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	virtual void writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang);
	//IAbbreviations
	virtual QMap<int, QString> findTextAbbreviations(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;

protected:
    bool readStorage();
    void createTreeItem(const QString &AKey, const QString &ATranslation);
	bool isWordBoundary(const QString &AText) const;
    int translateAbbreviations(QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const;

private:
	IMessageProcessor *FMessageProcessor;
	int FMaxAbbreviationsInMessage;
    AbbreviationTreeItem FRootTreeItem;
    QHash<QString, QString> FTranslate;
};

#endif // ABBREVIATIONS_H
