#include "consistentcolorgeneration.h"
#include <definitions/optionwidgetorders.h>
#include <definitions/optionnodeorders.h>
#include <definitions/optionnodes.h>
#include <definitions/optionvalues.h>
#include <definitions/menuicons.h>
#include <definitions/resources.h>

#include <definitions/namespaces.h>
#include <definitions/stanzahandlerorders.h>
#include <utils/logger.h>

#include <QComboBox>
#include <QCryptographicHash>

ConsistentColorGeneration::ConsistentColorGeneration():
	FOptionsManager(NULL)
{}

ConsistentColorGeneration::~ConsistentColorGeneration()
{}

void ConsistentColorGeneration::pluginInfo(IPluginInfo *APluginInfo)
{
	APluginInfo->name = tr("Consistent Color Generation");
	APluginInfo->description = tr("Allows to consistently generate colors for a given string");
	APluginInfo->version = "1.0";
	APluginInfo->author = "Road Works Software";
	APluginInfo->homePage = "http://www.eyecu.ru";
}

bool ConsistentColorGeneration::initConnections(IPluginManager *APluginManager, int &AInitOrder)
{
	Q_UNUSED(AInitOrder);
	IPlugin *plugin = APluginManager->pluginInterface("IOptionsManager").value(0,NULL);
    if (plugin)
        FOptionsManager = qobject_cast<IOptionsManager *>(plugin->instance());
	connect(Options::instance(), SIGNAL(optionsChanged(OptionsNode)), SLOT(onOptionsChanged(OptionsNode)));
	return true;
}

bool ConsistentColorGeneration::initObjects()
{
	if (FOptionsManager)
		FOptionsManager->insertOptionsDialogHolder(this);
	return true;
}

bool ConsistentColorGeneration::initSettings()
{
	Options::setDefaultValue(OPV_CCG_COLORVISIONDEFICIENCIES, None);
	return true;
}

QMultiMap<int, IOptionsDialogWidget *> ConsistentColorGeneration::optionsDialogWidgets(const QString &ANodeId, QWidget *AParent)
{
	QMultiMap<int, IOptionsDialogWidget *> widgets;
	if (FOptionsManager)
	{
		if (ANodeId == OPN_COMMON)
		{
			QComboBox *comboBox = new QComboBox();
			comboBox->addItem(tr("None"), None);
			comboBox->addItem(tr("Red/Green-blindness"), RedGreenBlindness);
			comboBox->addItem(tr("Blue-blindness"), BlueBlindness);
			widgets.insertMulti(OWO_COMMON_CCG, FOptionsManager->newOptionsDialogWidget(
									Options::node(OPV_CCG_COLORVISIONDEFICIENCIES),
									tr("Color Vision Deficiencies"), comboBox, AParent));
		}

	}
	return widgets;
}

float ConsistentColorGeneration::getHueF(const QString &AString) const
{
	QByteArray hash = QCryptographicHash::hash(AString.toUtf8(), QCryptographicHash::Sha1);
	quint16 value = hash.at(0) + (hash.at(1)<<8);

	int cvd = Options::node(OPV_CCG_COLORVISIONDEFICIENCIES).value().toInt();
	switch (cvd)
	{
		case RedGreenBlindness:
			value = (value & 0x7fff) | (value & 0x4000) << 1;
			break;
		case BlueBlindness:
			value &= 0x3fff;
			break;
	}

	return value / 65536.0;
}

int ConsistentColorGeneration::getHue(const QString &AString) const
{
	return getHueF(AString);
}

float ConsistentColorGeneration::getHueAngleF(const QString &AString) const
{
	return getHueF(AString)*360;
}

int ConsistentColorGeneration::getHueAngle(const QString &AString) const
{
	return getHueAngleF(AString);
}

QColor ConsistentColorGeneration::calculateColor(const QString &AString, int ASaturation, int AVolume) const
{
	return calculateColor(AString, ASaturation/float(256), AVolume/float(256));
}

void ConsistentColorGeneration::onOptionsChanged(const OptionsNode &ANode)
{
	if (ANode.path() == OPV_CCG_COLORVISIONDEFICIENCIES)
		emit cvdChanged();
}

QColor ConsistentColorGeneration::calculateColor(const QString &AString, float ASaturation, float AVolume) const
{
	QColor color;
	color.setHsvF(getHueF(AString), ASaturation, AVolume);
	return color;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(plg_consistentcolorgeneration, ConsistentColorGeneration)
#endif
