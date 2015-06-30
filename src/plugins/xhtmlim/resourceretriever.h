#ifndef RESOURCERETRIEVER_H
#define RESOURCERETRIEVER_H

#include <QObject>
#include <QTextDocument>
#include <QTextEdit>
#include <QNetworkReply>

class ResourceRetriever : public QObject
{
    Q_OBJECT
public:
    ResourceRetriever(QTextEdit *ATextEdit, QNetworkReply *AReply);
    
signals:

private:
    QTextEdit *FTextEdit;
    
private slots:
    void onFinished();
};

#endif // RESOURCERETRIEVER_H
