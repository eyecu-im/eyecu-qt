#ifndef MOODSELECT_H
#define MOODSELECT_H

#include "ui_moodselect.h"
#include "mood.h"

namespace Ui {
	class MoodSelect;
}

class MoodSelect : public QDialog {
    Q_OBJECT
public:
	MoodSelect(Mood *AMood, const QStringList &AMoodList, const QHash<QString, QStringList> &ATextList, const QHash<QString,QString> &AMoodKeys, const MoodData &AMoodData, QWidget *parent = 0);
	~MoodSelect();
	Ui::MoodSelect *ui;
    MoodData moodData() const;

private slots:
    void onEditTextChanged(const QString &ANewText);
    void onCurItemChanged(QTreeWidgetItem *ANewItem, QTreeWidgetItem *AOldItem);

protected:
    void changeEvent(QEvent *e);

	void fillTextList(const QString &AMoodName);
    void setCurrentItem(const MoodData &AMoodData);
    void fillMoodList();

private:
	Mood            *FMood;
    QTreeWidgetItem *FMoodTree;
    QStringList     FMoodList;
    QHash<QString, QStringList> FTextList;
    QHash<QString,QString>  FMoodKeys;
    MoodData        FMoodData;
};

#endif // MOODSELECT_H
