#ifndef ICONSISTENTCOLORGENERATION_H
#define ICONSISTENTCOLORGENERATION_H

#define CONSISTENTCOLORGENERATION_UUID "{ac4bf782-0362-fc52-dc12-dfc159ab762a}"

#include <QColor>

class IConsistentColorGeneration
{
public:
	virtual int getHue(const QString &AString) const = 0;
	virtual float getHueF(const QString &AString) const = 0;
	virtual int getHueAngle(const QString &AString) const = 0;
	virtual float getHueAngleF(const QString &AString) const = 0;
	virtual QColor calculateColor(const QString &AString, float ASaturation, float AVolume) const = 0;
	virtual QColor calculateColor(const QString &AString, int ASaturation, int AVolume) const = 0;
protected:
	virtual void cvdChanged() = 0;
};

Q_DECLARE_INTERFACE(IConsistentColorGeneration,"eyeCU.Plugin.IConsistentColorGeneration/1.1")

#endif // ICONSISTENTCOLORGENERATION_H
