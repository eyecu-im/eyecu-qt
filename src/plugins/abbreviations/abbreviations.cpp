#include <QSet>
#include <QChar>
#include <QTextBlock>
#include <QDir>
#include <definitions/resources.h>
#include <definitions/menuicons.h>
#include <definitions/actiongroups.h>
#include <definitions/toolbargroups.h>
#include <definitions/optionvalues.h>
#include <definitions/optionnodes.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionwidgetorders.h>
#include <definitions/messagewriterorders.h>
#include <definitions/messageeditcontentshandlerorders.h>
#include <utils/filestorage.h>
#include <utils/options.h>
#include <utils/logger.h>
#include <utils/menu.h>
#include <QpXhtml>

#include "abbreviations.h"

Abbreviations::Abbreviations()
{
	FMessageProcessor = nullptr;
	FMaxAbbreviationsInMessage = 20;
}

Abbreviations::~Abbreviations()
{
//	clearTreeItem(&FRootTreeItem);
}

void Abbreviations::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Abbreviations");
	APluginInfo->description = tr("Translates abbreviations frequently used in network communities");
	APluginInfo->version = "1.0";
    APluginInfo->author = "Road Works Software";
    APluginInfo->homePage = "http://www.eyecu.ru";
	APluginInfo->dependences.append(MESSAGEWIDGETS_UUID);
}

bool Abbreviations::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);

	IPlugin *plugin = APluginManager->pluginInterface("IMessageProcessor").value(0,NULL);
	if (plugin)
		FMessageProcessor = qobject_cast<IMessageProcessor *>(plugin->instance());

    return true;
}

bool Abbreviations::initObjects()
{
	if (FMessageProcessor)
		FMessageProcessor->insertMessageWriter(MWO_ABBREVIATIONS, this);

    readStorage();

	return true;
}

bool Abbreviations::initSettings()
{
	return true;
}

bool Abbreviations::writeMessageHasText(int AOrder, Message &AMessage, const QString &ALang)
{
	Q_UNUSED(AOrder); Q_UNUSED(AMessage); Q_UNUSED(ALang);

	return false;
}

bool Abbreviations::writeTextToMessage(int AOrder, QTextDocument *ADocument, Message &AMessage, const QString &ALang)
{
    Q_UNUSED(AOrder); Q_UNUSED(AMessage); Q_UNUSED(ADocument); Q_UNUSED(ALang);

	return false;
}

bool Abbreviations::writeMessageToText(int AOrder, Message &AMessage, QTextDocument *ADocument, const QString &ALang)
{
	Q_UNUSED(AOrder); Q_UNUSED(AMessage); Q_UNUSED(ALang);

	return translateAbbreviations(ADocument);
}

QMap<int, QString> Abbreviations::findTextAbbreviations(const QTextDocument *ADocument, int AStartPos, int ALength) const
{
	QMap<int,QString> abbreviations;
	QTextBlock block = ADocument->findBlock(AStartPos);
	int stopPos = ALength < 0 ? ADocument->characterCount() : AStartPos+ALength;
	while (block.isValid() && block.position()<stopPos)
	{
		for (QTextBlock::iterator it = block.begin(); !it.atEnd(); ++it)
		{
			QTextFragment fragment = it.fragment();
			if (fragment.length()>0 && fragment.position()<stopPos)
			{
				bool searchStarted = true;
				QString searchText = fragment.text();
				for (int keyPos=0; keyPos<searchText.length(); keyPos++)
				{
					searchStarted = searchStarted || searchText.at(keyPos).isSpace();
					if (searchStarted && !searchText.at(keyPos).isSpace())
					{
                        int keyLength = 0;
                        QString key = QString();
                        const AbbreviationTreeItem *item = &FRootTreeItem;                        
                        while (item && keyLength<=searchText.length()-keyPos && fragment.position()+keyPos+keyLength<=stopPos)
                        {
                            const QChar nextChar = keyPos+keyLength<searchText.length() ? searchText.at(keyPos+keyLength) : QChar(' ');
                            if (!item->translation.isEmpty())
                                key = searchText.mid(keyPos, keyLength);
                            keyLength++;
                            item = item->children.value(nextChar);
                        }
                        if (!key.isEmpty())
                        {
                            abbreviations.insert(fragment.position()+keyPos, key);
                            keyPos += key.size()-1;
                        }
						searchStarted = false;
					}
				}
			}
		}
		block = block.next();
	}
    return abbreviations;
}

void Abbreviations::createTreeItem(const QString &AKey, const QString &ATranslation)
{
    AbbreviationTreeItem *item = &FRootTreeItem;
    for (int i=0; i<AKey.size(); i++)
    {
        QChar itemChar = AKey.at(i);
        if (!item->children.contains(itemChar))
        {
            AbbreviationTreeItem *childItem = new AbbreviationTreeItem;
            item->children.insert(itemChar, childItem);
            item = childItem;
        }
        else
            item = item->children.value(itemChar);
    }
    item->translation = ATranslation;
}


bool Abbreviations::readStorage()
{
    QDir dir(FileStorage::resourcesDirs()[0]);
    if (dir.isReadable())
        if(dir.cd(RSR_STORAGE_ABBREVIATIONS))
            if(dir.cd(FILE_STORAGE_SHARED_DIR))
            {
                QStringList files = dir.entryList(QStringList() << FILE_STORAGE_DEFINITIONS_MASK);
                for(QStringList::const_iterator it=files.constBegin(); it!=files.constEnd(); ++it)
                {
                    QDomDocument doc;
                    QFile file(dir.filePath(*it));
                    if(file.open(QFile::ReadOnly) && doc.setContent(file.readAll(),false))
                    {
                        file.close();
                        QDomElement objElem = doc.documentElement().firstChildElement();
                        if(objElem.tagName() == "abbreviation")
                            for(QDomElement e = objElem; !e.isNull(); e = e.nextSiblingElement())
                            {
                                QString key = e.firstChildElement("key").text();
                                QString value = e.firstChildElement("value").text();
                                if (!key.isEmpty() && !value.isEmpty())
                                {
                                    createTreeItem(key, value);
                                    FTranslate.insert(key, value);
                                }
                            }
                    }
                }
                return true;
            }
    return false;
}

bool Abbreviations::isWordBoundary(const QString &AText) const
{
	return !AText.isEmpty() ? AText.at(0).isSpace() : true;
}

int Abbreviations::translateAbbreviations(QTextDocument *ADocument, int AStartPos, int ALength) const
{
	int posOffset = 0;    
    QMap<int,QString> abbreviations = findTextAbbreviations(ADocument,AStartPos,ALength);

    if (!abbreviations.isEmpty() && abbreviations.count()<=FMaxAbbreviationsInMessage)
	{
		QTextCursor cursor(ADocument);
		cursor.beginEditBlock();
        for (QMap<int,QString>::const_iterator it=abbreviations.constBegin(); it!=abbreviations.constEnd(); ++it)
		{
            QString translated = FTranslate.value(it.value());
            if (!translated.isEmpty())
			{
				cursor.setPosition(it.key()-posOffset);
				cursor.movePosition(QTextCursor::NextCharacter,QTextCursor::KeepAnchor,it->length());

                QTextCharFormat charFormat;
				charFormat.setProperty(QpXhtml::ToolTipType, QpXhtml::Abbreviation);
                charFormat.setProperty(QTextCharFormat::TextToolTip, translated);
                charFormat.setUnderlineStyle(QTextCharFormat::DotLine);
                charFormat.setUnderlineColor(Qt::red);
                cursor.setCharFormat(charFormat);
			}
		}
		cursor.endEditBlock();
	}
	return posOffset;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_abbreviations, Abbreviations)
#endif
