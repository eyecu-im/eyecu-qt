#ifndef VIEWWIDGET_H
#define VIEWWIDGET_H

#include <QDropEvent>
#include <QDragMoveEvent>
#include <QDragEnterEvent>
#include <QDragLeaveEvent>
#include <interfaces/imessagewidgets.h>
#include <interfaces/imessageprocessor.h>
#include "ui_viewwidget.h"

class ViewWidget :
	public QWidget,
	public IMessageViewWidget
{
	Q_OBJECT;
	Q_INTERFACES(IMessageWidget IMessageViewWidget);
public:
	ViewWidget(IMessageWidgets *AMessageWidgets, IMessageWindow *AWindow, QWidget *AParent);
	~ViewWidget();
	//IMessageWidget
	virtual QWidget *instance() { return this; }
	virtual bool isVisibleOnWindow() const;
	virtual IMessageWindow *messageWindow() const;
	//IMessageViewWidget
	virtual void clearContent();
	virtual QWidget *styleWidget() const;
	virtual IMessageStyle *messageStyle() const;
	virtual void setMessageStyle(IMessageStyle *AStyle, const IMessageStyleOptions &AOptions);
	virtual bool appendHtml(const QString &AHtml, const IMessageStyleContentOptions &AOptions);
	virtual bool appendText(const QString &AText, const IMessageStyleContentOptions &AOptions);
	virtual bool appendMessage(const Message &AMessage, const IMessageStyleContentOptions &AOptions);
	virtual void contextMenuForView(const QPoint &APosition, Menu *AMenu);
	virtual QTextDocumentFragment selection() const;
	virtual QTextCharFormat textFormatAt(const QPoint &APosition) const;
	virtual QTextDocumentFragment textFragmentAt(const QPoint &APosition) const;
// *** <<< eyeCU <<< ***
	virtual QImage imageAt(const QPoint &APosition) const;
	virtual bool setImageUrl(const QString &AId, const QString &AUrl);
	virtual bool setObjectTitle(const QString &AId, const QString &ATitle);
// *** >>> eyeCU >>> ***
signals:
	void urlClicked(const QUrl &AUrl);
	void viewContextMenu(const QPoint &APosition, Menu *AMenu);
	void contentAppended(const QString &AHtml, const IMessageStyleContentOptions &AOptions);
	void messageStyleOptionsChanged(const IMessageStyleOptions &AOptions, bool ACleared);
	void messageStyleChanged(IMessageStyle *ABefore, const IMessageStyleOptions &AOptions);
protected:
	void initialize();
protected:
	void dropEvent(QDropEvent *AEvent);
	void dragEnterEvent(QDragEnterEvent *AEvent);
	void dragMoveEvent(QDragMoveEvent *AEvent);
	void dragLeaveEvent(QDragLeaveEvent *AEvent);
protected slots:
	void onMessageStyleUrlClicked(QWidget *AWidget, const QUrl &AUrl);
	void onMessageStyleWidgetCustomContextMenuRequested(const QPoint &APosition);
	void onMessageStyleOptionsChanged(QWidget *AWidget, const IMessageStyleOptions &AOptions, bool AClean);
	void onMessageStyleContentAppended(QWidget *AWidget, const QString &AHtml, const IMessageStyleContentOptions &AOptions);
private:
	Ui::ViewWidgetClass ui;
private:
	IMessageStyle *FMessageStyle;
	IMessageWidgets *FMessageWidgets;
	IMessageProcessor *FMessageProcessor;
private:
	QWidget *FStyleWidget;
	IMessageWindow *FWindow;
	IMessageStyleOptions FStyleOptions;
	QList<IMessageViewDropHandler *> FActiveDropHandlers;
};

#endif // VIEWWIDGET_H
