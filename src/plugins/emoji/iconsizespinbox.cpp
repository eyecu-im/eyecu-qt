#include "iconsizespinbox.h"

IconSizeSpinBox::IconSizeSpinBox(QWidget *AParent):
	QSpinBox(AParent)
{
	setMinimum(-1);
	setMaximum(-1);
	connect(this, SIGNAL(valueChanged(int)), SLOT(onValueChanged(int)));
}

void IconSizeSpinBox::setSizes(const QList<int> &ASizes)
{
	FSizes = ASizes;
	if (ASizes.isEmpty())
	{
		setMinimum(-1);
		setMaximum(-1);
	}
	else
	{
		setMinimum(0);
		setMaximum(ASizes.size()-1);
	}
}

void IconSizeSpinBox::setCurrentSize(int ASize)
{
	int index = FSizes.indexOf(ASize);
	if (index>-1)
		setValue(index);
}

int IconSizeSpinBox::currentSize() const
{
	if (FSizes.size() < (value()+1))
		return 0;
	else
		return FSizes.value(value());
}

int IconSizeSpinBox::valueFromText(const QString &AText) const
{
	if (FSizes.isEmpty())
		return 0;
	else
		return FSizes.indexOf(AText.left(AText.indexOf(' ')).toInt());
}

QString IconSizeSpinBox::textFromValue(int AValue) const
{
	if (FSizes.isEmpty())
		return QString();
	else
		return QString("%1 x %1").arg(FSizes.at(AValue));
}

void IconSizeSpinBox::onValueChanged(int AValue)
{
	if (FSizes.isEmpty())
		setSuffix(QString());
	else
		setSuffix(QString(" ")+tr("pixels", "", FSizes.value(AValue)));
}
