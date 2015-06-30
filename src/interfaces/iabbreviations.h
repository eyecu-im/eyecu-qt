#ifndef IABBREVIATIONS_H
#define IABBREVIATIONS_H

#include <QMap>
#include <QIcon>
#include <QString>
#include <QStringList>
#include <QTextDocument>

#define ABBREVIATIONS_UUID "{8cdb4a02-f58d-cb54-0d3b-acabf7162f62}"

class IAbbreviations
{
public:
	virtual QObject *instance() =0;
//	virtual QList<QString> activeAbbreviationSets() const =0;
	virtual QMap<int, QString> findTextAbbreviations(const QTextDocument *ADocument, int AStartPos=0, int ALength=-1) const =0;
};

Q_DECLARE_INTERFACE(IAbbreviations,"RWS.Plugin.IAbbreviations/1.0")

#endif
