#include "optionsdialogheader.h"
#include <utils/qt4qt5compat.h>

#include <QTextDocument>

OptionsDialogHeader::OptionsDialogHeader(const QString &ACaption, QWidget *AParent) : QLabel(AParent)
{
	setTextFormat(Qt::RichText);
	setText(QString("<h2>%1</h2>").arg(HTML_ESCAPE(ACaption)));
}

void OptionsDialogHeader::apply()
{
	emit childApply();
}

void OptionsDialogHeader::reset()
{
	emit childReset();
}
