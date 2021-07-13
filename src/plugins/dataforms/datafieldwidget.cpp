#include "datafieldwidget.h"

#include <QFocusEvent>
#include <QVBoxLayout>
#include <utils/jid.h>
#include <utils/qt4qt5compat.h>

DataFieldWidget::DataFieldWidget(IDataForms *ADataForms, const IDataField &AField, bool AReadOnly, QWidget *AParent) : QWidget(AParent)
{
	FField = AField;
	FReadOnly = AReadOnly;
	FDataForms = ADataForms;
	FMediaWidget = NULL;

	setLayout(new QVBoxLayout(this));
	layout()->setMargin(0);

	if (FDataForms->isMediaValid(AField.media))
	{
		FMediaWidget = FDataForms->mediaWidget(AField.media,this);
		layout()->addWidget(FMediaWidget->instance());
	}

	QString label = !FField.label.isEmpty() ? FField.label : FField.desc;
	QString desc = !FField.desc.isEmpty() ? QString("<span>%1</span>").arg(HTML_ESCAPE(FField.desc)) : QString();
	if (!FReadOnly && FField.type == DATAFIELD_TYPE_BOOLEAN)
	{
		FCheckBox = new QCheckBox(this);
		FCheckBox->setText(label + (FField.required ? QString("*") : QString()));
		FCheckBox->setToolTip(desc);
		FCheckBox->installEventFilter(this);
		connect(FCheckBox,SIGNAL(stateChanged(int)),SIGNAL(changed()));
		layout()->addWidget(FCheckBox);
	}
	else if (FField.type == DATAFIELD_TYPE_FIXED)
	{
		FLabel = new QLabel(this);
		FLabel->setWordWrap(true);
		FLabel->setTextFormat(Qt::PlainText);
		layout()->addWidget(FLabel);
	}
	else if (!FReadOnly && FField.type == DATAFIELD_TYPE_LISTSINGLE)
	{
		FComboBox = new QComboBox(this);
		appendLabel(label,FComboBox);
		foreach(const IDataOption &option, FField.options)
			FComboBox->addItem(option.label, option.value);
		if (FField.validate.method == DATAVALIDATE_METHOD_OPEN)
		{
			FComboBox->setEditable(true);
			FComboBox->setValidator(FDataForms->dataValidator(FField.validate,FComboBox));
			FLineEdit = FComboBox->lineEdit();
			connect(FLineEdit,SIGNAL(editTextChanged(const QString &)),SIGNAL(changed()));
		}
		FComboBox->setToolTip(desc);
		FComboBox->installEventFilter(this);
		connect(FComboBox,SIGNAL(currentIndexChanged(const QString &)),SIGNAL(changed()));
		layout()->addWidget(FComboBox);
	}
	else if (FField.type == DATAFIELD_TYPE_LISTMULTI)
	{
		FListWidget = new ListWidget(this);
		appendLabel(label,FListWidget);
		foreach(const IDataOption &option, FField.options)
		{
			QListWidgetItem *item = new QListWidgetItem(option.label);
			item->setData(Qt::UserRole,option.value);
			item->setFlags(!FReadOnly ? Qt::ItemIsEnabled|Qt::ItemIsUserCheckable : Qt::ItemIsEnabled);
			FListWidget->addItem(item);
		}
		FListWidget->setWrapping(true);
		FListWidget->setToolTip(desc);
		FListWidget->installEventFilter(this);
		connect(FListWidget,SIGNAL(itemChanged(QListWidgetItem *)),SIGNAL(changed()));
		layout()->addWidget(FListWidget);
	}
	else if (FField.type == DATAFIELD_TYPE_JIDMULTI || FField.type == DATAFIELD_TYPE_TEXTMULTI)
	{
		FTextEdit = new TextEdit(this);
		appendLabel(label,FTextEdit);
		FTextEdit->setToolTip(desc);
		FTextEdit->setReadOnly(FReadOnly);
		FTextEdit->setAcceptRichText(false);
		FTextEdit->installEventFilter(this);
		connect(FTextEdit,SIGNAL(textChanged()),SIGNAL(changed()));
		layout()->addWidget(FTextEdit);
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_DATE)
	{
		FDateEdit = new QDateEdit(this);
		appendLabel(label,FDateEdit);
		QDate min = QDate::fromString(FField.validate.min,Qt::ISODate);
		QDate max = QDate::fromString(FField.validate.max,Qt::ISODate);
		if (min.isValid())
			FDateEdit->setMinimumDate(min);
		if (max.isValid())
			FDateEdit->setMaximumDate(max);
		FDateEdit->setToolTip(desc);
		FDateEdit->setReadOnly(FReadOnly);
		FDateEdit->setCalendarPopup(true);
		FDateEdit->installEventFilter(this);
		connect(FDateEdit,SIGNAL(dateChanged(const QDate &)),SIGNAL(changed()));
		layout()->addWidget(FDateEdit);
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_TIME)
	{
		FTimeEdit = new QTimeEdit(this);
		appendLabel(label,FTimeEdit);
		QTime min = QTime::fromString(FField.validate.min,Qt::ISODate);
		QTime max = QTime::fromString(FField.validate.max,Qt::ISODate);
		if (min.isValid())
			FTimeEdit->setMinimumTime(min);
		if (max.isValid())
			FTimeEdit->setMaximumTime(max);
		FTimeEdit->setToolTip(desc);
		FTimeEdit->setReadOnly(FReadOnly);
		FTimeEdit->installEventFilter(this);
		connect(FTimeEdit,SIGNAL(timeChanged(const QTime &)),SIGNAL(changed()));
		layout()->addWidget(FTimeEdit);
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_DATETIME)
	{
		FDateTimeEdit = new QDateTimeEdit(this);
		appendLabel(label,FDateTimeEdit);
		QDateTime min = QDateTime::fromString(FField.validate.min,Qt::ISODate);
		QDateTime max = QDateTime::fromString(FField.validate.max,Qt::ISODate);
		if (min.isValid())
		{
			FDateTimeEdit->setMinimumDate(min.date());
			FDateTimeEdit->setMinimumTime(min.time());
		}
		if (max.isValid())
		{
			FDateTimeEdit->setMaximumDate(max.date());
			FDateTimeEdit->setMaximumTime(max.time());
		}
		FDateTimeEdit->setToolTip(desc);
		FDateTimeEdit->setReadOnly(FReadOnly);
		FDateTimeEdit->setCalendarPopup(true);
		FDateTimeEdit->installEventFilter(this);
		connect(FDateTimeEdit,SIGNAL(dateTimeChanged(const QDateTime &)),SIGNAL(changed()));
		layout()->addWidget(FDateTimeEdit);
	}
	else //HIDDEN JIDSINGLE TEXTPRIVATE TEXTSINGLE
	{
		FLineEdit = new QLineEdit(this);
		appendLabel(label,FLineEdit);
		if (FField.type == DATAFIELD_TYPE_TEXTPRIVATE)
			FLineEdit->setEchoMode(QLineEdit::Password);
		FLineEdit->setToolTip(desc);
		FLineEdit->setReadOnly(FReadOnly);
		FLineEdit->setValidator(FDataForms->dataValidator(FField.validate,FLineEdit));
		FLineEdit->installEventFilter(this);
		connect(FLineEdit,SIGNAL(textChanged(const QString &)),SIGNAL(changed()));
		layout()->addWidget(FLineEdit);
	}
	setValue(FField.value);
}

bool DataFieldWidget::isReadOnly() const
{
	return FReadOnly;
}

IDataField DataFieldWidget::userDataField() const
{
	IDataField field = FField;
	field.value = value();
	return field;
}

IDataField DataFieldWidget::dataField() const
{
	return FField;
}

QVariant DataFieldWidget::value() const
{
	if (!FReadOnly && FField.type == DATAFIELD_TYPE_BOOLEAN)
	{
		return FCheckBox->isChecked();
	}
	else if (FField.type == DATAFIELD_TYPE_FIXED)
	{
		return FField.value;
	}
	else if (FField.type == DATAFIELD_TYPE_JIDSINGLE)
	{
		return Jid::fromUserInput(FLineEdit->text()).full();
	}
	else if (FField.type == DATAFIELD_TYPE_JIDMULTI)
	{
		QStringList values = FTextEdit->toPlainText().split("\n", QString::SkipEmptyParts);
		for (int i = 0; i < values.count(); i++)
			values[i] = Jid::fromUserInput(values.at(i)).full();
		return values;
	}
	else if (!FReadOnly && FField.type == DATAFIELD_TYPE_LISTSINGLE)
	{
		if (FComboBox->currentIndex()>=0)
			return FComboBox->itemData(FComboBox->currentIndex()).toString();
		else if (FField.validate.method == DATAVALIDATE_METHOD_OPEN)
			return FComboBox->lineEdit()->text();
		return QString();
	}
	else if (FField.type == DATAFIELD_TYPE_LISTMULTI)
	{
		QStringList values;
		for (int i=0; i<FListWidget->count(); i++)
		{
			QListWidgetItem *item = FListWidget->item(i);
			if (item->checkState() == Qt::Checked)
				values.append(item->data(Qt::UserRole).toString());
		}
		return values;
	}
	else if (FField.type == DATAFIELD_TYPE_TEXTMULTI)
	{
		QStringList values;
		if (!FTextEdit->document()->isEmpty())
			values = FTextEdit->toPlainText().split("\n", QString::KeepEmptyParts);
		return values;
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_DATE)
	{
		return FDateEdit->date().toString(Qt::ISODate);
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_TIME)
	{
		return FTimeEdit->time().toString(Qt::ISODate);
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_DATETIME)
	{
		return FDateTimeEdit->dateTime().toString(Qt::ISODate);
	}
	else
	{
		return FLineEdit->text();
	}
}

void DataFieldWidget::setValue(const QVariant &AValue)
{
	if (!FReadOnly && FField.type == DATAFIELD_TYPE_BOOLEAN)
	{
		FCheckBox->setChecked(AValue.toBool());
	}
	else if (FField.type == DATAFIELD_TYPE_FIXED)
	{
		QString text = FField.label;
		QString prefix = !text.isEmpty() ? QString("\n   ") :  QString("\n");
		foreach(const QString &line, AValue.toStringList())
			text += !text.isEmpty() ? prefix + line : line;
		FLabel->setText(text);
	}
	else if (FField.type == DATAFIELD_TYPE_JIDSINGLE)
	{
		FLineEdit->setText(Jid(AValue.toString()).uFull());
	}
	else if (FField.type == DATAFIELD_TYPE_JIDMULTI)
	{
		FTextEdit->clear();
		foreach(const QString &line, AValue.toStringList())
			FTextEdit->append(Jid(line).uFull());
	}
	else if (!FReadOnly && FField.type == DATAFIELD_TYPE_LISTSINGLE)
	{
		int index = FComboBox->findData(AValue.toString());
		if (index>=0)
			FComboBox->setCurrentIndex(index);
		else if (FField.validate.method == DATAVALIDATE_METHOD_OPEN)
			FComboBox->setEditText(AValue.toString());
	}
	else if (FField.type == DATAFIELD_TYPE_LISTMULTI)
	{
		QStringList values = AValue.toStringList();
		for (int i=0; i<FListWidget->count(); i++)
		{
			QListWidgetItem *item = FListWidget->item(i);
			item->setCheckState(values.contains(item->data(Qt::UserRole).toString()) ? Qt::Checked  : Qt::Unchecked);
		}
	}
	else if (FField.type == DATAFIELD_TYPE_TEXTMULTI)
	{
		FTextEdit->clear();
		foreach(const QString &line, AValue.toStringList())
			FTextEdit->append(line);
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_DATE)
	{
		FDateEdit->setDate(QDate::fromString(AValue.toString(),Qt::ISODate));
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_TIME)
	{
		FTimeEdit->setTime(QTime::fromString(AValue.toString(),Qt::ISODate));
	}
	else if (FField.validate.type == DATAVALIDATE_TYPE_DATETIME)
	{
		FDateTimeEdit->setDateTime(QDateTime::fromString(AValue.toString(),Qt::ISODate));
	}
	else
	{
		FLineEdit->setText(AValue.toString());
	}
	emit changed();
}

IDataMediaWidget *DataFieldWidget::mediaWidget() const
{
	return FMediaWidget;
}

void DataFieldWidget::appendLabel(const QString &AText, QWidget *ABuddy)
{
	if (!AText.isEmpty())
	{
		FLabel = new QLabel(this);
		FLabel->setWordWrap(true);
		FLabel->setTextFormat(Qt::PlainText);
		FLabel->setText(AText + (FField.required ? QString("*") : QString()));
		FLabel->setBuddy(ABuddy);
		layout()->addWidget(FLabel);
	}
}

bool DataFieldWidget::eventFilter(QObject *AObject, QEvent *AEvent)
{
	if (AEvent->type()==QEvent::FocusIn || AEvent->type()==QEvent::FocusOut)
	{
		QFocusEvent *focusEvent = static_cast<QFocusEvent *>(AEvent);
		if (focusEvent)
		{
			if (AEvent->type() == QEvent::FocusIn)
				emit focusIn(focusEvent->reason());
			else
				emit focusOut(focusEvent->reason());
		}
	}
	return QObject::eventFilter(AObject,AEvent);
}

