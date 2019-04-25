#pragma once
#ifndef ITEM_H
#define ITEM_H

#include <QGraphicsItem>

#include "setattributedialog.h"

inline bool CompairY(const QPointF &p1, const QPointF &p2){ return p1.y() > p2.y(); }
inline bool CompairX(const QPointF &p1, const QPointF &p2){ return p1.x() > p2.x(); }

/// 不规则图形
class GraphicsPolygonItem : public QGraphicsItem
{
public:
	GraphicsPolygonItem(const QVector<QPointF> point, const QColor color, const QString content, const QString id);
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	~GraphicsPolygonItem();
	// 修改属性
	void modifyAttr(const NodeAttribute attr);
private:
	// 颜色
	QColor m_color;
	// 多边形点
	QVector<QPointF> m_pointVec;
	// Tips字符串
	QString m_tips;
	SetAttributeDialog* m_pDialog;
	// 图元ID
	QString m_id;
	QPixmap m_nodePix;
protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
};

/// 文字标签
class GraphicsTextItem : public QGraphicsItem
{
public:
	GraphicsTextItem(const QPointF point, const QColor color, const QString content, const QString id, const QString front, const bool isMile = false);
	QRectF boundingRect() const;

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);

	~GraphicsTextItem();
	// 修改属性
	void modifyAttr(const NodeAttribute attr);
	// 设置里程碑标记
	void setIsMile(const bool isMile);
	// 里程标记判断
	bool IsMile();
	// 设置Tips信息
	bool setContent(const QString content);
	// 设置Tips位置
	bool setPosition(const QPointF point);
	// 获得ID
	QString getID() const;
private:
	// 文本显示颜色
	QColor m_color;
	// 文本显示位置
	QPointF m_point;
	// 显示文本
	QString m_showText;
	// 设置属性对话框
	SetAttributeDialog* m_pDialog;
	// 图元ID
	QString m_id;
	// 文字格式
	QString m_front;
	// 文字标签画板宽度
	int m_width;
	// 文字标签画板高度
	int m_height;
	// 里程标记
	bool m_isMile;
protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
};

/// 图片
class GraphicsPixmapItem : public QGraphicsItem
{
public:
	GraphicsPixmapItem(const QString filePath);
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	// 设置图片位置
	void setPixmapPos(const QPointF pos);
	// 设置描述信息
	void setDeclare(const QString declare);
	// 获得ID
	QString getID() const;
	// 设置ID
	void setID(const QString id);
	~GraphicsPixmapItem();
private:
	// 图片路径
	QString m_imgPath;
	// 图片位置
	QPointF *m_pPoint;
	// 详情信息
	QString m_declare;
	// ID
	QString m_id;
protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
};

/// 折线段
class GraphicsLineItem : public QGraphicsItem
{
public:
	GraphicsLineItem(const QString id, const QVector<QPointF> points, const QColor color, const double width);
	~GraphicsLineItem();
	QPainterPath shape() const;
	QRectF boundingRect() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR);
	// 获取ID
	QString getID() const;
	// 设置ID
	void setID(const QString id);
	void modifyAttr(const NodeAttribute attr);
private:
	// 线宽
	double m_strokeWidth;
	// 线颜色
	QColor m_color;
	// ID
	QString m_id;
	QVector<QPointF> m_pointVec;
	SetAttributeDialog* m_pDialog;
protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
};

#endif // ITEM_H
