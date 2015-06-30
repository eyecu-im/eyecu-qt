#include <QDesktopServices>
#include <QDir>
#include <QImageReader>
#include "imageopenthread.h"
#include <utils/logger.h>

ImageOpenThread::ImageOpenThread(const QUrl &AUrl, QObject *parent):
    QThread(parent),FUrl(AUrl)
{
    start();
}

void ImageOpenThread::run()
{
    if (FUrl.scheme()=="file")
    {
        int dot=FUrl.path().lastIndexOf('.');
        if (dot>0)
        {
            QString ext=FUrl.path().mid(dot+1);
            if (!ext.isEmpty())
            {
                QStringList validExtensions;
                validExtensions << "png" << "gif" << "jpg" << "jpeg" << "ico" << "bmp" << "tif" << "tiff" << "svg";
                if (validExtensions.contains(ext, Qt::CaseInsensitive))
                {
                    QDesktopServices::openUrl(FUrl.path());
                    return;
                }
            }
        }

		QDir temp = QDir::temp();
        if (temp.isReadable())
        {
            QString fileName=FUrl.toLocalFile();
            QImageReader reader(fileName);
            if (reader.canRead())
            {
                QString type(reader.format());
                QString tmpFileName(fileName);
                tmpFileName.append('.').append(type);
                QFile tmpFile(temp.filePath(tmpFileName));
                if (!tmpFile.exists() || tmpFile.size()!=reader.device()->size())    // Exists already
                    QFile::copy(fileName, tmpFileName);
                QDesktopServices::openUrl(QUrl::fromLocalFile(tmpFile.fileName()));
            }
            else
                LOG_WARNING("Reader can't' read!");
        }
    }
    else
        QDesktopServices::openUrl(FUrl);
}
