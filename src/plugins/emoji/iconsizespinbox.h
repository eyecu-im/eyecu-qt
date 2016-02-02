#ifndef ICONSIZESPINBOX_H
#define ICONSIZESPINBOX_H

#include <QSpinBox>

class IconSizeSpinBox : public QSpinBox
{
	Q_OBJECT
public:
	explicit IconSizeSpinBox(QWidget *AParent=NULL);
	void setSizes(const QList<int> &ASizes);
	void setCurrentSize(int ASize);
	int  currentSize() const;
	// QSpinBox interface
protected:
	int valueFromText(const QString &AText) const;
	QString textFromValue(int AValue) const;

protected slots:
	void onValueChanged(int AValue);

private:
	QList<int> FSizes;
};

#endif // ICONSIZESPINBOX_H
