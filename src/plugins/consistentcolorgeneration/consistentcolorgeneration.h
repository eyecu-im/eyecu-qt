#ifndef CONSISTENTCOLORGENERATION_H
#define CONSISTENTCOLORGENERATION_H

#include <interfaces/ipluginmanager.h>
#include <interfaces/iconsistentcolorgeneration.h>
#include <interfaces/ioptionsmanager.h>

class ConsistentColorGeneration :
	public QObject,
	public IPlugin,
	public IConsistentColorGeneration,
	public IOptionsDialogHolder
{
	Q_OBJECT;
	Q_INTERFACES(IPlugin IConsistentColorGeneration IOptionsDialogHolder);
#if QT_VERSION >= 0x050000
	Q_PLUGIN_METADATA(IID "ru.rwsoftware.eyecu.IConsistentColorGeneration")
#endif
public:
	ConsistentColorGeneration();
	~ConsistentColorGeneration();
	// IPlugin
	virtual QObject *instance() override { return this; }
	virtual QUuid pluginUuid() const override { return CONSISTENTCOLORGENERATION_UUID; }
	virtual void pluginInfo(IPluginInfo *APluginInfo) override;
	virtual bool initConnections(IPluginManager *APluginManager, int &AInitOrder) override;
	virtual bool initObjects() override;
	virtual bool initSettings() override;
	virtual bool startPlugin() override { return true; }

	// IOptionsDialogHolder
	virtual QMultiMap<int, IOptionsDialogWidget *> optionsDialogWidgets(const QString &ANodeId, QWidget *AParent) override;

	// IConsistentColorGeneration
	virtual int getHue(const QString &AString) const override;
	virtual float getHueF(const QString &AString) const override;
	virtual int getHueAngle(const QString &AString) const override;
	virtual float getHueAngleF(const QString &AString) const override;
	virtual QColor calculateColor(const QString &AString, float ASaturation, float AVolume) const override;
	virtual QColor calculateColor(const QString &AString, int ASaturation, int AVolume) const override;

protected slots:
	void onOptionsChanged(const OptionsNode &ANode);

signals:
	void cvdChanged();

private:
	enum ColorVisionDeficiencies
	{
		None,
		RedGreenBlindness,
		BlueBlindness
	};

	IOptionsManager		*FOptionsManager;
};

#endif // CONSISTENTCOLORGENERATION_H
