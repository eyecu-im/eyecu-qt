#include <QtDebug>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDirIterator>
#include <QDomDocument>
#include <QCoreApplication>

#if QT_VERSION < 0x050000
#include <stdlib.h>
void myMessageOutput(QtMsgType AType, const QMessageLogContext &AContext, const QString &AMessage)
{
	switch (type) 
	{
		 case QtDebugMsg:
			 fprintf(stderr, "%s\n", msg);
			 break;
		 case QtWarningMsg:
			 fprintf(stderr, "Warning: %s\n", msg);
			 break;
		 case QtCriticalMsg:
			 fprintf(stderr, "Critical: %s\n", msg);
			 break;
		 case QtFatalMsg:
			 fprintf(stderr, "Fatal: %s\n", msg);
			 exit(-1);
	}
}
#else
#include <QMessageLogContext>
void myMessageHandler(QtMsgType AType, const QMessageLogContext &AContext, const QString &AMessage)
{
	Q_UNUSED(AContext)
	QByteArray localMsg = AMessage.toLocal8Bit();
	switch (AType)
	{
		 case QtDebugMsg:
			 fprintf(stderr, "%s\n", localMsg.constData());
			 break;
		 case QtWarningMsg:
			 fprintf(stderr, "Warning: %s\n", localMsg.constData());
			 break;
		 case QtCriticalMsg:
			 fprintf(stderr, "Critical: %s\n", localMsg.constData());
			 break;
		 case QtFatalMsg:
			 fprintf(stderr, "Fatal: %s\n", localMsg.constData());
			 abort();
	}
}
#endif

int main(int argc, char *argv[])
{
#if QT_VERSION < 0x050000
	qInstallMsgHandler(myMessageHandler);
#else
	qInstallMessageHandler(myMessageHandler);
#endif
	QCoreApplication app(argc, argv);

	if (argc != 2)
	{
		qCritical("Usage: autotranslate <target-dir>");
		return -1;
	}

	QDir srcDir(app.arguments().value(1),"*.ts",QDir::Name,QDir::Files);
	if (!srcDir.exists())
	{
		qCritical("Source directory '%s' not found.",srcDir.dirName().toLocal8Bit().constData());
		return -1;
	}

	QDirIterator srcIt(srcDir);
	while (srcIt.hasNext())
	{
		QFile srcFile(srcIt.next());
		if (srcFile.open(QFile::ReadOnly))
		{
			QDomDocument doc;
			if (doc.setContent(&srcFile,true))
			{
				qDebug("Generation auto translation from '%s'.",srcFile.fileName().toLocal8Bit().constData());

				QDomElement rootElem = doc.firstChildElement("TS");
				rootElem.setAttribute("language",rootElem.attribute("sourcelanguage","en"));
				
				QDomElement contextElem = rootElem.firstChildElement("context");
				while(!contextElem.isNull())
				{
					QDomElement messageElem = contextElem.firstChildElement("message");
					while(!messageElem.isNull())
					{
						QDomElement sourceElem = messageElem.firstChildElement("source");
						QDomElement translationElem = messageElem.firstChildElement("translation");
						if (!sourceElem.isNull() && !translationElem.isNull())
							if (translationElem.attribute("type")=="unfinished" && translationElem.text().isEmpty())
							{
								QString sourceText = sourceElem.text();
								if (messageElem.attribute("numerus") == "yes")
									qWarning("Untranslated numerus message: \"%s\"", sourceText.toLocal8Bit().constData());
								else
								{
									translationElem.removeChild(translationElem.firstChild());
									translationElem.appendChild(doc.createTextNode(sourceText));
									translationElem.removeAttribute("type");
								}
							}
						messageElem = messageElem.nextSiblingElement("message");
					}
					contextElem = contextElem.nextSiblingElement("context");
				}
				srcFile.close();
				if (srcFile.open(QFile::WriteOnly|QFile::Truncate))
				{
					srcFile.write(doc.toByteArray());
					srcFile.close();
				}
				else
					qWarning("Failed to open destination file '%s' for write.",srcFile.fileName().toLocal8Bit().constData());
			}
			else
				qWarning("Invalid translation source file '%s'.",srcFile.fileName().toLocal8Bit().constData());
			srcFile.close();
		}
		else
			qWarning("Could not open translation source file '%s'.",srcFile.fileName().toLocal8Bit().constData());
	}
	return 0;
}
