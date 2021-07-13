#ifndef OPTIONS_H
#define OPTIONS_H

#include <QObject>
#include <QVariant>
#include <QByteArray>
#include <QDomDocument>
#include "utilsexport.h"

class UTILS_EXPORT OptionsNode
{
	friend class Options;
	struct OptionsNodeData;
public:
	OptionsNode();
	OptionsNode(const OptionsNode &ANode);
	~OptionsNode();
	bool isNull() const;
	QString path() const;
	QString name() const;
	QString nspace() const;
	QString cleanPath() const;
	OptionsNode parent() const;
	QList<QString> parentNSpaces() const;
	QList<QString> childNames() const;
	QList<QString> childNSpaces(const QString &AName) const;
	bool isChildNode(const OptionsNode &ANode) const;
	QString childPath(const OptionsNode &ANode) const;
	void removeChilds(const QString &AName = QString(), const QString &ANSpace = QString());
	bool hasNode(const QString &APath, const QString &ANSpace = QString()) const;
	OptionsNode node(const QString &APath, const QString &ANSpace = QString()) const;
	void removeNode(const QString &APath, const QString &ANSpace = QString());
	bool hasValue(const QString &APath = QString(), const QString &ANSpace = QString()) const;
	QVariant value(const QString &APath = QString(), const QString &ANSpace = QString()) const;
	void setValue(const QVariant &AValue, const QString &APath = QString(), const QString &ANSpace = QString());
public:
	bool operator==(const OptionsNode &AOther) const;
	bool operator!=(const OptionsNode &AOther) const;
	OptionsNode &operator=(const OptionsNode &AOther);
public:
	static const OptionsNode null;
private:
	OptionsNode(const QDomElement &ANode);
	OptionsNodeData *d;
};

class UTILS_EXPORT Options :
	public QObject
{
	Q_OBJECT;
	friend class OptionsNode;
	struct OptionsData;
public:
	static Options *instance();
	static bool isNull();
	static QString filesPath();
	static QByteArray cryptKey();
	static QString cleanNSpaces(const QString &APath);
	static bool hasNode(const QString &APath, const QString &ANSpace = QString());
	static OptionsNode node(const QString &APath, const QString &ANSpace = QString());
	static QVariant fileValue(const QString &APath, const QString &ANSpace = QString());
	static void setFileValue(const QVariant &AValue, const QString &APath, const QString &ANSpace = QString());
	static void setOptions(const QDomDocument &AOptions, const QString &AFilesPath, const QByteArray &ACryptKey);
	static QVariant defaultValue(const QString &APath);
	static void setDefaultValue(const QString &APath, const QVariant &ADefault);
	static QByteArray encrypt(const QVariant &AValue, const QByteArray &AKey = cryptKey());
	static QVariant decrypt(const QByteArray &AData, const QByteArray &AKey = cryptKey());
public:
	static QString variantToString(const QVariant &AValue);
	static QVariant stringToVariant(const QString &AValue, QVariant::Type AType);
	static void exportNode(const QString &APath, QDomElement &AToElem);
	static void importNode(const QString &APath, const QDomElement &AFromElem);
	static OptionsNode createNodeForElement(const QDomElement &AElement);
signals:
	void optionsOpened();
	void optionsClosed();
	void optionsCreated(const OptionsNode &ANode);
	void optionsChanged(const OptionsNode &ANode);
	void optionsRemoved(const OptionsNode &ANode);
	void defaultValueChanged(const QString &APath, const QVariant &ADefault);
private:
	Options();
	~Options();
	OptionsData *d;
};

#endif // OPTIONS_H
