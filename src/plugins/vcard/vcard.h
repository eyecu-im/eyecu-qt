#ifndef VCARD_H
#define VCARD_H

#include <interfaces/ivcardmanager.h>

#define VCARD_TAGNAME                   "vCard"

class VCardManager;

class VCard :
	public QObject,
	public IVCard
{
	Q_OBJECT;
	Q_INTERFACES(IVCard);
public:
	VCard(VCardManager *APlugin, const Jid &AContactJid);
	~VCard();
	virtual QObject *instance() { return this; }
	virtual bool isValid() const;
	virtual bool isEmpty() const;
	virtual Jid contactJid() const;
	virtual QDomElement vcardElem() const;
	virtual QDateTime loadDateTime() const;
	virtual QMultiHash<QString,QStringList> values(const QString &AName, const QStringList &ATagList) const;
	virtual QString value(const QString &AName, const QStringList &ATags = QStringList(), const QStringList &ATagList = QStringList()) const;
	virtual void setTagsForValue(const QString &AName, const QString &AValue, const QStringList &ATags = QStringList(), const QStringList &ATagList = QStringList());
	virtual void setValueForTags(const QString &AName, const QString &AValue, const QStringList &ATags = QStringList(), const QStringList &ATagList = QStringList());
	virtual void clear();
	virtual bool update(const Jid &AStreamJid);
	virtual bool publish(const Jid &AStreamJid, const Jid &AContactJid, bool AMuc=false);
	virtual void unlock();
signals:
	void vcardUpdated();
	void vcardPublished();
	void vcardError(const XmppError &AError);
protected:
	void loadVCardFile();
	QDomElement createElementByName(const QString &AName, const QStringList &ATags, const QStringList &ATagList);
	QDomElement firstElementByName(const QString &AName) const;
	QDomElement nextElementByName(const QString &AName, const QDomElement &APrevElem) const;
	QDomElement setTextToElem(QDomElement &AElem, const QString &AText) const;
protected slots:
	void onVCardReceived(const Jid &AContactJid);
	void onVCardPublished(const Jid &AStreamJid);
	void onVCardError(const Jid &AContactJid, const XmppError &AError);
private:
	VCardManager *FVCardPlugin;
private:
	Jid FStreamJid;
	Jid FContactJid;
	QDomDocument FDoc;
private:
	QDateTime FLoadDateTime;
};

#endif // VCARD_H
