#include "webpage.h"

#include <QAction>
#include <QWebFrame>

WebPage::WebPage(QObject *AParent) : QWebPage(AParent)
{
	setContentEditable(false);
	setNetworkAccessManager(NULL);
	setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
}

WebPage::~WebPage()
{

}
