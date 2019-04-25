#ifndef XMLNODEACTION_H
#define XMLNODEACTION_H

#include <QFile>

#include "setattributedialog.h"
#include "svgviewshow.h"

class XmlNodeAction : public QObject
{
	Q_OBJECT
public:
	const static XmlNodeAction* Instance;
	static QString getNewID();
	// 加载svg
	static bool loadXml(QList<TipsWithPos> &itemList);
public slots:
	// 移除点
	static bool SLOT_Remove(const QString nodeId);
	// 移除点集合(List)
	static bool SLOT_Remove(const QList<QString> nodeIdList);
	// 更新属性
	static bool SLOT_Update(const QString nodeId, const NodeAttribute attr);
	// 更新属性
	static bool SLOT_UpdatePoints(const QString nodeId, const QVector<QPointF> points, EditType type);
	// 增加多边形
	static bool SLOT_Add(const QVector<QPoint> &points, const NodeAttribute attr, const QSize size);
	// 增加里程标记
	static bool SLOT_Add(const QPoint &start, const QPoint &end, const qreal startMile, const QSize size);
	// 增加里程起始点
	static bool SLOT_Add(const QPointF &points, const NodeAttribute attr, const QSize size, TextTagType textType = Normal);
	// 增加线段
	static bool SLOT_Add(const QVector<QPoint> &points, const NodeAttribute attr, const QSize size, const double strokeWidth);
private:
	XmlNodeAction();
	// 创建SVG新文件
	static bool create(const QString fileName, const QSize size);

	// 获得SVG文件句柄，当SVG文件找不到时自动创建
	static bool getFileHandle(QDomDocument& doc, QFile& file, const QSize size);
};

#endif // XMLNODEACTION_H
