#ifndef ACTIVITYSELECT_H
#define ACTIVITYSELECT_H

#include "ui_activityselect.h"
#include "activity.h"

namespace Ui {
	class ActivitySelect;
}

class ActivitySelect : public QDialog {
    Q_OBJECT
public:
	ActivitySelect(Activity *AActivity, const QHash<QString, QStringList> &AActivityList, const QHash<QString, QStringList> &AActivityTexts, const ActivityData &AActivityData, QWidget *parent = 0);
	~ActivitySelect();
    const ActivityData &activityData() const {return FActivityData;}

private slots:
    void onCurItemChanged(QTreeWidgetItem *AItemNew, QTreeWidgetItem *AItemOld);
    void onEditTextChanged(const QString &ANewText);

protected:
    void changeEvent(QEvent *e);
    void fillActivityTree();
    void fillTextList(const QString &AActivity);

private:
    Activity        *FActivity;
    ActivityData    FActivityData;
    QHash<QString, QStringList> FActivityNames;
    QHash<QString, QStringList> FActivityTexts;
	Ui::ActivitySelect  *ui;
};

#endif // ACTIVITYSELECT_H
