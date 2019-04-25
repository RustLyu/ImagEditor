#ifndef COMMON_H
#define COMMON_H

#include <QString>
#include <QVector>
#include <QColor>

enum EditType
{
	EditType_None = 0,
	EditType_Polygon = 1,
	EditType_Text = 2,
	EditType_Mile = 3,
	EditType_Line = 4
};

enum modifyImagColor
{
	ToNone = 0,
	ToRedImag = 1,
	ToGreenImag = 2,
	ToBlueImag = 3,
	ToGrayImag = 4,
	ToRawImag = 5
};

enum TextTagType
{
	Normal = 0,
	StartMile = 1,
	EndMile = 2
};

struct TipsWithPos
{
	QString id;
	QString content;
	EditType type;
	struct Point
	{
		Point(){}
		Point(qreal srcX, qreal srcY)
		{
			x = srcX;
			y = srcY;
		}
		qreal x;
		qreal y;
	};
	QVector<Point> points;
	QColor color;
	qreal front;
	QString strokeWidth;
	bool isMile;
	TipsWithPos()
	{
		id = content = strokeWidth = nullptr;
		type = EditType_None;
		front = 0;
		isMile = false;
	}
};

struct NodeAttribute
{
	NodeAttribute()
	{
		red = green = blue = alpha = content = strokeWidth = "0";
	}
	QString content;
	QString red;
	QString green;
	QString blue;
	QString alpha;
	QString front;
	QString strokeWidth;
};

struct SetMileWithPos
{
	QPoint startPos;
	QPoint endPos;
	qreal startMile;
	qreal endMile;
	void Reset()
	{
		startMile = endMile = 0;
		startPos.setX(0);
		startPos.setY(0);
		endPos.setX(0);
		endPos.setY(0);
	}
};

#endif // COMMON_H
