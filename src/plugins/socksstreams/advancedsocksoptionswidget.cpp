#include "advancedsocksoptionswidget.h"

#include <QVBoxLayout>
#include <QListWidgetItem>
#include <definitions/optionvalues.h>
#include <utils/jid.h>

AdvancedSocksOptionsWidget::AdvancedSocksOptionsWidget(ISocksStreams *ASocksStreams, IConnectionManager *AConnectionManager, const OptionsNode &ANode, QWidget *AParent):
	QWidget(AParent),
	FSocksStreams(ASocksStreams),
	FConnectionManager(AConnectionManager),
	FOptionsNode(ANode),
	FProxySettings(NULL)
{
	ui.setupUi(this);

	FProxySettings = FConnectionManager!=NULL ? FConnectionManager->proxySettingsWidget(FOptionsNode.node("network-proxy"),ui.wdtProxySettings) : NULL;
	if (FProxySettings)
	{
		QVBoxLayout *layout = new QVBoxLayout(ui.wdtProxySettings);
		layout->setMargin(0);
		layout->addWidget(FProxySettings->instance());
		connect(FProxySettings->instance(),SIGNAL(modified()),SIGNAL(modified()));
	}

#if QT_VERSION >= 0x040000
	ui.lneForwardDirectAddress->setPlaceholderText(tr("host:port"));
#endif

	connect(ui.pbtAddStreamProxy,SIGNAL(clicked(bool)),SLOT(onAddStreamProxyClicked(bool)));
	connect(ui.pbtStreamProxyUp,SIGNAL(clicked(bool)),SLOT(onStreamProxyUpClicked(bool)));
	connect(ui.pbtStreamProxyDown,SIGNAL(clicked(bool)),SLOT(onStreamProxyDownClicked(bool)));
	connect(ui.pbtDeleteStreamProxy,SIGNAL(clicked(bool)),SLOT(onDeleteStreamProxyClicked(bool)));

	connect(ui.spbListenPort,SIGNAL(valueChanged(int)),SIGNAL(modified()));
	connect(ui.spbConnectTimeout,SIGNAL(valueChanged(int)),SIGNAL(modified()));
	connect(ui.chbEnableDirect,SIGNAL(stateChanged(int)),SIGNAL(modified()));
	connect(ui.chbEnableForwardDirect,SIGNAL(stateChanged(int)),SIGNAL(modified()));
	connect(ui.lneForwardDirectAddress,SIGNAL(textChanged(const QString &)),SIGNAL(modified()));
	connect(ui.chbUseAccountStreamProxy,SIGNAL(stateChanged(int)),SIGNAL(modified()));
	connect(ui.chbUseUserStreamProxy,SIGNAL(stateChanged(int)),SIGNAL(modified()));
	connect(ui.chbUseAccountNetworkProxy,SIGNAL(stateChanged(int)),SIGNAL(modified()));

	reset();
}

AdvancedSocksOptionsWidget::~AdvancedSocksOptionsWidget()
{}

void AdvancedSocksOptionsWidget::apply()
{
	Options::node(OPV_DATASTREAMS_SOCKSLISTENPORT).setValue(ui.spbListenPort->value());

	FOptionsNode.setValue(ui.spbConnectTimeout->value()*1000,"connect-timeout");
	FOptionsNode.setValue(ui.chbEnableDirect->isChecked(),"enable-direct-connections");
	FOptionsNode.setValue(ui.chbEnableForwardDirect->isChecked(),"enable-forward-direct");
	FOptionsNode.setValue(ui.lneForwardDirectAddress->text().trimmed(),"forward-direct-address");
	FOptionsNode.setValue(ui.chbUseAccountStreamProxy->isChecked(),"use-account-stream-proxy");
	FOptionsNode.setValue(ui.chbUseUserStreamProxy->isChecked(),"use-user-stream-proxy");
	FOptionsNode.setValue(ui.chbUseAccountNetworkProxy->isChecked(),"use-account-network-proxy");

	QStringList proxyItems;
	for (int row=0; row<ui.ltwStreamProxy->count(); row++)
	{
		QString proxyItem = Jid(ui.ltwStreamProxy->item(row)->text()).pBare();
		if (!proxyItems.contains(proxyItem))
			proxyItems.append(proxyItem);
	}
	FOptionsNode.setValue(proxyItems,"user-stream-proxy-list");

	if (FProxySettings)
		FConnectionManager->saveProxySettings(FProxySettings);

	emit childApply();
}

void AdvancedSocksOptionsWidget::reset()
{
	ui.spbListenPort->setValue(Options::node(OPV_DATASTREAMS_SOCKSLISTENPORT).value().toInt());
	ui.spbConnectTimeout->setValue(FOptionsNode.value("connect-timeout").toInt()/1000);
	ui.chbEnableDirect->setChecked(FOptionsNode.value("enable-direct-connections").toBool());
	ui.chbEnableForwardDirect->setChecked(FOptionsNode.value("enable-forward-direct").toBool());
	ui.lneForwardDirectAddress->setText(FOptionsNode.value("forward-direct-address").toString());
	ui.chbUseUserStreamProxy->setChecked(FOptionsNode.value("use-user-stream-proxy").toBool());
	ui.ltwStreamProxy->clear();
	ui.ltwStreamProxy->addItems(FOptionsNode.value("user-stream-proxy-list").toStringList());
	ui.chbUseAccountStreamProxy->setChecked(FOptionsNode.value("use-account-stream-proxy").toBool());
	ui.chbUseAccountNetworkProxy->setChecked(FOptionsNode.value("use-account-network-proxy").toBool());
	if (FProxySettings)
		FProxySettings->reset();
	emit childReset();
}

void AdvancedSocksOptionsWidget::onAddStreamProxyClicked(bool)
{
	QString proxy = ui.lneStreamProxy->text().trimmed();
	if (Jid(proxy).isValid() && ui.ltwStreamProxy->findItems(proxy, Qt::MatchExactly).isEmpty())
	{
		ui.ltwStreamProxy->addItem(proxy);
		ui.lneStreamProxy->clear();
		emit modified();
	}
}

void AdvancedSocksOptionsWidget::onStreamProxyUpClicked(bool)
{
	if (ui.ltwStreamProxy->currentRow() > 0)
	{
		int row = ui.ltwStreamProxy->currentRow();
		ui.ltwStreamProxy->insertItem(row-1, ui.ltwStreamProxy->takeItem(row));
		ui.ltwStreamProxy->setCurrentRow(row-1);
		emit modified();
	}
}

void AdvancedSocksOptionsWidget::onStreamProxyDownClicked(bool)
{
	if (ui.ltwStreamProxy->currentRow() < ui.ltwStreamProxy->count()-1)
	{
		int row = ui.ltwStreamProxy->currentRow();
		ui.ltwStreamProxy->insertItem(row+1, ui.ltwStreamProxy->takeItem(row));
		ui.ltwStreamProxy->setCurrentRow(row+1);
		emit modified();
	}
}

void AdvancedSocksOptionsWidget::onDeleteStreamProxyClicked(bool)
{
	if (ui.ltwStreamProxy->currentRow()>=0)
	{
		delete ui.ltwStreamProxy->takeItem(ui.ltwStreamProxy->currentRow());
		emit modified();
	}
}

void AdvancedSocksOptionsWidget::onTimeoutValueChanged(int ATimeout)
{
	ui.spbConnectTimeout->setSuffix(QString(" ")+tr("second(s)", "", ATimeout));
}
