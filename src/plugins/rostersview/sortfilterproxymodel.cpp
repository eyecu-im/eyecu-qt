#include "sortfilterproxymodel.h"

#include <definitions/optionvalues.h>
#include <definitions/rosterindexkinds.h>
#include <definitions/rosterindexroles.h>
#include <definitions/rosterindexkindorders.h>
#include <utils/options.h>

SortFilterProxyModel::SortFilterProxyModel(IRostersViewPlugin *ARostersViewPlugin, QObject *AParent) : QSortFilterProxyModel(AParent)
{
	FShowOffline = true;
	FSortMode = IRostersView::SortByStatus;
	FRostersView = ARostersViewPlugin->rostersView();
}

SortFilterProxyModel::~SortFilterProxyModel()
{

}

void SortFilterProxyModel::invalidate()
{
	FSortMode = Options::node(OPV_ROSTER_SORTMODE).value().toInt();
	FShowOffline = Options::node(OPV_ROSTER_SHOWOFFLINE).value().toBool();
	QSortFilterProxyModel::invalidate();
}

bool SortFilterProxyModel::compareVariant( const QVariant &ALeft, const QVariant &ARight ) const
{
	switch (ALeft.userType())
	{
	case QVariant::Invalid:
		return (ARight.type() != QVariant::Invalid);
	case QVariant::Int:
		return ALeft.toInt() < ARight.toInt();
	case QVariant::UInt:
		return ALeft.toUInt() < ARight.toUInt();
	case QVariant::LongLong:
		return ALeft.toLongLong() < ARight.toLongLong();
	case QVariant::ULongLong:
		return ALeft.toULongLong() < ARight.toULongLong();
	case QMetaType::Float:
		return ALeft.toFloat() < ARight.toFloat();
	case QVariant::Double:
		return ALeft.toDouble() < ARight.toDouble();
	case QVariant::Char:
		return ALeft.toChar() < ARight.toChar();
	case QVariant::Date:
		return ALeft.toDate() < ARight.toDate();
	case QVariant::Time:
		return ALeft.toTime() < ARight.toTime();
	case QVariant::DateTime:
		return ALeft.toDateTime() < ARight.toDateTime();
	case QVariant::String:
	default:
		if (isSortLocaleAware())
			return ALeft.toString().localeAwareCompare(ARight.toString()) < 0;
		else
			return ALeft.toString().compare(ARight.toString(), sortCaseSensitivity()) < 0;
	}
	return false;
}

bool SortFilterProxyModel::lessThan(const QModelIndex &ALeft, const QModelIndex &ARight) const
{
	int leftTypeOrder = ALeft.data(RDR_KIND_ORDER).toInt();
	int rightTypeOrder = ARight.data(RDR_KIND_ORDER).toInt();
	if (leftTypeOrder == rightTypeOrder)
	{
		QVariant leftSortOrder = ALeft.data(RDR_SORT_ORDER);
		QVariant rightSortOrder = ARight.data(RDR_SORT_ORDER);
		if (leftSortOrder.isNull() || rightSortOrder.isNull() || leftSortOrder==rightSortOrder)
		{
			if (FSortMode==IRostersView::SortByStatus && leftTypeOrder!=RIKO_STREAM_ROOT)
			{
				int leftShow = ALeft.data(RDR_SHOW).toInt();
				int rightShow = ARight.data(RDR_SHOW).toInt();
				if (leftShow != rightShow)
				{
					static const int show2order[] = {6,2,1,3,5,4,7,8};
					static const int showCount = sizeof(show2order)/sizeof(show2order[0]);
					if (leftShow<showCount && rightShow<showCount)
						return show2order[leftShow] < show2order[rightShow];
				}
			}
			return compareVariant(ALeft.data(Qt::DisplayRole),ARight.data(Qt::DisplayRole));
		}
		return compareVariant(leftSortOrder,rightSortOrder);
	}
	return leftTypeOrder < rightTypeOrder;
}

bool SortFilterProxyModel::filterAcceptsRow(int AModelRow, const QModelIndex &AModelParent) const
{
	QModelIndex index = sourceModel()->index(AModelRow,0,AModelParent);
	IRostersModel *rootModel = FRostersView->rostersModel();

	int visible = index.data(RDR_FORCE_VISIBLE).toInt();
	if (visible > 0)
	{
		return true;
	}
	else if (visible < 0)
	{
		return false;
	}
	else if (rootModel!=NULL && rootModel->isGroupKind(index.data(RDR_KIND).toInt()))
	{
		for (int childRow=0; sourceModel()->index(childRow,0, index).isValid(); childRow++)
			if (filterAcceptsRow(childRow,index))
				return true;
		return false;
	}
	else if (!FShowOffline && !index.data(RDR_SHOW).isNull())
	{
		int indexShow = index.data(RDR_SHOW).toInt();
		return indexShow!=IPresence::Offline && indexShow!=IPresence::Error;
	}

	return true;
}
